// The ground under the player stays loaded while a cutscene's camera looks from elsewhere. A plain bug fix, always on.
//
// The game loads the world's blocks around the camera, not the player: dll2 hands the camera's position to
// func_80046B58 (D_800B97AC, D_800B97B4), and once it crosses a cell (640 units) func_80046BC8 recentres its 16 by 16
// grid of cells on the camera's (D_80092A68 and D_80092A6C its corner, the camera's cell at 7, 7). For each of its five
// grids (layers) func_80047404 marks the cells to load around that cell with -3, four boxes the map's data gives each
// cell (at 0x80046F9C), and the loop at 0x80047178 loads each marked cell (func_800485FC) and lets the others go. In
// play the camera stands a few hundred units behind the player, so his cell is the camera's or one its boxes hold.
// A sequence's camera can cut to a cell whose boxes leave his out, and the player, still standing and still
// falling where nothing holds him, drops through the block that just went: Warlock Mountain's sequence 9 (seqpoint
// 297b, "Father.", act 5 with Krazoa Shrines on Restored) cuts 6.2 s in, at its time 0x176, from beside him in the
// corridor at (13995, 13, 1610) to the guards' room below it (the seqpoint + (207, -89, 667), its camera track 1769
// in ROM file 13), whose cell (22, 3) loads (22, 2), (22..24, 3) and (22, 4) only; the corridor's (21, 2) went, he
// fell out of the world, and came back at the last checkpoint in the laser hall. The streaming is the ROM's, so the
// console would drop him too (not played there).
//
// dino_streaming_keep_player_cell, us.toml's hook at 0x80047178, marks the player's cell too in each grid that
// marks the camera's own: the block he stands on stays, and a cell with no block in that grid loads nothing
// (func_800485FC fails and the loop marks it -2, as for any other). Nothing changes while his cell is marked
// already, as it is whenever the camera follows the player. A cutaway loads at most one block a grid more, the one the
// player stood on the frame before, well within the 80 the block table holds (us.toml, func_80041E84).

#include <cmath>
#include <cstdint>
#include <cstring>

#include "recomp.h"

namespace {
    constexpr int32_t player_object = 0x80023914;
    constexpr gpr grid_corner_x = (gpr)(int32_t)0x80092A68;
    constexpr gpr grid_corner_z = (gpr)(int32_t)0x80092A6C;
    constexpr float cell_size = 640.0f;
    constexpr int8_t marked = -3;

    float real(uint8_t* rdram, gpr address) {
        int32_t bits = MEM_W(0, address);
        float value;
        memcpy(&value, &bits, 4);
        return value;
    }
}

// $sp's +0x70 points at this grid's word in the table of the five grids' marks (0x800B96D0), 16 by 16 bytes, a row of
// 16 a cell of z. The hook runs as the loop starts: func_80046BC8 has set every cell to -2 and func_80047404 marked
// its boxes -3.
extern "C" void dino_streaming_keep_player_cell(uint8_t* rdram, recomp_context* ctx) {
    gpr marks = (gpr)(int32_t)MEM_W(0, (gpr)(int32_t)MEM_W(0x70, ctx->r29));
    if (marks == 0 || (int8_t)MEM_B(7 * 16 + 7, marks) != marked) {
        return;
    }
    recomp_context saved = *ctx;
    ctx->r29 -= 0x40;
    LOOKUP_FUNC(player_object)(rdram, ctx);
    gpr player = ctx->r2;
    *ctx = saved;
    if (player == 0) {
        return;
    }
    float x = real(rdram, player + 0x18);
    float z = real(rdram, player + 0x20);
    int gx = (int)floorf(x / cell_size) - (int32_t)MEM_W(0, grid_corner_x);
    int gz = (int)floorf(z / cell_size) - (int32_t)MEM_W(0, grid_corner_z);
    if (gx < 0 || gx >= 16 || gz < 0 || gz >= 16) {
        return;
    }
    MEM_B(gz * 16 + gx, marks) = marked;
}
