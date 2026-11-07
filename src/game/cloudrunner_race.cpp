// CloudRunner Fortress's snow race (map 43, below the treasure caves): won, it leads nowhere. Taking the SpellStone the
// SnowClaw drops ends the race, but the bit that runs the race's end, 0x49A, is set by nothing in the ROM, so its
// closing sequence never plays and Bonehead never hears of it (docs/story-points.md, CloudRunner Fortress). With
// CloudRunner Race on Restored (the default) taking the stone in the race sets it; on Original it plays as the console
// does.
//
// Read from the ROM (offsets are a DLL's text):
// - CRCloudRace (dll729, map 43's group 1) keeps its state at its state's +0x4: 2 waiting for the player to mount
//   CRSnowBike (dll210's export 7 says she rides), which sets 0x49D and 0x497 and makes it 3, the race. Each frame of
//   the race (0x0308) it sees the race lost (0x499, the fuel run out: then back to 2 by 5) or won: 0x2E8, the second
//   SpellStone carried, clears 0x497 and 0x49D and makes it 4 (0x0380). At 4 it waits for 0x49F (0x02AC).
// - A ram is CRSnowBike's hit of type 0x14, which its hit box makes only while the player rides it (its state's +0x3F0
//   is 2) faster than 2 (0x1188-0x1214). CRSnowClaw (dll728, 0x0418) takes one at most every 20 frames, counting its
//   +0x34 down from 4 (0x0440); at the fifth it falls off its bike and, the one that is object 0x389 (not
//   CRSnowClaw2), drops the SpellStone (CRSpellstone, dll730, object type 0x20) if the stone is within 500 of it: the
//   stone's export 7 with 2 (dll730 0x02EC). The stone stands where it is placed, by the start, and once dropped sets
//   its placement's +0x1E (0x2E8) when the player is within 105 of it (dll730 0x01A0).
// - Map 43's CFseqobject 30CCA plays sequence 0x18 on 0x49A and sets 0x49F when it ends; 0x49F also hides the stone
//   (its placement's +0x20). Bonehead (CFGuardian, dll369 0x14DC) moves from state 12 to 13 on 0x49A, where the
//   SpellStone used on him starts its placing. Nothing else reads 0x49A, and nothing sets it.
//
// What Restored changes: as CRCloudRace sees the stone taken (its state going to 4), 0x49A is set.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string cloud_race = "cloud_race";

    enum class CloudRace {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> cloud_race_options = {
        { CloudRace::Original, "Original" },
        { CloudRace::Restored, "Restored" },
    };

    bool restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(cloud_race)) ==
            (uint32_t)CloudRace::Restored;
    }

    const bool logging = std::getenv("DINO_LOG_CLOUDRUNNER") != nullptr;
    constexpr int32_t set_bit = 0x80014670;
    constexpr int race_won = 0x49A;
}

void dino::add_cloud_race_options(recomp::config::Config& general) {
    general.add_enum_option(
        cloud_race,
        "CloudRunner Race",
        "Below CloudRunner Fortress, taking the SpellStone the SnowClaw drops wins the snow race: its closing scene "
        "plays and Bonehead hears of it. <b>Original is the prototype as it was left</b>: the race ends with the stone "
        "and nothing follows.",
        cloud_race_options,
        CloudRace::Restored
    );
}

// us.toml's hook in CRCloudRace's race (dll729 0x0380), as it sees 0x2E8 set and makes its state 4.
extern "C" void dino_cloud_race_won(uint8_t* rdram, recomp_context* ctx) {
    if (logging) {
        fprintf(stderr, "cloudrunner: race won\n");
    }
    if (!restored()) {
        return;
    }
    recomp_context saved = *ctx;
    ctx->r29 -= 0x40;
    ctx->r4 = race_won;
    ctx->r5 = 1;
    LOOKUP_FUNC(set_bit)(rdram, ctx);
    *ctx = saved;
}

// us.toml's hook in CRSnowClaw's hit test (dll728 0x0440) with DINO_LOG_CLOUDRUNNER, as a ram counts: $t8 the rams
// it takes yet before it falls, below 0 at the fifth.
extern "C" void dino_snow_claw_rammed(uint8_t* rdram, recomp_context* ctx) {
    if (logging) {
        fprintf(stderr, "cloudrunner: SnowClaw rammed, %d to go\n", (int8_t)ctx->r24 + 1);
    }
}
