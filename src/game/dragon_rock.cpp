// BOSS Kamerian Dragon (map 53), under Dragon Rock, and the fourth SpellStone. The fight is built but can't be won in
// the ROM: the dragon's health runs down and nothing reads it, and no object anywhere gives the fourth SpellStone
// (0x7A8), which the Desert Force Point's other SpellPlace takes (docs/story-points.md, Dragon Rock). With Kamerian
// Dragon on Restored (the default) the dragon dies when its health runs out and leaves the fourth SpellStone; on
// Original it plays as the console does.
//
// The fight, read from the ROM (offsets are a DLL's text):
// - KamerianBoss (dll707, object 0x4D9) keeps its state in the word at its +0xB8 pointer, its health in bits 9-16,
//   10 at the start (0x3B8). Its update (0x0FC4) lowers it by one for each hit taken in its last stage (0x1B90-0x1BB4),
//   and nothing reads it: at 0 it wraps to 255 and the fight goes on. The update also fills the player's magic.
// - Its stages: a hit of any kind on a head's spheres (13 and 14 on the left, 8 and 9 on the right) lights it, a hit of
//   type 0xF within about 9 s provokes it, and a hit of type 0xF on sphere 0 or 1 finishes that head; then the last
//   stage, with its health bar and flame. Type 0xF is the projectile spell's (projball, dll255), which Krystal picks up
//   on Dragon Rock's top (ProjectileSpell, 0x5CE) and casts from the spell scroll (C-left): A on it aims, A fires.
// - The Nurse (dll708) circles the player and knocks her back, into the pit beside the dragon too; a KamSwitch (dll285)
//   hit by the projectile raises the KamColumn (dll709) out of it.
// - CRSpellstone (dll730, object 0x390) is the SpellStone CloudRunner's race drops: +0x1E the bit set once picked up,
//   +0x20 a bit that hides it (both checked each update), and at its state 2 it is picked up by walking within 105
//   units of it (0x01A8). A stone already picked up when it is placed sets 0x49A (its table at 0x3F4), which the race
//   in map 43 has already set by then.
//
// What Restored changes:
// - The health's store (0x1BB8): at 0 the dragon is beaten. A CRSpellstone is placed at the player, its +0x1E 0x7A8
//   and +0x20 0x5F4 (the stone placed at the Desert Force Point), and picked up there at once.
// - The dragon's and the Nurse's updates: once it is beaten (0x7A8 or 0x5F4 set, so on every later visit too) each is
//   hidden (+0x6 0x4000, as CRSpellstone hides itself) and taken out of the hit list (func_80022274), and its update
//   is skipped.

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string kamerian_dragon = "kamerian_dragon";

    enum class KamerianDragon {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> kamerian_dragon_options = {
        { KamerianDragon::Original, "Original" },
        { KamerianDragon::Restored, "Restored" },
    };

    bool restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(kamerian_dragon)) ==
            (uint32_t)KamerianDragon::Restored;
    }

    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t new_placement = 0x800233F4; // (size, object id): a zeroed placement record
    constexpr int32_t spawn_object = 0x800213EC;  // (placement, 5, map, -1, [sp+0x10] parent): the object
    constexpr int32_t leave_hit_list = 0x80022274;
    constexpr int dragon_map = 53;
    constexpr int dragon_object = 0x4D9;
    constexpr int spellstone = 0x390;
    constexpr int fourth_stone_carried = 0x7A8;
    constexpr int fourth_stone_placed = 0x5F4;

    // DINO_LOG_DRAGON set (development): the dragon's health as it falls, and its end, are printed.
    const bool logging = std::getenv("DINO_LOG_DRAGON") != nullptr;
    // DINO_DRAGON_HEALTH=<n> (development): the dragon starts the fight with n health, to play its end in a short run.
    const int start_health = std::getenv("DINO_DRAGON_HEALTH") ? std::atoi(std::getenv("DINO_DRAGON_HEALTH")) : -1;

    void call(uint8_t* rdram, recomp_context* ctx, int32_t function) {
        LOOKUP_FUNC(function)(rdram, ctx);
    }

    // A hook's calls run below the hooked function's frame, and put back every register it had.
    struct Calls {
        recomp_context* ctx;
        recomp_context saved;
        explicit Calls(recomp_context* ctx) : ctx(ctx), saved(*ctx) {
            ctx->r29 -= 0x40;
        }
        ~Calls() {
            *ctx = saved;
        }
    };

    int get(uint8_t* rdram, recomp_context* ctx, int bit) {
        ctx->r4 = bit;
        call(rdram, ctx, get_bit);
        return (int32_t)ctx->r2;
    }

    bool beaten(uint8_t* rdram, recomp_context* ctx) {
        return get(rdram, ctx, fourth_stone_carried) || get(rdram, ctx, fourth_stone_placed);
    }

    void place_stone(uint8_t* rdram, recomp_context* ctx) {
        call(rdram, ctx, player_object);
        gpr player = ctx->r2;
        if (player == 0) {
            return;
        }
        ctx->r4 = 0x24;
        ctx->r5 = spellstone;
        call(rdram, ctx, new_placement);
        gpr p = ctx->r2;
        MEM_B(2, p) = 0x24 / 4;
        MEM_W(0x8, p) = MEM_W(0x18, player);
        MEM_W(0xC, p) = MEM_W(0x1C, player);
        MEM_W(0x10, p) = MEM_W(0x20, player);
        MEM_H(0x1E, p) = fourth_stone_carried;
        MEM_H(0x20, p) = fourth_stone_placed;
        ctx->r4 = p;
        ctx->r5 = 5;
        ctx->r6 = dragon_map;
        ctx->r7 = -1;
        MEM_W(0x10, ctx->r29) = 0;
        call(rdram, ctx, spawn_object);
        gpr stone = ctx->r2;
        if (stone != 0) {
            MEM_B(0, (gpr)MEM_W(0xB8, stone)) = 2; // dropped, as its export 0x2F0 makes it
        }
        if (logging) {
            fprintf(stderr, "dragon: beaten, SpellStone placed at the player (object 0x%08X)\n", (uint32_t)stone);
        }
    }

    void hide(uint8_t* rdram, recomp_context* ctx, gpr object) {
        if (MEM_H(0x6, object) & 0x4000) {
            return;
        }
        MEM_H(0x6, object) = MEM_H(0x6, object) | 0x4000;
        ctx->r4 = object;
        call(rdram, ctx, leave_hit_list);
    }

    // The way in and out. The layer the player is on is the signed byte at 0x80092A8C (func_80047374 raises it).
    constexpr int32_t player_layer = 0x80092A8C;
    constexpr int arena_won = 0x609;
    constexpr int into_room = 89;
    constexpr int out_of_room = 84;

    struct Box {
        float x0, x1, y0, y1, z0, z1;
        bool holds(float x, float y, float z) const {
            return x >= x0 && x <= x1 && y >= y0 && y <= y1 && z >= z0 && z <= z1;
        }
    };
    // In world units: the Skeetla arena at the bottom's east end (map 52, layer -1), and the dragon's room (53, layer
    // 0) over the same ground.
    const Box arena = { 1300.0f, 2400.0f, -1900.0f, -1700.0f, -6550.0f, -5900.0f };
    const Box room = { 1280.0f, 2560.0f, -2100.0f, -1600.0f, -7040.0f, -5700.0f };

    // The two warp entries, empty in ROM file 30: the room's setuppoint (map 53 @0000), and the arena where the
    // bottom's TriggerPlane 33F78 stands. +0xC and +0xE hold -1 for layer -1, as entry 88's do.
    struct DoorEntry {
        int index;
        float x, y, z;
        uint32_t layer;
    };
    const DoorEntry door_entries[] = {
        { into_room, 1925.0f, -1805.0f, -5965.0f, 0 },
        { out_of_room, 1659.0f, -1822.0f, -6121.0f, 0xFFFFFFFF },
    };
}

void dino::add_dragon_rock_options(recomp::config::Config& general) {
    general.add_enum_option(
        kamerian_dragon,
        "Kamerian Dragon",
        "The Kamerian Dragon under Dragon Rock dies once its health runs out, and leaves the fourth SpellStone for the "
        "Desert Force Point. <b>Original is the prototype as it was left</b>: the fight never ends, and the fourth "
        "SpellStone is nowhere.",
        kamerian_dragon_options,
        KamerianDragon::Restored
    );
}

// us.toml's hook in KamerianBoss's update (dll707 0x1BB8) once it has stored its health less one: $s4 its state, $s2
// the dragon.
extern "C" void dino_dragon_health(uint8_t* rdram, recomp_context* ctx) {
    int health = ((uint32_t)MEM_W(0, ctx->r20) >> 9) & 0xFF;
    if (logging) {
        fprintf(stderr, "dragon: health %d\n", health);
    }
    if (health != 0 || !restored()) {
        return;
    }
    Calls calls(ctx);
    if (!beaten(rdram, ctx)) {
        place_stone(rdram, ctx);
    }
}

// us.toml's hooks at the start of KamerianBoss's and the Nurse's updates (dll707 0x0FC4, dll708 0x0290): a0 the object.
// True skips the update.
extern "C" int dino_dragon_update(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return 0;
    }
    Calls calls(ctx);
    gpr object = ctx->r4;
    gpr placement = (gpr)MEM_W(0x4C, object);
    if (start_health >= 0 && placement != 0 && MEM_H(0, placement) == dragon_object) {
        static std::map<gpr, bool> started;
        if (!started[object]) {
            started[object] = true;
            gpr state = (gpr)MEM_W(0xB8, object);
            MEM_W(0, state) = (MEM_W(0, state) & ~(0xFF << 9)) | ((start_health & 0xFF) << 9);
        }
    }
    if (logging && placement != 0 && MEM_H(0, placement) == dragon_object) {
        static std::map<gpr, uint32_t> last;
        uint32_t state = MEM_W(0, (gpr)MEM_W(0xB8, object));
        if (last[object] != state) {
            fprintf(stderr, "dragon: 0x%08X state 0x%08X\n", (uint32_t)object, state);
            last[object] = state;
        }
    }
    if (!beaten(rdram, ctx)) {
        return 0;
    }
    hide(rdram, ctx, object);
    return 1;
}

// us.toml's hook in WARP_ENTRY (func_8004D1D4) once it has read entry 0x20(sp) of ROM file 30 into the buffer at
// 0x1C(sp), as krazoa.cpp's: the door's two entries.
extern "C" void dino_dragon_rock_warp_entry(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return;
    }
    int index = MEM_W(0x20, ctx->r29);
    gpr entry = (gpr)MEM_W(0x1C, ctx->r29);
    for (const DoorEntry& d : door_entries) {
        if (d.index == index) {
            memcpy(&MEM_W(0x0, entry), &d.x, 4);
            memcpy(&MEM_W(0x4, entry), &d.y, 4);
            memcpy(&MEM_W(0x8, entry), &d.z, 4);
            MEM_W(0xC, entry) = d.layer;
        }
    }
}

// us.toml's hook at the start of func_800145AC, once a pass of the main thread's loop: the warp the door takes now, or
// -1. In the arena once it is won and the dragon isn't, into the room; in the room 3 s after the SpellStone is
// picked up, back to the arena.
extern "C" int dino_dragon_rock_door(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return -1;
    }
    using clock = std::chrono::steady_clock;
    static clock::time_point quiet_until; // a warp's fade takes a while: nothing more until it is done
    static clock::time_point carried_at;
    static bool carried = false;
    clock::time_point now = clock::now();
    if (now < quiet_until) {
        return -1;
    }
    Calls calls(ctx);
    call(rdram, ctx, player_object);
    gpr player = ctx->r2;
    if (player == 0) {
        return -1;
    }
    float x, y, z;
    memcpy(&x, &MEM_W(0x18, player), 4);
    memcpy(&y, &MEM_W(0x1C, player), 4);
    memcpy(&z, &MEM_W(0x20, player), 4);
    int layer = MEM_B(0, (gpr)player_layer);
    int warp = -1;
    if (layer == -1 && arena.holds(x, y, z)) {
        if (get(rdram, ctx, arena_won) && !beaten(rdram, ctx)) {
            warp = into_room;
        }
    }
    else if (layer == 0 && room.holds(x, y, z) && beaten(rdram, ctx)) {
        if (!carried) {
            carried = true;
            carried_at = now;
        }
        else if (now - carried_at >= std::chrono::seconds(3)) {
            warp = out_of_room;
        }
    }
    else {
        carried = false;
    }
    if (warp >= 0) {
        carried = false;
        quiet_until = now + std::chrono::seconds(5);
        if (logging) {
            fprintf(stderr, "dragon: the door warps to %d from %.0f %.0f %.0f, layer %d\n", warp, x, y, z, layer);
        }
    }
    return warp;
}
