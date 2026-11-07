// BOSS TRex's setting. The prototype's arena (map 48) can't be won: nothing fires its laser walls, so the TRex can't be
// hurt, and its defeat loads the front end with no SpellStone (patches/boss_trex.c has the evidence and the fixes).
// With BOSS TRex on Restored (the default) a charged wall fires and the defeat gives the third SpellStone and the way
// back to Walled City; on Original us.toml's hooks leave the arena as the prototype was left.

#include <cstdint>
#include <string>
#include <vector>

#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string boss_trex = "boss_trex";

    enum class BossTrex {
        Original,
        Restored,
        OptionCount
    };

    const std::vector<recomp::config::ConfigOptionEnumOption> boss_trex_options = {
        { BossTrex::Original, "Original" },
        { BossTrex::Restored, "Restored" },
    };
}

void dino::add_boss_trex_options(recomp::config::Config& general) {
    general.add_enum_option(
        boss_trex,
        "BOSS TRex",
        "The prototype's laser walls never fire, so the TRex can't be hurt, and beating it goes back to the title. "
        "<b>Restored</b> fires a wall once its corridor is charged, and the TRex's defeat gives the third SpellStone and "
        "opens the way back to Walled City. <b>Original is the prototype as it was left</b>.",
        boss_trex_options,
        BossTrex::Restored);
}

// us.toml's hooks in the arena's laser walls and the TRex's last state.
extern "C" int dino_trex_restored(void) {
    return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(boss_trex)) ==
        (uint32_t)BossTrex::Restored;
}
