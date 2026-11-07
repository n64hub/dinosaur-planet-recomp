// A HitAnimator (dll353, object 0x24F) whose change is lost when its block is loaded again. Always on: a plain bug fix.
//
// A HitAnimator turns a group of its block's hit lines on or off by a bit: its setup's +0x18 the bit, +0x1B the group,
// +0x1C flags (1 the lines' state with the bit clear, 4 that it changes them). Its state's byte 0 is the state they
// take, byte 1 bit 4 a change still to make. Its setup (dll353.0) and its update (dll353.1, 0x0160) find its block by
// its position (func_8004454C, func_80044BB0) and, with the change to make, set or clear 0x800 on each of the block's
// lines of that group (dll353_03E8, 0x03E8) and clear bit 4. The update sets it again only when the block isn't loaded
// or the bit changes. But a block can be loaded again between two updates, and the new one has its lines as the ROM
// has them: Dragon Rock's bottom's (map 52) loads again when the player comes down the stairs from the top, and the
// HitAnimator at its west end (uid 0x412A9, 646,-6111, bit 0x95, group 2), which the top's DR_EarthCallPad leaves
// open, made its change to the block loaded before, so the wall it opens stands, and the way from the stairs to the
// Skeetla arena is shut (docs/story-points.md, Dragon Rock). Each update makes the change again: it only sets or clears
// a flag on each of its group's lines.

#include <cstdint>

#include "recomp.h"

// us.toml's hook at the start of dll353's update (0x0160): a0 the object.
extern "C" void dino_hit_animator(uint8_t* rdram, recomp_context* ctx) {
    gpr object = ctx->r4;
    gpr setup = (gpr)MEM_W(0x4C, object);
    gpr state = (gpr)MEM_W(0xB8, object);
    if (setup == 0 || state == 0 || !(MEM_BU(0x1C, setup) & 4)) {
        return;
    }
    MEM_B(1, state) = MEM_BU(1, state) | 4;
}
