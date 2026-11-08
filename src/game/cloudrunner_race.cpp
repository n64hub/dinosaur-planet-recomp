// CloudRunner's race (map 43): the snow bike's wall test at the port's frame rate. The race starts in a bay whose west
// edge the start's sequence launches the bike over, onto a course west and north of it, and CRSnowClaw, whose rams drop
// the SpellStone, stands in the bay, so the race is won only by riding back over that edge (docs/story-points.md,
// CloudRunner Fortress). At 2 ticks a frame, the 30 fps this recomp holds, the edge stopped the bike dead wherever it
// came at it, in tens of plays; in a frame of 3 ticks or more (20 fps or slower) it rides over. The console's rate in
// the race isn't known: 3 ticks is the fastest that lets the bike over, so a shorter frame is tested as one of 3.
// Always on, as a plain bug: the race can't be won without it.
//
// Read from the ROM (offsets are dll732's text):
// - CRSnowBike's ridden state (0x07C4) moves it once a tick (0x0C44-0x0D70, dll732_2340 as many times as D_8008C950),
//   each tick adding gravity to its speed and moving it (0x26D0-0x2720, 0x2974-0x29AC), so before the frame's ground
//   snap it has dropped 0.1956 * k(k+1)/2 below the ground over a frame of k ticks: 0.59 at 2, 1.17 at 3.
// - Only on the frame's last tick (0x29C0) does it test walls (dll732_3AF8) and snap to the ground (dll27.3-.5). The
//   wall test casts five points, each its table's place at D_4E44 (12 bytes each, y 1.0 at +4) above the bike, from
//   where they were saved last frame (state +0x330, saved at 0x3D70, before the snap) to where they are now
//   (func_80059C40), and puts the bike back at a hit. A wall counts only where the point's height is within it, from 1
//   above its lowest corner (func_8005A3F8, 0x8005A800-0x8005A8B8).
// - So the saved points sit 0.41 above the ground at 2 ticks and 0.17 below it at 3, and an edge whose wall's lowest
//   corner is 0.59 to 1.17 below the ground catches every frame of 2 ticks and none of 3: the pocket's east edge, the
//   way back into the bay, is one.
//
// The fix: us.toml's hook before the wall test's transform of each point (0x3C08) lowers the point's height (a2) in a
// frame of fewer than 3 ticks by what that frame drops less than one of 3, so the test sees what it sees at 3 ticks.
// A frame of 3 or more is left as it is.

#include <cstdint>
#include <cstring>

#include "recomp.h"

namespace {
    constexpr int32_t tick_count = 0x8008C950; // the frame's length in ticks, a byte (D_8008C950)

    // How far below the ground the bike has dropped by a frame's ground snap, over k ticks.
    float dropped(int k) {
        return 0.1956f * k * (k + 1) / 2.0f;
    }
}

extern "C" void dino_snow_bike_wall_point(uint8_t* rdram, recomp_context* ctx) {
    int k = MEM_BU(0, (gpr)tick_count);
    if (k >= 3 || k < 1) {
        return;
    }
    float y;
    uint32_t word = (uint32_t)ctx->r6;
    memcpy(&y, &word, 4);
    y -= dropped(3) - dropped(k);
    memcpy(&word, &y, 4);
    ctx->r6 = (gpr)(int32_t)word;
}
