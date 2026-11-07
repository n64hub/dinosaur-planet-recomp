// The moonseeds Krystal plants in Moon Mountain Pass, which nothing in the prototype gives.
//
// The task list has Sabre collect them for Krystal once the third SpellStone is placed ("COLLECTED MOONSEEDS FOR
// KRYSTAL", "SEARCH IN DARK, WET PLACES", ROM file 19), and Krystal plant one to reach the test of fear. Their count is
// bit 0x86A, 3 bits in bank 2 and so kept for both characters, which both inventories show (dll1's data, rows at +0x2B4
// and +0x440: 086A ffff 040D, the moonseed icon). Each of Moon Mountain Pass's four MMP_mseedrecept (dll722, map 18
// groups 2, 3, 4 and 6) takes one as the item is used beside it and lowers the count (0x350). Nothing raises it: no
// code but dll722 names 0x86A, no placement in any map holds it, the Swap Store's items (dll768) don't list it, and
// MMP_moonseed (object 0x54B) is a prop of dll214 placed in no map. So here the third SpellStone's placing gives four,
// one for each receptacle: VFP_SpellPlace (dll637 0x388) sets its placed bit, 0x878 for the third. On Third
// SpellStone's Original (acts.cpp) it gives none, as the prototype.

#include <cstdint>
#include <cstdio>

#include "recomp.h"
#include "dino.h"

namespace {
    constexpr int32_t set_bit = 0x80014670;
    constexpr int third_stone_placed = 0x878;
    constexpr int moonseeds = 0x86A;
    constexpr int receptacles = 4;
}

// us.toml's hook after VFP_SpellPlace sets its placed bit (dll637 0x388), its state in $s0 with the bit at +0.
extern "C" void dino_moonseeds_stone_placed(uint8_t* rdram, recomp_context* ctx) {
    if (MEM_H(0, ctx->r16) != third_stone_placed || !dino::third_stone_restored()) {
        return;
    }
    recomp_context saved = *ctx;
    ctx->r29 -= 0x40;
    ctx->r4 = moonseeds;
    ctx->r5 = receptacles;
    LOOKUP_FUNC(set_bit)(rdram, ctx);
    *ctx = saved;
}
