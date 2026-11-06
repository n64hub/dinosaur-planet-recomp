// The maps' acts: the story moves the Volcano Force Point and Walled City on to the objects their later visits play.
//
// Each map has an act, a story value of 4 bits that dll29's export 15 reads and export 16 sets (map 4's is bit 0x4FE,
// Walled City's 0xE6, Diamond Bay's 0x3EE: the u16 at +0x1F90 + 2 * map in dll29's data). A new game puts every map
// in act 1 (dll29 0x300-0x334), and the object loader (func_8004B4A0) skips a placement whose byte +3 has bit act - 1
// set (byte +5 for acts 9 to 16). Sequences set the act of the map the playing object is in (dll3's event 0x1D, at
// 0x6DF0: a word act << 16 | 0x746 of ROM file 13), and the one such event in all of file 13 sets act 3. Nothing else
// sets the two acts this file moves:
// - Volcano Force Point (map 4). Its second SpellPlace (@15B8, VFP_SpellPlace dll637: placed bit 0x878, carried 0x83B),
//   the podium that takes the third SpellStone for the second path (VFP_PodiumPoint @0D5C, which sets 0x515), its
//   seqpoint (@1380, holding 0x878), the door switch (@0D14) and the round door (@2B90) load in act 2 only, and the
//   first SpellPlace and podium in act 1 only. VFP_LevelControl (dll619) sets the map's act to 1 in its constructor
//   (0x58-0x70) and then reads it back, so its act 2 (0x4C0: the cutscene, then 0x83B and 0x22B, the stone given and
//   activated) and its act 3 (0x69C: 0x4E9 and 0x63C, which moves Diamond Bay on to act 3, the Pointback's second
//   egg in DBPointMum dll658 0xC0 and the stealer worms) never run. Here the act is 2 while the third SpellStone is
//   carried (0x83B) and 3 once it is placed (0x878), both at the map's load and in that constructor.
// - Walled City (map 13). Its Sun and Moon Stones beside the boss door and the sockets on the TRex statues that take
//   them (WCSunStone dll272 and dll785, group 4) load in act 2 only, where the RedEye teeth and their sockets load in
//   act 1 only, and only in act 2 does WCLevelControl (dll779 0x1928) turn the block puzzles' counts into the bits
//   that open the Sun and Moon temples (0x810/0x811 into 0x812/0x813, 0x1BD0-0x1C84). The task list sends Sabre back
//   to Walled City for the stones once Moon Mountain Pass's spirit is released, so the act is 2 from then (0x317,
//   which WL_seqpoint's release sets).

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "recomp.h"
#include "dino.h"

namespace {
    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t dll29_handle = 0x8008C9F4;

    constexpr int volcano_force_point = 4;
    constexpr int walled_city = 13;

    constexpr int third_stone_carried = 0x83B;
    constexpr int third_stone_placed = 0x878;
    constexpr int third_spirit_released = 0x317;

    // DINO_LOG_ACTS set (development): each act this file moves is printed.
    const bool logging = std::getenv("DINO_LOG_ACTS") != nullptr;

    int32_t word(uint8_t* rdram, int32_t address) {
        return MEM_W(0, (gpr)address);
    }

    void call(uint8_t* rdram, recomp_context* ctx, int32_t function) {
        LOOKUP_FUNC(function)(rdram, ctx);
    }

    bool get(uint8_t* rdram, recomp_context* ctx, int bit) {
        ctx->r4 = bit;
        call(rdram, ctx, get_bit);
        return (int32_t)ctx->r2 != 0;
    }

    // dll29's export n is word n + 1 of the table its handle points at: 15 reads a map's act, 16 sets it.
    int32_t dll29_export(uint8_t* rdram, int n) {
        return word(rdram, word(rdram, word(rdram, dll29_handle)) + 4 + 4 * n);
    }

    int act_of(uint8_t* rdram, recomp_context* ctx, int map) {
        ctx->r4 = map;
        call(rdram, ctx, dll29_export(rdram, 15));
        return (int32_t)ctx->r2;
    }

    void set_act(uint8_t* rdram, recomp_context* ctx, int map, int act) {
        ctx->r4 = map;
        ctx->r5 = act;
        call(rdram, ctx, dll29_export(rdram, 16));
    }

    // The act the story has the map in, from the one it is in.
    int story_act(uint8_t* rdram, recomp_context* ctx, int map, int act) {
        if (map == volcano_force_point) {
            if (get(rdram, ctx, third_stone_placed)) {
                return 3;
            }
            if (get(rdram, ctx, third_stone_carried)) {
                return 2;
            }
            return 1;
        }
        if (map == walled_city && act == 1 && get(rdram, ctx, third_spirit_released)) {
            return 2;
        }
        return act;
    }

    // A hook's calls run below the hooked function's frame, and put back every register it had.
    struct Calls {
        recomp_context* ctx;
        recomp_context saved;
        Calls(recomp_context* ctx) : ctx(ctx), saved(*ctx) {
            ctx->r29 -= 0x40;
        }
        ~Calls() {
            *ctx = saved;
        }
    };
}

// us.toml's hook at the end of func_80045BF4, which has loaded map $s0 and placed none of its objects yet.
extern "C" void dino_acts_map_loaded(uint8_t* rdram, recomp_context* ctx) {
    int map = (int32_t)ctx->r16;
    if (map != volcano_force_point && map != walled_city) {
        return;
    }
    Calls calls(ctx);
    int act = act_of(rdram, ctx, map);
    int story = story_act(rdram, ctx, map, act);
    if (story != act) {
        set_act(rdram, ctx, map, story);
        if (logging) {
            fprintf(stderr, "acts: map %d from act %d to %d\n", map, act, story);
        }
    }
}

// us.toml's hook in VFP_LevelControl's constructor (dll619 0x70), as it sets its map's act with $a1 = 1.
extern "C" void dino_acts_volcano_level_control(uint8_t* rdram, recomp_context* ctx) {
    int act;
    {
        Calls calls(ctx);
        act = story_act(rdram, ctx, volcano_force_point, 1);
    }
    ctx->r5 = act;
    if (logging) {
        fprintf(stderr, "acts: VFP_LevelControl sets act %d\n", act);
    }
}
