// Walled City's tasks the prototype never built: the Queen EarthWalker outside the wall, the RedEye chase for ten teeth
// that open the city's gate, and the King EarthWalker activating the third SpellStone once it is won from BOSS TRex
// (tasks 76-78 and 83 of the task list, ROM file 19). With Walled City on Restored (the default) they play; on Original
// the city is the prototype as it was left: its gate stands open and the stone is carried as BOSS TRex gives it.
//
// What the ROM holds, read from it (positions are world ones; map 13's cell is at x -8960, z -2560):
// - The task list: 75 ENTERED WALLED CITY ("check again where Tricky's mother once stood"), 76 QUEEN EARTHWALKER TELLS
//   YOU WHAT TO DO ("only Tricky can find his mom"), 77 TRICKY'S FLAME BEGINS THE CHASE! ("RedEyes need to be mad
//   before they are of any use"), 78 COLLECTED 10 TEETH TO ENTER THE CITY ("a roaring RedEye is good news"), 79 FOUND
//   THE KING EARTHWALKER, 80-82 the beacons, the boss door and the boss, and 83 KING EARTHWALKER ACTIVATES THE
//   SPELLSTONE ("don't forget about the King").
// - Objects made for them and placed nowhere: WCQueenEarthWal (#4FB, dll773, a dll18 character that turns to a player
//   within 100), WCTrexChase (#506, dll777: it draws its model and does nothing else), WCSabreChase (#508, dll778, the
//   same) and WCTeethBowl (#4FD, dll308, the game's object a player uses with A, which waits for a bit and sets one).
//   The teeth are WCTrexTooth (#522, dll272, the collectable of the boss door's two), and the collectable adds one to
//   the bit in its setup's +0x2C as it is picked up. Tricky's flame is a flameblast (#4F0, dll278).
// - The King (WCKingEarthWalk, dll774) stands in his cage in the courtyard at (-7987, -874, -6896). A with him sets
//   0x7FB, which plays his sequence: the SpellStone was taken by the RedEyes' leader, "bring it back to me, as only I
//   can activate it". No text and no code activates it, and the stone BOSS TRex gives is carried as 0x83B, the bit the
//   Volcano Force Point's second place takes (patches/boss_trex.c).
// - The way in from SwapStone Hollow (setup point 0, (-8634, -733, -4413)) is a corridor south and then east along z
//   -4800 to the RedEye-faced arch at x -7640, the gate, and the city beyond it. The prototype's arch is open.
//
// What Restored adds, with bits no code and no placement uses (the 4 bits at 0xD5 count the teeth):
// - The Queen stands in the corridor. Sabre coming to her with Tricky near (or alone, if no sidekick is out) has met
//   her (0x80A), and from then the RedEye by the gate can be angered.
// - The RedEye (WCTrexChase) waits by the gate. Tricky's flame reaching it (a flameblast within 150) makes it mad, and
//   it runs at Sabre; catching him it roars, and a tooth falls from it. Then it walks back to its place and waits for
//   the next flame, and it calms after 10 s of not catching him.
// - Ten teeth (0xD5 at 10) set 0x821, which lets the bowl beside the arch be used, and A on it opens the gate (0x82D)
//   and uses the teeth up. Until then the arch holds back a player coming from outside: one who starts inside the city
//   walks out freely.
// - BOSS TRex's defeat gives the stone as 0x82E, won but not active, and Sabre coming to the King with it has him
//   activate it: 0x83B, carried on to the Volcano Force Point as on Original.

#include <cmath>
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
    const std::string walled_city_option = "walled_city";

    enum class WalledCity {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> walled_city_options = {
        { WalledCity::Original, "Original" },
        { WalledCity::Restored, "Restored" },
    };

    bool restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(walled_city_option)) ==
            (uint32_t)WalledCity::Restored;
    }

    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t sidekick_object = 0x8002394C;
    constexpr int32_t new_placement = 0x800233F4; // (size, object id): a zeroed placement record
    constexpr int32_t spawn_object = 0x800213EC;  // (placement, 5, map, -1, [sp+0x10] parent): the object
    constexpr int32_t set_animation = 0x80023D30; // (object, animation, progress, 0)
    constexpr int32_t add_to_list = 0x80030EF0;   // (object, list), as a constructor joins its object to a list
    constexpr int32_t map_structs = 0x800B5328;   // each loaded map's header, by map number (0 while not loaded)
    constexpr int32_t objects = 0x800B1920;
    constexpr int32_t object_count = 0x800B1924;
    constexpr int32_t ticks = 0x8008C958;         // the frame's length in ticks, a float

    constexpr int walled_city = 13;

    constexpr int queen_object = 0x4FB;
    constexpr int chaser_object = 0x506;
    constexpr int bowl_object = 0x4FD;
    constexpr int tooth_object = 0x522;
    constexpr int flame_object = 0x4F0;
    constexpr int king_object = 0x500;
    constexpr int sideload_object = 0x75;

    constexpr int queen_met = 0x80A;
    constexpr int flame_learned = 0x245;
    constexpr int flame_command = 4;
    constexpr int flame_targets = 0x33;
    constexpr int teeth = 0xD5;
    constexpr int ten_teeth = 0x821;
    constexpr int gate_open = 0x82D;
    constexpr int stone_won = 0x82E;
    constexpr int stone_carried = 0x83B;

    struct Place {
        float x, y, z;
    };
    constexpr Place queen_place = { -8330.0f, -733.0f, -4720.0f };
    constexpr Place chaser_place = { -7820.0f, -735.0f, -4810.0f };
    constexpr Place bowl_place = { -7720.0f, -748.0f, -4700.0f };
    constexpr Place sideload_place = { -8634.0f, -733.0f, -4480.0f };
    constexpr uint8_t queen_yaw = 0x00;  // facing across the corridor
    constexpr uint8_t bowl_yaw = 0x40;

    // The arch, as a line across the corridor a player from outside doesn't cross while the gate is shut.
    constexpr float gate_x = -7660.0f;
    constexpr float gate_z_low = -4960.0f;
    constexpr float gate_z_high = -4640.0f;

    constexpr float meet_reach = 150.0f;
    constexpr float tricky_reach = 400.0f;
    constexpr float flame_reach = 150.0f;
    constexpr float offer_reach = 300.0f;
    constexpr float catch_reach = 70.0f;
    constexpr float king_reach = 120.0f;
    constexpr float chase_speed = 3.2f;    // per tick: a running Sabre outruns it, a walking one doesn't
    constexpr float walk_speed = 1.4f;
    constexpr float give_up_ticks = 600.0f;
    constexpr float roar_ticks = 90.0f;
    constexpr int idle_animation = 0;
    constexpr int walk_animation = 1;
    constexpr float walk_cycle_ticks = 50.0f;

    // DINO_LOG_WALLED_CITY set (development): the objects' placing, the chase, the teeth, the gate and the stone.
    const bool logging = std::getenv("DINO_LOG_WALLED_CITY") != nullptr;

    enum class Chase {
        Waiting,
        Mad,
        Roaring,
        Returning,
    };

    struct State {
        int32_t queen = 0;
        int32_t chaser = 0;
        int32_t bowl = 0;
        Chase chase = Chase::Waiting;
        float chase_ticks = 0.0f;
        float progress = 0.0f;
        bool placed = false;
        bool outside = false;
        int test_mad = 0; // the player was last seen west of the gate, in the corridor
    } state;

    void call(uint8_t* rdram, recomp_context* ctx, int32_t function) {
        LOOKUP_FUNC(function)(rdram, ctx);
    }

    int32_t word(uint8_t* rdram, int32_t address) {
        return (int32_t)MEM_W(0, (gpr)address);
    }

    float real(uint8_t* rdram, gpr address) {
        int32_t bits = MEM_W(0, address);
        float value;
        memcpy(&value, &bits, 4);
        return value;
    }

    void set_real(uint8_t* rdram, gpr address, float value) {
        int32_t bits;
        memcpy(&bits, &value, 4);
        MEM_W(0, address) = bits;
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

    // Whether an object made here is still in the game's list.
    bool alive(uint8_t* rdram, int32_t object, int oid) {
        if (object == 0) {
            return false;
        }
        gpr all = (gpr)MEM_W(0, (gpr)objects);
        for (int i = 0; all != 0 && i < MEM_W(0, (gpr)object_count); i++) {
            if ((int32_t)MEM_W(i * 4, all) == object) {
                return (int16_t)MEM_H(0x46, (gpr)object) == oid;
            }
        }
        return false;
    }

    // The nearest object of that id to (x, z) on the ground, and its distance.
    int32_t nearest(uint8_t* rdram, int oid, float x, float z, float* distance) {
        int32_t best = 0;
        float best_d = 1e30f;
        gpr all = (gpr)MEM_W(0, (gpr)objects);
        for (int i = 0; all != 0 && i < MEM_W(0, (gpr)object_count); i++) {
            gpr object = MEM_W(i * 4, all);
            if (object == 0 || (int16_t)MEM_H(0x46, object) != oid) {
                continue;
            }
            float dx = real(rdram, object + 0x18) - x, dz = real(rdram, object + 0x20) - z;
            float d = sqrtf(dx * dx + dz * dz);
            if (d < best_d) {
                best_d = d;
                best = (int32_t)object;
            }
        }
        *distance = best_d;
        return best;
    }

    float ground_distance(uint8_t* rdram, gpr a, gpr b) {
        float dx = real(rdram, a + 0x18) - real(rdram, b + 0x18), dz = real(rdram, a + 0x20) - real(rdram, b + 0x20);
        return sqrtf(dx * dx + dz * dz);
    }

    int32_t spawn(uint8_t* rdram, recomp_context* ctx, int oid, int size, Place at, const uint8_t* setup, int setup_size) {
        ctx->r4 = size;
        ctx->r5 = oid;
        call(rdram, ctx, new_placement);
        gpr p = ctx->r2;
        MEM_B(2, p) = size / 4;
        // Flag 2: the object loop (func_8004A800, 0x8004A94C) never frees it as out of the player's range, since
        // these are made once as the map loads (krazoa.cpp's palace objects are too).
        MEM_B(4, p) = 2;
        set_real(rdram, p + 0x8, at.x);
        set_real(rdram, p + 0xC, at.y);
        set_real(rdram, p + 0x10, at.z);
        for (int i = 0; i < setup_size; i++) {
            MEM_B(0x18 + i, p) = setup[i];
        }
        ctx->r4 = p;
        ctx->r5 = 5;
        ctx->r6 = walled_city;
        ctx->r7 = -1;
        MEM_W(0x10, ctx->r29) = 0;
        call(rdram, ctx, spawn_object);
        int32_t object = (int32_t)ctx->r2;
        if (logging) {
            fprintf(stderr, "walled city: object #%X placed at (%.0f, %.0f, %.0f): 0x%08X\n", oid, at.x, at.y, at.z,
                (uint32_t)object);
        }
        return object;
    }

    void place_objects(uint8_t* rdram, recomp_context* ctx) {
        {
            // As the King's placement: its turn, then no bits.
            const uint8_t setup[] = { queen_yaw, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF };
            state.queen = spawn(rdram, ctx, queen_object, 0x24, queen_place, setup, sizeof(setup));
        }
        {
            const uint8_t setup[] = { 0x40, 0x00, 0x00, 0x00 };
            state.chaser = spawn(rdram, ctx, chaser_object, 0x1C, chaser_place, setup, sizeof(setup));
            if (state.chaser != 0) {
                // Tricky's flame (dll211 0x8F18) is aimed at the object of list 0x33 nearest the player.
                ctx->r4 = state.chaser;
                ctx->r5 = flame_targets;
                call(rdram, ctx, add_to_list);
            }
            state.chase = Chase::Waiting;
            state.progress = 0.0f;
        }
        {
            // dll308: its turn (+0x18-0x1A), 0x18 (unusable until its bit, which it clears once used), the bit it sets
            // (+0x1C), no item (+0x1E), no sequence (+0x20), model 0, the bit it waits for (+0x22).
            const uint8_t setup[] = { bowl_yaw, 0x00, 0x00, 0x18, gate_open >> 8, gate_open & 0xFF, 0xFF, 0xFF,
                0xFF, 0x00, ten_teeth >> 8, ten_teeth & 0xFF, 0x00, 0x00, 0x00, 0x00 };
            state.bowl = spawn(rdram, ctx, bowl_object, 0x28, bowl_place, setup, sizeof(setup));
        }
        {
            // dll269 with no bit of its own waits for 0x4E5, Tricky's, and makes him when no sidekick is out.
            const uint8_t setup[] = { 0xFF, 0xFF, 0x00, 0x00 };
            spawn(rdram, ctx, sideload_object, 0x1C, sideload_place, setup, sizeof(setup));
        }
    }

    void drop_tooth(uint8_t* rdram, recomp_context* ctx, gpr chaser, gpr player) {
        // Between the RedEye and Sabre, on the ground. The boss door's teeth with no bit of their own and the count's
        // field in +0x2C, which the collectable adds one to.
        Place at = {
            (real(rdram, chaser + 0x18) + real(rdram, player + 0x18)) * 0.5f,
            real(rdram, player + 0x1C) + 5.0f,
            (real(rdram, chaser + 0x20) + real(rdram, player + 0x20)) * 0.5f,
        };
        const uint8_t setup[] = { 0x0A, 0x00, 0x05, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
            0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, teeth >> 8, teeth & 0xFF };
        spawn(rdram, ctx, tooth_object, 0x30, at, setup, sizeof(setup));
    }

    void pose(uint8_t* rdram, recomp_context* ctx, gpr chaser, int animation, float step) {
        state.progress += step;
        state.progress -= floorf(state.progress);
        int32_t bits;
        memcpy(&bits, &state.progress, 4);
        ctx->r4 = chaser;
        ctx->r5 = animation;
        ctx->r6 = bits;
        ctx->r7 = 0;
        call(rdram, ctx, set_animation);
    }

    // Turns the chaser to (x, z).
    void face(uint8_t* rdram, gpr chaser, float x, float z) {
        float dx = x - real(rdram, chaser + 0xC), dz = z - real(rdram, chaser + 0x14);
        MEM_H(0, chaser) = (int16_t)(atan2f(-dx, -dz) * (32768.0f / 3.14159265f));
    }

    // Moves the chaser on the ground toward (x, z) by up to `by`, facing that way; true once it is there.
    bool walk_to(uint8_t* rdram, gpr chaser, float x, float y, float z, float by) {
        float cx = real(rdram, chaser + 0xC), cz = real(rdram, chaser + 0x14);
        float dx = x - cx, dz = z - cz;
        float d = sqrtf(dx * dx + dz * dz);
        if (d < 1.0f) {
            return true;
        }
        float t = by < d ? by / d : 1.0f;
        set_real(rdram, chaser + 0xC, cx + dx * t);
        set_real(rdram, chaser + 0x14, cz + dz * t);
        float cy = real(rdram, chaser + 0x10);
        set_real(rdram, chaser + 0x10, cy + (y - cy) * 0.1f);
        face(rdram, chaser, x, z);
        return by >= d;
    }

    void run_chase(uint8_t* rdram, recomp_context* ctx, gpr player) {
        gpr chaser = (gpr)state.chaser;
        float frame = real(rdram, (gpr)ticks);
        float d = ground_distance(rdram, chaser, player);
        switch (state.chase) {
        case Chase::Waiting: {
            pose(rdram, ctx, chaser, idle_animation, frame / walk_cycle_ticks);
            if (d < 600.0f) {
                face(rdram, chaser, real(rdram, player + 0xC), real(rdram, player + 0x14));
            }
            if (!get(rdram, ctx, queen_met)) {
                break;
            }
            float flame_d;
            int32_t flame = nearest(rdram, flame_object, real(rdram, chaser + 0x18), real(rdram, chaser + 0x20), &flame_d);
            if ((flame != 0 && flame_d < flame_reach) || (getenv("DINO_WC_TEST_MAD") && d < 300.0f && !state.test_mad++)) {
                state.chase = Chase::Mad;
                state.chase_ticks = 0.0f;
                if (logging) {
                    fprintf(stderr, "walled city: the RedEye is mad\n");
                }
            }
            break;
        }
        case Chase::Mad:
            state.chase_ticks += frame;
            pose(rdram, ctx, chaser, walk_animation, frame * 2.0f / walk_cycle_ticks);
            walk_to(rdram, chaser, real(rdram, player + 0xC), real(rdram, player + 0x10), real(rdram, player + 0x14),
                chase_speed * frame);
            if (d < catch_reach) {
                state.chase = Chase::Roaring;
                state.chase_ticks = 0.0f;
                if (!get(rdram, ctx, gate_open) && get(rdram, ctx, teeth) < 15) {
                    drop_tooth(rdram, ctx, chaser, player);
                }
                if (logging) {
                    fprintf(stderr, "walled city: the RedEye roars, with %d teeth\n", get(rdram, ctx, teeth));
                }
            }
            else if (state.chase_ticks > give_up_ticks) {
                state.chase = Chase::Returning;
                if (logging) {
                    fprintf(stderr, "walled city: the RedEye calms down\n");
                }
            }
            break;
        case Chase::Roaring:
            state.chase_ticks += frame;
            pose(rdram, ctx, chaser, idle_animation, frame * 3.0f / walk_cycle_ticks);
            if (state.chase_ticks > roar_ticks) {
                state.chase = Chase::Returning;
            }
            break;
        case Chase::Returning:
            pose(rdram, ctx, chaser, walk_animation, frame / walk_cycle_ticks);
            if (walk_to(rdram, chaser, chaser_place.x, chaser_place.y, chaser_place.z, walk_speed * frame)) {
                state.chase = Chase::Waiting;
                MEM_H(0, chaser) = 0x4000;
            }
            break;
        }
    }

    // As a flame target near the player does (SwapStone Hollow's vines, dll265 0x104), the waiting RedEye puts the
    // flame on Tricky's command scroll this frame through his export 14 (dll211 0x1248), which 0x245 lets through.
    void offer_flame(uint8_t* rdram, recomp_context* ctx, gpr player) {
        if (state.chase != Chase::Waiting || ground_distance(rdram, (gpr)state.chaser, player) > offer_reach) {
            return;
        }
        call(rdram, ctx, sidekick_object);
        gpr tricky = ctx->r2;
        if (tricky == 0) {
            return;
        }
        ctx->r4 = tricky;
        ctx->r5 = flame_command;
        call(rdram, ctx, word(rdram, word(rdram, word(rdram, (int32_t)tricky + 0x68)) + 4 + 4 * 14));
    }

    void meet_queen(uint8_t* rdram, recomp_context* ctx, gpr player) {
        if (get(rdram, ctx, queen_met) || ground_distance(rdram, (gpr)state.queen, player) > meet_reach) {
            return;
        }
        call(rdram, ctx, sidekick_object);
        gpr tricky = ctx->r2;
        if (tricky != 0 && ground_distance(rdram, (gpr)state.queen, tricky) > tricky_reach) {
            return;
        }
        set(rdram, ctx, queen_met, 1);
        set(rdram, ctx, flame_learned, 1);
        if (logging) {
            fprintf(stderr, "walled city: met the Queen (%s)\n", tricky != 0 ? "with Tricky" : "no sidekick out");
        }
    }

    void hold_gate(uint8_t* rdram, recomp_context* ctx, gpr player) {
        float x = real(rdram, player + 0xC), z = real(rdram, player + 0x14);
        bool in_corridor = z > gate_z_low && z < gate_z_high;
        if (!in_corridor || get(rdram, ctx, gate_open)) {
            state.outside = in_corridor && x < gate_x;
            return;
        }
        if (x < gate_x) {
            state.outside = true;
        }
        else if (state.outside) {
            set_real(rdram, player + 0xC, gate_x - 1.0f);
            set_real(rdram, player + 0x18, gate_x - 1.0f);
        }
    }

    void count_teeth(uint8_t* rdram, recomp_context* ctx) {
        if (!get(rdram, ctx, gate_open) && !get(rdram, ctx, ten_teeth) && get(rdram, ctx, teeth) >= 10) {
            set(rdram, ctx, teeth, 0);
            set(rdram, ctx, ten_teeth, 1);
            if (logging) {
                fprintf(stderr, "walled city: ten teeth\n");
            }
        }
    }

    void activate_stone(uint8_t* rdram, recomp_context* ctx, gpr player) {
        if (!get(rdram, ctx, stone_won)) {
            return;
        }
        float d;
        int32_t king = nearest(rdram, king_object, real(rdram, player + 0x18), real(rdram, player + 0x20), &d);
        if (king == 0 || d > king_reach) {
            return;
        }
        set(rdram, ctx, stone_won, 0);
        set(rdram, ctx, stone_carried, 1);
        if (logging) {
            fprintf(stderr, "walled city: the King activates the SpellStone\n");
        }
    }
}

void dino::add_walled_city_options(recomp::config::Config& general) {
    general.add_enum_option(
        walled_city_option,
        "Walled City",
        "The prototype never built Walled City's way in or its King's part: the Queen EarthWalker is missing, the gate "
        "stands open, and the third SpellStone is active as soon as it is won. <b>Restored</b> brings the Queen to the "
        "corridor outside, has Sabre anger a RedEye with Tricky's flame for ten teeth that open the gate, and has the "
        "King activate the stone. <b>Original is the prototype as it was left</b>.",
        walled_city_options,
        WalledCity::Restored);
}

// us.toml's hook after BOSS TRex's defeat has given the stone (dino_trex_beaten, patches/boss_trex.c): on Restored it
// is won, for the King to activate.
extern "C" void dino_walled_city_trex_beaten(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return;
    }
    Calls calls(ctx);
    set(rdram, ctx, stone_carried, 0);
    set(rdram, ctx, stone_won, 1);
    if (logging) {
        fprintf(stderr, "walled city: the SpellStone is won, not yet active\n");
    }
}

// us.toml's hook after func_80046BC8 places the loaded maps' objects, each frame.
extern "C" void dino_walled_city_frame(uint8_t* rdram, recomp_context* ctx) {
    if (word(rdram, map_structs + 4 * walled_city) == 0) {
        state = State();
        return;
    }
    if (!restored()) {
        return;
    }
    Calls calls(ctx);
    call(rdram, ctx, player_object);
    gpr player = ctx->r2;
    if (player == 0) {
        return;
    }
    if (!state.placed) {
        state.placed = true;
        place_objects(rdram, ctx);
    }
    static int frames = 0;
    if (logging && frames++ % 30 == 0) {
        float flame_d = -1.0f;
        if (state.chaser != 0) {
            nearest(rdram, flame_object, real(rdram, (gpr)state.chaser + 0x18), real(rdram, (gpr)state.chaser + 0x20),
                &flame_d);
        }
        call(rdram, ctx, sidekick_object);
        gpr tricky = ctx->r2;
        if (flame_d < 1e29f) {
            fprintf(stderr, "walled city: a flame %.0f from the RedEye\n", flame_d);
        }
        gpr ts = tricky ? (gpr)(int32_t)MEM_W(0xB8, tricky) : 0;
        fprintf(stderr, "walled city: sidekick 0x%08X at (%.0f, %.0f) w(%.0f, %.0f) parent %08X alive %d st %d cmd %d "
            "tgt %08X, chase %d, teeth %d\n", (uint32_t)tricky,
            tricky ? real(rdram, tricky + 0xC) : 0.0f, tricky ? real(rdram, tricky + 0x14) : 0.0f,
            tricky ? real(rdram, tricky + 0x18) : 0.0f, tricky ? real(rdram, tricky + 0x20) : 0.0f,
            tricky ? (uint32_t)MEM_W(0x30, tricky) : 0, alive(rdram, (int32_t)tricky, 0x4E5) ? 1 : 0,
            ts ? (int)MEM_BU(0x18, ts) : -1, ts ? (int)MEM_BU(0x1D, ts) : -1, ts ? (uint32_t)MEM_W(0x28, ts) : 0,
            (int)state.chase, get(rdram, ctx, teeth));
    }
    if (alive(rdram, state.queen, queen_object)) {
        meet_queen(rdram, ctx, player);
    }
    if (alive(rdram, state.chaser, chaser_object)) {
        run_chase(rdram, ctx, player);
        offer_flame(rdram, ctx, player);
    }
    hold_gate(rdram, ctx, player);
    count_teeth(rdram, ctx);
    activate_stone(rdram, ctx, player);
}
