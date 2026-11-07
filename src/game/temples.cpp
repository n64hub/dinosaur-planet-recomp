// Walled City's sun and moon temples, the third SpellStone's stretch: the night the Moon Stone shows in, in the map's
// act 2 (acts.cpp).
//
// The moon temple stands north of the east courtyard (its group 7, which the TriggerPlane 4130e at the courtyard's west
// entrance turns on). The courtyard's push-block puzzle, solved by night, counts 0x811 to 4, and WCLevelControl (dll779)
// sets 0x813 then; the seqobject 41458 plays the temple's door on it and sets 0x25D. The lift at the foot of the stairs
// (WCMoonTempleLif, dll786, 413a7) carries Sabre up to the ring at the top while he stands on it (its floor's type
// 0x21, dll210.70), waits 300 ticks and carries him down again. At the top the moon symbol (WCApertureSymbo, dll787,
// 41463) takes the moonlight: with 0x25D set as it loads, or after the door's sequence, its update (dll787_00FC) brightens
// it while Sabre stands on that floor within 200 of it in the first person view (C-up, camera mode 0x56) and it is on
// screen, by the time of day between 77000 and 81500 s (21:23 to 22:38; the sun's symbol 66000 to 74000). Above 200
// (its setup's +0x1A) it sets 0x829 (+0x1E), and the seqobject 41474 plays the moon in the ring and opens the way in
// at the foot of the stairs, setting 0x244.
//
// The lift's top stands 200.25 from the symbol, so the moon never shows on Original. On Restored the reach is 210.

#include <cstdint>

#include "recomp.h"
#include "dino.h"

namespace {
    constexpr float symbol_reach = 210.0f;
}

// us.toml's hook in WCApertureSymbo's update (dll787_00FC 0x1F0), before it compares the player's distance ($f0) with
// its reach ($f4).
extern "C" void dino_temples_symbol_reach(uint8_t* rdram, recomp_context* ctx) {
    if (dino::third_stone_restored()) {
        ctx->f4.fl = symbol_reach;
    }
}
