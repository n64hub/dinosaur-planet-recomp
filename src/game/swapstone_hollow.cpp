// SwapStone Hollow's setting. Tricky's run to his mother, the Queen Earthwalker, which task 10's clue points to, waits
// on bit 0xC3, which nothing in the ROM sets. With Tricky's Run on Restored (the default) us.toml's hook in
// SHseqobject's update has patches/swapstone_hollow.c set it as the player comes near; on Original it never starts, as
// on the console. The hooks that keep SHtricky from crashing with no Tricky out (us.toml beside dll501) and the
// hollow's other fixes are on whichever it is.

#include <cstdint>
#include <string>
#include <vector>

#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string tricky_run = "tricky_run";

    enum class TrickyRun {
        Original,
        Restored,
        OptionCount
    };

    const std::vector<recomp::config::ConfigOptionEnumOption> tricky_run_options = {
        { TrickyRun::Original, "Original" },
        { TrickyRun::Restored, "Restored" },
    };
}

void dino::add_swapstone_hollow_options(recomp::config::Config& general) {
    general.add_enum_option(
        tricky_run,
        "Tricky's Run",
        "In the prototype, Tricky never runs to his mother in SwapStone Hollow, the run the Queen Earthwalker's clue "
        "asks the player to watch. <b>Restored</b> starts it as the player comes near. <b>Original is the prototype "
        "as it was left</b>.",
        tricky_run_options,
        TrickyRun::Restored);
}

// us.toml's hook in SHseqobject's update.
extern "C" int dino_tricky_run_restored(void) {
    return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(tricky_run)) ==
        (uint32_t)TrickyRun::Restored;
}
