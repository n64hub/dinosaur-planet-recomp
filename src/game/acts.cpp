// The maps' acts: the story moves the Volcano Force Point and Walled City on to the objects their later visits play,
// with Third SpellStone on Restored (the default); on Original both stay in act 1 as in the prototype.
//
// Each map has an act, a story value of 4 bits that dll29's export 15 reads and export 16 sets (map 4's is bit 0x4FE,
// Walled City's 0xE6, Diamond Bay's 0x3EE: the u16 at +0x1F90 + 2 * map in dll29's data). A new game puts every map
// in act 1 (dll29 0x300-0x334), and the object loader (func_8004B4A0) skips a placement whose byte +3 has bit act - 1
// set (byte +5 for acts 9 to 16). Sequences set the act of the map the playing object is in (dll3's event 0x1D, at
// 0x6DF0: a word act << 16 | 0x746 of ROM file 13), and the one such event in all of file 13 sets act 3. Of the code
// that sets a map's act by its number (`tools/dino.py calls dll29.16`), only the Queen Earthwalker names map 4 or 13
// (below), and the rest set their own map's act or another's. The two acts this file moves:
// - Volcano Force Point (map 4). Its second SpellPlace (@15B8, VFP_SpellPlace dll637: placed bit 0x878, carried 0x83B),
//   the podium that takes the third SpellStone for the second path (VFP_PodiumPoint @0D5C, which sets 0x515), its
//   seqpoint (@1380, holding 0x878), the door switch (@0D14) and the round door (@2B90) load in act 2 only, and the
//   first SpellPlace and podium in act 1 only. VFP_LevelControl (dll619) sets the map's act to 1 in its constructor
//   (0x58-0x70) and then reads it back, so its act 2 (0x4C0: the cutscene, then 0x83B and 0x22B, the stone given and
//   activated) and its act 3 (0x69C: 0x4E9 and 0x63C, which moves Diamond Bay on to act 3, the Pointback's second
//   egg in DBPointMum dll658 0xC0 and the stealer worms) never run. Here the act is 2 while the third SpellStone is
//   carried (0x83B) and 3 once it is placed (0x878), both at the map's load and in that constructor.
//   Act 11 is the Queen Earthwalker's: in SwapStone Hollow she sets map 4 to it before each of her two warps there
//   (SHqueenearthwal dll507_04F8+0xb8 and +0x118, then 0x486 or 0x487 and warp entry 81), and only act 11 loads the
//   two SHseqobjects that play those visits (30afa, played once 0x484 is set; 30afc, once 0x485 is) and the WarpPoint
//   beside entry 81, without the level's statues, doors, puzzles and SpellPlaces. Neither visit loads
//   VFP_LevelControl (group 10), the only object that puts the act back to 1, so on Original the map stays in act 11
//   after them: warp 81 with the first SpellStone carried (0x123) finds no SpellPlace. Here act 11 is left to the
//   game while a visit is to play (0x486 without 0x484, or 0x487 without 0x485), and once both are spent the map
//   loads in the story's act, as that constructor would have it. Any other act outside 1 to 3 is left alone.
// - Walled City (map 13). Its Sun and Moon Stones beside the boss door and the sockets on the TRex statues that take
//   them (WCSunStone dll272 and dll785, group 4) load in act 2 only, where the RedEye teeth and their sockets load in
//   act 1 only, and only in act 2 does WCLevelControl (dll779 0x1928) turn the block puzzles' counts into the bits
//   that open the Sun and Moon temples (0x810/0x811 into 0x812/0x813, 0x1BD0-0x1C84). The task list sends Sabre back
//   to Walled City for the stones once Moon Mountain Pass's spirit is released, so the act is 2 from then (0x317,
//   which WL_seqpoint's release sets).

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
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

    constexpr int queen_act = 11;
    constexpr int queen_first_visit = 0x486;
    constexpr int queen_first_visit_played = 0x484;
    constexpr int queen_second_visit = 0x487;
    constexpr int queen_second_visit_played = 0x485;

    const std::string third_stone = "third_stone";

    enum class ThirdStone {
        Original,
        Restored,
        OptionCount
    };

    const std::vector<recomp::config::ConfigOptionEnumOption> third_stone_options = {
        { ThirdStone::Original, "Original" },
        { ThirdStone::Restored, "Restored" },
    };

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

    // Whether one of the Queen Earthwalker's visits to the volcano is still to play.
    bool queen_visit_pending(uint8_t* rdram, recomp_context* ctx) {
        return (get(rdram, ctx, queen_first_visit) && !get(rdram, ctx, queen_first_visit_played)) ||
            (get(rdram, ctx, queen_second_visit) && !get(rdram, ctx, queen_second_visit_played));
    }

    // The act the story has the map in, from the one it is in: the volcano's from acts 1 to 3, and from the Queen's
    // act 11 once her visits are spent.
    int story_act(uint8_t* rdram, recomp_context* ctx, int map, int act) {
        if (map == volcano_force_point) {
            if (act == queen_act ? queen_visit_pending(rdram, ctx) : act < 1 || act > 3) {
                return act;
            }
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

void dino::add_third_stone_options(recomp::config::Config& general) {
    general.add_enum_option(
        third_stone,
        "Third SpellStone",
        "In the prototype, the Volcano Force Point and Walled City never get past the first visit, so the third "
        "SpellStone has nowhere to go, and nothing gives the moonseeds Krystal plants. <b>Restored</b> moves both "
        "with the story and gives four moonseeds as the stone is placed. <b>Original is the prototype as it was "
        "left</b>.",
        third_stone_options,
        ThirdStone::Restored);
}

bool dino::third_stone_restored() {
    return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(third_stone)) ==
        (uint32_t)ThirdStone::Restored;
}

// us.toml's hook at the end of func_80045BF4, which has loaded map $s0 and placed none of its objects yet.
extern "C" void dino_acts_map_loaded(uint8_t* rdram, recomp_context* ctx) {
    int map = (int32_t)ctx->r16;
    if ((map != volcano_force_point && map != walled_city) || !dino::third_stone_restored()) {
        return;
    }
    Calls calls(ctx);
    int act = act_of(rdram, ctx, map);
    int story = story_act(rdram, ctx, map, act);
    if (story != act) {
        set_act(rdram, ctx, map, story);
    }
    if (logging) {
        fprintf(stderr, story != act ? "acts: map %d from act %d to %d\n" : "acts: map %d stays in act %d\n", map, act,
            story);
    }
}

// us.toml's hook in VFP_LevelControl's constructor (dll619 0x70), as it sets its map's act with $a1 = 1.
extern "C" void dino_acts_volcano_level_control(uint8_t* rdram, recomp_context* ctx) {
    if (!dino::third_stone_restored()) {
        return;
    }
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
