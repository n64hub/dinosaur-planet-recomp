// Moon Mountain Pass's part of the third SpellStone's stretch: the moonseeds Krystal plants, which nothing in the
// prototype gives, and the meteor in front of the test of fear's entrance, which nothing moves.
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
//
// The meteor (task 94, "MOVED METEOR TO REACH SHRINE ENTRANCE"): three MMP_moonrock (dll445, uids 425FA-425FC, in no
// group and every act) are carried and dropped into MMP_gyservent's vents (dll446, group 6); a rock dropped within 20 of
// a vent plugs it, and dll445_0628 counts the rocks in vents 3, 4 and 6 into 0x88C and sets 0x89B at three (0x9D4-0x9EC).
// The meteor's MMP_seqobject (dll309, uid 412AF) plays its sequence 5 on its trigger, 0x803, and sets its done bit
// 0x804, but nothing sets 0x803: no code, no placement and no event of ROM file 13 names it, and only a HitAnimator
// (uid 4267A, map animation 7) reads 0x89B. So with Third SpellStone on Restored, the three rocks in place set 0x803
// too, and the meteor's sequence plays.

#include <cstdint>
#include <cstdio>

#include "recomp.h"
#include "dino.h"

namespace {
    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t get_bit = 0x800147EC;
    constexpr int third_stone_placed = 0x878;
    constexpr int moonseeds = 0x86A;
    constexpr int receptacles = 4;
    constexpr int rocks_in_vents = 0x89B;
    constexpr int meteor_trigger = 0x803;
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

// us.toml's hook in dll445_0628 (0xA14), where it has set or cleared 0x89B by the rocks in vents 3, 4 and 6.
extern "C" void dino_moonseeds_meteor(uint8_t* rdram, recomp_context* ctx) {
    if (!dino::third_stone_restored()) {
        return;
    }
    recomp_context saved = *ctx;
    ctx->r29 -= 0x40;
    ctx->r4 = rocks_in_vents;
    LOOKUP_FUNC(get_bit)(rdram, ctx);
    bool in_vents = (int32_t)ctx->r2 != 0;
    ctx->r4 = meteor_trigger;
    LOOKUP_FUNC(get_bit)(rdram, ctx);
    if (in_vents && (int32_t)ctx->r2 == 0) {
        ctx->r4 = meteor_trigger;
        ctx->r5 = 1;
        LOOKUP_FUNC(set_bit)(rdram, ctx);
    }
    *ctx = saved;
}
