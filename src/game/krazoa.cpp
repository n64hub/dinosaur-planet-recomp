// The Krazoa shrines: the eight tests that each give Krystal or Sabre a spirit, carried to Warlock Mountain (map 11) and
// released there. The prototype has all eight shrines' maps and tests, but only Discovery Falls', Diamond Bay's and Moon
// Mountain Pass's are reached from the world, and the spirits of three (Willow Grove's, Golden Plains' and Northern
// Wastes') have no step at Warlock Mountain. With Krazoa Shrines on Finished (the default) this file finishes them out
// of what the ROM already holds, and on Original it leaves them as the console plays them. One fix is always on: Moon
// Mountain Pass's shrine Transporter sends the player to an empty warp entry.
//
// How a spirit goes, read from the ROM (offsets are a DLL's text, `@` a placement in its map's object list):
// - A shrine's win sets Warlock Mountain's act with dll29's export 16 (map 0xB): DF 2 (dll433 0xBC4), DB 3 (dll477
//   0xA04), MM 4 (dll466 0xA30), EC 5 (dll469 0x11A4), CC 6 (dll486 0xC44), GP 8 (dll472 0xD44), NW 9 (dll481 0xA84).
//   Willow Grove's (dll488) sets none: 7 is missing.
// - The spirit carried is a mask of the player's: its export 0x9C(mask) asks, 0xA0(mask, on) gives or takes. DF 1, DB 2,
//   MM 4, EC 8, CC 0x20, WG 0x40. 0x10 and 0x80 are given by nothing: here they are GP's and NW's.
// - WL_LevelControl (dll607) gives the act's mask on arrival unless its release is done (its update dispatches acts 1
//   to 7 at 0x298, `sltiu 8`): acts 6 and 7 read 0x366 for that (0x1544, 0x17D4), a bit nothing sets, so they gave CC's
//   and WG's spirits again on every visit. Acts 8 and 9 have no handler.
// - A WL_seqpoint (dll611, object 0x25F: +0x18 yaw, +0x19 mode, +0x1A radius, +0x1C its sequence, +0x1E trigger bit,
//   +0x20 done bit) at each spirit's site plays the release when the player walks in with its trigger bit set (mode 2),
//   or clear, setting it (mode 3). The sequence's last event takes the mask back and sets the released bits (dll611
//   0x4B4): DF 0x21B/0x277, DB 0x21F/0x316, MM 0x21C/0x317, EC 0x21D/0x318, CC 0x221 (case 15, mask at 0x8D4), WG 0x222
//   (case 17, mask at 0x778). The sequence plays where its seqpoint stands (dll3 0x8AC4).
// - The seqpoint's done bit is set the frame its sequence starts, and once set the seqpoint never plays again (dll611
//   0xD8-0x144). MM's, EC's, CC's and WG's all name bit 0, so the first of them to play stopped the other three.
//
// What Finished changes:
// - Ways in. ROM file 30's warp table has empty entries where the shrines' own Transporters send the player back: 65
//   (NW), 67 (WG), 68 (EC), 70 (GP) and 71 (CC); WARP_ENTRY (func_8004D1D4) gets them filled. Golden Plains has its
//   pad (map 9 @03F0, own entry 31) pointed at the empty entry 32: it goes to GP's shrine (42). The others get a
//   Transporter (object 0xEC, dll334: +0x19 its own entry, +0x1A where it sends, +0x20 the bit that shows it) beside
//   the place in their area the ROM ties to the shrine: EC's between Walled City's Sun and Moon stones, which open the
//   shrine in the older layout of map 36, a map with no place in the world grid; CC's by Cape Claw's three Krazoa
//   tablet receptacles; WG's by Willow Grove's sundial; NW's where Northern Wastes is entered. Each shows once the
//   spirit before it in the act order is released, EC's once the Moon Stone is placed.
// - Golden Plains' shrine exit (map 34 @06A0) went to DB's pad (66), a copy of DB's map, and in its mode 0 it sent
//   the player out on the way to the shrine: it goes to GP's pad (70), and only once the shrine is won.
// - Willow Grove: its state 3 (dll488 0x8FC) gave the spirit only while 0x1D1, the torch puzzle's bit, was clear, and
//   the torches light themselves as a demonstration about 15 s after the map loads (dll492 0x3F4), which sets it; it
//   now asks whether WG's spirit is already won (act 7 or later). Its win sets act 7.
// - Golden Plains' "already passed" check (dll472 0xBB0) read 0x12A, Moon Mountain Pass's bit, which MM's earlier win
//   always set, so GP never paid out: it reads act 8 or later instead.
// - Warlock Mountain, each visit: act 5 sets 0x142, the trigger of EC's release (@0FC0), which nothing set; act 7 sets
//   0x1D1, WG's (@1C0C), which the torches also clear; acts 8 and 9 give GP's and NW's spirits as acts 6 and 7 do
//   theirs, raise the column to an upper site (WL_colrise @1358, @1AB0, shown in acts 8 and 9 too) and place a release
//   seqpoint there: GP's at CC's site, playing CC's sequence, NW's at WG's, playing WG's, with the mask the sequence
//   takes back made 0x10 or 0x80 and the released bits 0x21E and 0x220, which ROM file 55 has saved between the other
//   released bits and nothing uses. Acts 6 and 7 read CC's and WG's released bits. The done bits of MM's, EC's, CC's
//   and WG's seqpoints become their released bits.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string krazoa_shrines = "krazoa_shrines";

    enum class KrazoaShrines {
        Original,
        Finished,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> krazoa_shrines_options = {
        { KrazoaShrines::Original, "Original" },
        { KrazoaShrines::Finished, "Finished" },
    };

    bool finished() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(krazoa_shrines)) ==
            (uint32_t)KrazoaShrines::Finished;
    }

    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t dll29_handle = 0x8008C9F4;
    constexpr int32_t new_placement = 0x800233F4; // (size, object id): a zeroed placement record
    constexpr int32_t spawn_object = 0x800213EC;  // (placement, 5, map, -1, [sp+0x10] parent), as dll434 0x1F4-0x33C
    constexpr int32_t map_structs = 0x800B5328;   // each loaded map's header, by map number (0 while not loaded)
    constexpr int32_t loading_map = 0x800B96AC;   // the header func_80045BF4 has just loaded
    constexpr int warlock_mountain = 0xB;

    constexpr int transporter = 0xEC;
    constexpr int seqpoint = 0x25F;

    int warlock_act = 0; // as WL_LevelControl last read it, for dll611's release

    // DINO_LOG_KRAZOA set (development): each object placed, spirit given and mask taken back is printed.
    const bool logging = std::getenv("DINO_LOG_KRAZOA") != nullptr;

    int32_t word(uint8_t* rdram, int32_t address) {
        return MEM_W(0, (gpr)address);
    }

    void call(uint8_t* rdram, recomp_context* ctx, int32_t function) {
        LOOKUP_FUNC(function)(rdram, ctx);
    }

    int get(uint8_t* rdram, recomp_context* ctx, int bit) {
        ctx->r4 = bit;
        call(rdram, ctx, get_bit);
        return (int32_t)ctx->r2;
    }

    void set(uint8_t* rdram, recomp_context* ctx, int bit, int value) {
        ctx->r4 = bit;
        ctx->r5 = value;
        call(rdram, ctx, set_bit);
    }

    void set_if_clear(uint8_t* rdram, recomp_context* ctx, int bit) {
        if (!get(rdram, ctx, bit)) {
            set(rdram, ctx, bit, 1);
        }
    }

    // dll29's export n is word n + 1 of the table its handle points at: 15 reads a map's act, 16 sets it.
    int warlock_act_now(uint8_t* rdram, recomp_context* ctx) {
        ctx->r4 = warlock_mountain;
        call(rdram, ctx, word(rdram, word(rdram, word(rdram, dll29_handle)) + 4 + 4 * 15));
        return (int32_t)ctx->r2;
    }

    void set_warlock_act(uint8_t* rdram, recomp_context* ctx, int act) {
        ctx->r4 = warlock_mountain;
        ctx->r5 = act;
        call(rdram, ctx, word(rdram, word(rdram, word(rdram, dll29_handle)) + 4 + 4 * 16));
    }

    // The player's own functions are reached through the table at **(player + 0x68), as dll611 0x760 does.
    int player_call(uint8_t* rdram, recomp_context* ctx, int offset, int mask, int on) {
        call(rdram, ctx, player_object);
        int32_t player = (int32_t)ctx->r2;
        ctx->r4 = player;
        ctx->r5 = mask;
        ctx->r6 = on;
        call(rdram, ctx, word(rdram, word(rdram, word(rdram, player + 0x68)) + offset));
        return (int32_t)ctx->r2;
    }

    bool has_spirit(uint8_t* rdram, recomp_context* ctx, int mask) {
        return player_call(rdram, ctx, 0x9C, mask, 0) != 0;
    }

    void give_spirit(uint8_t* rdram, recomp_context* ctx, int mask) {
        player_call(rdram, ctx, 0xA0, mask, 1);
    }

    // A hook's calls run below the hooked function's frame, and put back every register it had.
    struct Calls {
        uint8_t* rdram;
        recomp_context* ctx;
        recomp_context saved;
        Calls(uint8_t* rdram, recomp_context* ctx) : rdram(rdram), ctx(ctx), saved(*ctx) {
            ctx->r29 -= 0x40;
        }
        ~Calls() {
            *ctx = saved;
        }
    };

    struct WarpEntry {
        int index;
        float x, y, z;
    };

    // Each entry is a place the map it is in holds, in world units (a map's placement + 640 times its world grid cell,
    // ROM file 21). 70 is entry 31, GP's pad itself. 71 is the setuppoint by Cape Claw's receptacles (map 29 @22C8).
    // 68 is the setuppoint between Walled City's two stones (map 13 @0800). 67 is setuppoint 1 by Willow Grove's
    // sundial (map 37 @003C). 65 is the setuppoint in the snow where Northern Wastes is entered (map 10 @00B0).
    const WarpEntry warp_entries[] = {
        { 65, -3932.0f, -110.7f, -361.2f },
        { 67, -4523.2f, -1316.0f, -4004.5f },
        { 68, -7939.2f, -1123.0f, -7441.1f },
        { 70, -833.3f, 208.0f, 13285.5f },
        { 71, 4200.3f, -7.1f, 17613.1f },
    };

    // A change to one placement in a map's object list, found by its map, object id and unique id (+0x14).
    struct PlacementEdit {
        int map;
        int object;
        uint32_t uid;
        int offset;
        int size; // 1 or 2 bytes
        int value;
        bool always;
    };

    const PlacementEdit placement_edits[] = {
        // MM's shrine Transporter (map 32 @0B24) sends to entry 71, CC's: MM's pad is entry 73.
        { 32, transporter, 0x0C5D, 0x1A, 1, 73, true },
        // GP's pad (map 9 @03F0) to GP's shrine, shown once WG's spirit is released.
        { 9, transporter, 0x350D8, 0x1A, 1, 42, false },
        { 9, transporter, 0x350D8, 0x20, 2, 0x222, false },
        // GP's shrine exit (map 34 @06A0, a WM_WarpPoint, +0x1A where it sends) to GP's pad, and +0x1D's mode 0, which
        // warps within 508 units (+0x1E's 0x7F x 4) of it, 21 from the shrine, before the test, to the other shrines'
        // mode 2: only once +0x20's bit 0xFD, the win, is set (dll275 0x45C).
        { 34, 0x265, 0x2C8D, 0x1A, 1, 70, false },
        { 34, 0x265, 0x2C8D, 0x1D, 1, 2, false },
        // Warlock Mountain: the seqpoints' done bits, and the columns to the upper sites in acts 8 and 9 (a placement
        // is hidden in act n of 1-8 by bit n-1 of +3, and in act 9 by bit 7 of +5, func_8004B4A0).
        { 11, seqpoint, 0x27AB, 0x20, 2, 0x317, false }, // MM's, @2BD4
        { 11, seqpoint, 0x2983, 0x20, 2, 0x318, false }, // EC's, @0FC0
        { 11, seqpoint, 0x2AC7, 0x20, 2, 0x221, false }, // CC's, @1570
        { 11, seqpoint, 0x003C, 0x20, 2, 0x222, false }, // WG's, @1C0C
        { 11, 0x15D, 0x2AB7, 0x3, 1, 0xDF & ~0x80, false }, // CC's column, @1358: also act 8
        { 11, 0x15D, 0x002C, 0x5, 1, 0xE4 & ~0x80, false }, // WG's column, @1AB0: also act 9
    };

    // An object placed by this file while its map is loaded: the map frees it when it unloads (+0xAC, func_8004A800).
    struct Extra {
        int map;
        int object;
        float x, y, z;
        uint8_t params[12]; // from +0x18
        int act;            // the Warlock Mountain act it is placed in, or 0 for any
        bool placed;
    };

    // Transporters: yaw 0, own entry, destination, then dll334's 0x1E radius and its show bit, as GP's pad.
    // Seqpoints: yaw, mode 3 (walked into with its trigger clear, which it then sets), radius, sequence, trigger, no
    // done bit, at the upper sites of CC (@1570) and WG (@1C0C) with their yaws and radii.
    Extra extras[] = {
        { 13, transporter, -7939.2f, -1123.0f, -7441.1f, { 0, 68, 40, 0, 0, 0, 0x1E, 0xFF, 0x02, 0x64, 0, 0 }, 0, false },
        { 29, transporter, 4200.3f, -7.1f, 17613.1f, { 0, 71, 41, 0, 0, 0, 0x1E, 0xFF, 0x03, 0x18, 0, 0 }, 0, false },
        { 37, transporter, -4523.2f, -1316.0f, -4004.5f, { 0, 67, 39, 0, 0, 0, 0x1E, 0xFF, 0x02, 0x21, 0, 0 }, 0, false },
        { 10, transporter, -3932.0f, -110.7f, -361.2f, { 0, 65, 37, 0, 0, 0, 0x1E, 0xFF, 0x02, 0x1E, 0, 0 }, 0, false },
        { 11, seqpoint, 13959.0f, 470.0f, 2123.0f, { 0x00, 3, 0, 0x46, 0, 0x0F, 0x02, 0x1E, 0xFF, 0xFF, 0, 0 }, 8, false },
        { 11, seqpoint, 12932.0f, 456.0f, 1091.0f, { 0x7F, 3, 0, 0x48, 0, 0x11, 0x02, 0x20, 0xFF, 0xFF, 0, 0 }, 9, false },
    };

    void place(uint8_t* rdram, recomp_context* ctx, const Extra& e) {
        ctx->r4 = 0x24;
        ctx->r5 = e.object;
        call(rdram, ctx, new_placement);
        gpr p = ctx->r2;
        MEM_B(2, p) = 0x24 / 4;
        memcpy(&MEM_W(0x8, p), &e.x, 4);
        memcpy(&MEM_W(0xC, p), &e.y, 4);
        memcpy(&MEM_W(0x10, p), &e.z, 4);
        for (int i = 0; i < 12; i++) {
            MEM_B(0x18 + i, p) = (int8_t)e.params[i];
        }
        ctx->r4 = p;
        ctx->r5 = 5;
        ctx->r6 = e.map;
        ctx->r7 = -1;
        MEM_W(0x10, ctx->r29) = 0;
        call(rdram, ctx, spawn_object);
        if (logging) {
            fprintf(stderr, "krazoa: placed object 0x%X in map %d at %.0f %.0f %.0f\n", e.object, e.map, e.x, e.y, e.z);
        }
    }
}

void dino::add_krazoa_options(recomp::config::Config& general) {
    general.add_enum_option(
        krazoa_shrines,
        "Krazoa Shrines",
        "All eight Krazoa shrines can be reached and played, and each spirit is released at Warlock Mountain. "
        "<b>Original is the prototype as it was left</b>: three shrines are reached from the world, and Willow Grove's, "
        "Golden Plains' and Northern Wastes' spirits have nowhere to go.",
        krazoa_shrines_options,
        KrazoaShrines::Finished
    );
}

// us.toml's hook in WARP_ENTRY (func_8004D1D4) once it has read entry 0x20(sp) of ROM file 30 into the buffer at
// 0x1C(sp): 16 bytes, x, y and z as floats and an s16 layer, all zero in an empty entry.
extern "C" void dino_krazoa_warp_entry(uint8_t* rdram, recomp_context* ctx) {
    if (!finished()) {
        return;
    }
    int index = MEM_W(0x20, ctx->r29);
    gpr entry = (gpr)MEM_W(0x1C, ctx->r29);
    for (const WarpEntry& w : warp_entries) {
        if (w.index == index) {
            memcpy(&MEM_W(0x0, entry), &w.x, 4);
            memcpy(&MEM_W(0x4, entry), &w.y, 4);
            memcpy(&MEM_W(0x8, entry), &w.z, 4);
            MEM_W(0xC, entry) = 0;
        }
    }
}

// us.toml's hook at the end of func_80045BF4, which has loaded map $s0: its header (+0x8 the object list's bytes,
// +0x20 the list) is at 0x800B96AC, and none of its objects is placed yet.
extern "C" void dino_krazoa_map_loaded(uint8_t* rdram, recomp_context* ctx) {
    int map = (int32_t)ctx->r16;
    bool on = finished();
    for (Extra& e : extras) {
        if (e.map == map) {
            e.placed = false;
        }
    }
    gpr header = (gpr)word(rdram, loading_map);
    gpr list = (gpr)MEM_W(0x20, header);
    int bytes = MEM_HU(0x8, header);
    for (int at = 0; at < bytes;) {
        gpr p = list + at;
        int size = MEM_BU(2, p) * 4;
        if (size == 0) {
            break;
        }
        for (const PlacementEdit& edit : placement_edits) {
            if (edit.map == map && (edit.always || on) && MEM_H(0, p) == edit.object &&
                (uint32_t)MEM_W(0x14, p) == edit.uid) {
                if (edit.size == 1) {
                    MEM_B(edit.offset, p) = (int8_t)edit.value;
                }
                else {
                    MEM_H(edit.offset, p) = (int16_t)edit.value;
                }
            }
        }
        at += size;
    }
}

// us.toml's hook after func_80046BC8 places the loaded maps' objects (func_8004A800).
extern "C" void dino_krazoa_place_objects(uint8_t* rdram, recomp_context* ctx) {
    if (!finished()) {
        return;
    }
    Calls calls(rdram, ctx);
    for (Extra& e : extras) {
        if (word(rdram, map_structs + 4 * e.map) == 0) {
            e.placed = false;
        }
        else if (!e.placed) {
            e.placed = true;
            if (e.act == 0 || warlock_act_now(rdram, ctx) == e.act) {
                place(rdram, ctx, e);
            }
        }
    }
}

// us.toml's hook in WL_LevelControl's update (dll607 0x298), which has read Warlock Mountain's act into $v0.
extern "C" void dino_krazoa_warlock(uint8_t* rdram, recomp_context* ctx) {
    int act = (int32_t)ctx->r2;
    warlock_act = act;
    if (!finished()) {
        return;
    }
    Calls calls(rdram, ctx);
    switch (act) {
        case 5:
            if (!get(rdram, ctx, 0x318)) {
                set_if_clear(rdram, ctx, 0x142);
            }
            break;
        case 7:
            if (!get(rdram, ctx, 0x222)) {
                set_if_clear(rdram, ctx, 0x1D1);
            }
            break;
        case 8:
        case 9: {
            int released = act == 8 ? 0x21E : 0x220;
            int mask = act == 8 ? 0x10 : 0x80;
            if (!get(rdram, ctx, released)) {
                // What acts 6 and 7 set as they give their spirits (dll607 0x14FC, 0x1788).
                set_if_clear(rdram, ctx, 0x2D);
                set_if_clear(rdram, ctx, 0x1D7);
                set_if_clear(rdram, ctx, act == 8 ? 0x1CE : 0x1D1);
                if (!has_spirit(rdram, ctx, mask)) {
                    give_spirit(rdram, ctx, mask);
                    if (logging) {
                        fprintf(stderr, "krazoa: act %d gives spirit mask 0x%X\n", act, mask);
                    }
                }
            }
            break;
        }
    }
}

// us.toml's hooks in WL_seqpoint's release events (dll611), before the player's export 0xA0 takes CC's mask (0x8D4)
// and WG's (0x778) back: GP's and NW's releases play those sequences.
extern "C" void dino_krazoa_release_mask(uint8_t*, recomp_context* ctx) {
    if (warlock_act == 8) {
        ctx->r5 = 0x10;
    }
    else if (warlock_act == 9) {
        ctx->r5 = 0x80;
    }
    if (logging) {
        fprintf(stderr, "krazoa: act %d release takes mask 0x%X\n", warlock_act, (int)ctx->r5);
    }
}

// us.toml's hook at Willow Grove's state 3 (dll488 0x8FC), on GET_BIT(0x1D1)'s answer: with it set the shrine gives
// nothing.
extern "C" void dino_krazoa_wg_won(uint8_t* rdram, recomp_context* ctx) {
    if (!finished()) {
        return;
    }
    int won;
    {
        Calls calls(rdram, ctx);
        won = warlock_act_now(rdram, ctx) >= 7;
    }
    ctx->r2 = won;
}

// us.toml's hook at Willow Grove's state 4 (dll488 0xA18), its win, which sets no act.
extern "C" void dino_krazoa_wg_win(uint8_t* rdram, recomp_context* ctx) {
    if (!finished()) {
        return;
    }
    Calls calls(rdram, ctx);
    if (warlock_act_now(rdram, ctx) < 7) {
        set_warlock_act(rdram, ctx, 7);
    }
}

// us.toml's hook at Golden Plains' state 4 (dll472 0xBB0), on GET_BIT(0x12A)'s answer, which MM's win set.
extern "C" void dino_krazoa_gp_won(uint8_t* rdram, recomp_context* ctx) {
    if (!finished()) {
        return;
    }
    int won;
    {
        Calls calls(rdram, ctx);
        won = warlock_act_now(rdram, ctx) >= 8;
    }
    if (logging) {
        fprintf(stderr, "krazoa: Golden Plains' test passed, %s\n", won ? "already won" : "won now");
    }
    ctx->r2 = won;
}

// us.toml's hook at Golden Plains' timeout (dll472 0xB28), after func_800311E4 lists every carryable (group 0x12) and
// before the loop frees each one. The player's render writes the position of the object in its hands (its state's
// +0x868) every frame and calls that object's DLL, so freeing the one the player carries crashed the game. The barrel
// (dll301 0x158) lets go first through the player's export 11 (its table's +0x30) with 0; this does the same for the
// one carried, whatever the setting.
extern "C" void dino_krazoa_gp_timeout(uint8_t* rdram, recomp_context* ctx) {
    int32_t list = (int32_t)ctx->r2;
    int32_t count = word(rdram, (int32_t)ctx->r29 + 0x34);
    int32_t player = word(rdram, (int32_t)ctx->r29 + 0x50);
    if (logging) {
        fprintf(stderr, "krazoa: Golden Plains' test timed out, %d carryables\n", count);
    }
    if (!player) {
        return;
    }
    int32_t held = word(rdram, word(rdram, player + 0xB8) + 0x868);
    if (!held) {
        return;
    }
    for (int32_t i = 0; i < count; i++) {
        if (word(rdram, list + 4 * i) == held) {
            Calls calls(rdram, ctx);
            ctx->r4 = player;
            ctx->r5 = 0;
            call(rdram, ctx, word(rdram, word(rdram, word(rdram, player + 0x68)) + 0x30));
            if (logging) {
                fprintf(stderr, "krazoa: Golden Plains' timeout drops the carried object 0x%X\n", (unsigned)held);
            }
            return;
        }
    }
}

// us.toml's hook in Golden Plains' sequence callback (dll472 0x1134), with the event less one in t4 (event 4 is
// 0x11DC: state 4). The shrine's update doesn't run while its sequence plays: the win's sequence 1 sends event 6
// (state 5, the reward) and then event 4, so the shrine came back to state 4, played the win again and never paid out.
// While state 5 is pending, event 4 is dropped (t4 past the table, as an unknown event is).
extern "C" void dino_krazoa_gp_event(uint8_t* rdram, recomp_context* ctx) {
    if (!finished()) {
        return;
    }
    if ((int32_t)ctx->r12 == 4 - 1 && MEM_BU(0x15, ctx->r16) == 5) {
        ctx->r12 = 0xFF;
        if (logging) {
            fprintf(stderr, "krazoa: Golden Plains' win keeps its reward\n");
        }
    }
}
