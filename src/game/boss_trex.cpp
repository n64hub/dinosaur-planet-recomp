// BOSS TRex's setting. In the prototype's arena (map 48) the TRex's defeat loads the front end with no SpellStone and
// no way back. With BOSS TRex on Restored (the default) the defeat gives the third SpellStone and opens the way back to
// Walled City; on Original us.toml's hook makes the prototype's call. The fixes that let the fight be won at all
// (patches/boss_trex.c has them and their evidence) are on whichever it is.

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
        "In the prototype, beating the TRex goes back to the title with no SpellStone. <b>Restored</b> gives the third "
        "SpellStone and opens the way back to Walled City. <b>Original is the prototype as it was left</b>.",
        boss_trex_options,
        BossTrex::Restored);
}

// us.toml's hook in the TRex's last state.
extern "C" int dino_trex_restored(void) {
    return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(boss_trex)) ==
        (uint32_t)BossTrex::Restored;
}
