// The boot's sketched picture fills the frame's height. Always on.
//
// The two boot screens draw the picture behind their logos through dll76's export 2 (dll76_18A8): dll60 behind the
// Nintendo logo, dll61 behind Rareware's. Each call writes a list of its own: a viewport, a modelview matrix, the
// picture's mesh, and last the camera's projection for what comes after, so the mesh is drawn with that same
// projection, still loaded from the list before. The picture covers rows 62.4 to 428.8 of the 480, black above and
// below as the game clears it, and smaller still in a window. Asked to fill the height, the picture is drawn with a
// copy of the camera's projection narrowed to its rows, which enlarges it 1.31 times about its own middle: its sides,
// about 800 of 640 across, crop a little in 4:3 and show in full with Aspect Ratio Expand in 16:9. The logos are the
// screens' own rectangles and stay as they are.
//
// us.toml calls dino_boot_picture_begin before each screen's call, which puts the copy's load first in the list, and
// dino_boot_picture_end after it, which fills the copy from the projection the call loaded last.

#include <cstdint>

#include "recomp.h"

namespace {
    // The matrix the picture is drawn with, in the list the screen's call writes, and the command that loads a
    // projection (gSPMatrix(G_MTX_PROJECTION | G_MTX_LOAD)).
    gpr picture_matrix = 0;
    constexpr uint32_t projection_load = 0xDA380007;

    // The picture's top and bottom rows of the 480.
    constexpr float picture_top = 62.4f, picture_bottom = 428.8f;

    // An Mtx: the 16 elements' integer halves, two to a word, then their fractions.
    float element(uint8_t* rdram, gpr mtx, int row, int column) {
        int word = 4 * (2 * row + column / 2);
        int shift = column % 2 == 0 ? 16 : 0;
        int16_t whole = (int16_t)((uint32_t)MEM_W(word, mtx) >> shift);
        uint16_t fraction = (uint16_t)((uint32_t)MEM_W(32 + word, mtx) >> shift);
        return (float)(((int32_t)whole << 16) | fraction) / 65536.0f;
    }

    void set_element(uint8_t* rdram, gpr mtx, int row, int column, float value) {
        int word = 4 * (2 * row + column / 2);
        int shift = column % 2 == 0 ? 16 : 0;
        uint32_t keep = column % 2 == 0 ? 0x0000FFFF : 0xFFFF0000;
        uint32_t fixed = (uint32_t)(int32_t)(value * 65536.0f);
        MEM_W(word, mtx) = (int32_t)(((uint32_t)MEM_W(word, mtx) & keep) | ((fixed >> 16) << shift));
        MEM_W(32 + word, mtx) = (int32_t)(((uint32_t)MEM_W(32 + word, mtx) & keep) | ((fixed & 0xFFFF) << shift));
    }

    // In clip space the picture's rows are middle +- half: x and y are scaled by 1 / half and the middle moved to 0. A
    // vertex's clip y is its row times column 1, and its w is column 3, so column 1 becomes (column 1 - middle *
    // column 3) / half and column 0 is scaled with it. It reaches a line past the frame at the top and the bottom, or a
    // window's last row can round to black.
    void fill_height(uint8_t* rdram, gpr mtx) {
        float half = (picture_bottom - picture_top) / 482.0f;
        float middle = 1.0f - (picture_top + picture_bottom) / 480.0f;
        for (int row = 0; row < 4; row++) {
            set_element(rdram, mtx, row, 0, element(rdram, mtx, row, 0) / half);
            set_element(rdram, mtx, row, 1, (element(rdram, mtx, row, 1) - middle * element(rdram, mtx, row, 3)) / half);
        }
    }
}

// The list's first commands: a branch over a matrix (gSPBranchList), the matrix, and its load as the projection.
extern "C" void dino_boot_picture_begin(uint8_t* rdram, recomp_context* ctx) {
    gpr list = ctx->r4;
    gpr first = (gpr)MEM_W(0, list);
    picture_matrix = first + 8;
    MEM_W(0, first) = (int32_t)0xDE010000;
    MEM_W(4, first) = (int32_t)((uint32_t)(picture_matrix + 64) & 0x1FFFFFFF);
    MEM_W(64, picture_matrix) = (int32_t)projection_load;
    MEM_W(68, picture_matrix) = (int32_t)((uint32_t)picture_matrix & 0x1FFFFFFF);
    MEM_W(0, list) = (int32_t)(picture_matrix + 72);
}

// The projection the list loads last, the camera's, narrowed into the matrix; with none, the load is a no-op. The
// screen keeps the list's pointer's address at list_slot($sp).
extern "C" void dino_boot_picture_end(uint8_t* rdram, recomp_context* ctx, int32_t list_slot) {
    if (picture_matrix == 0) {
        return;
    }
    gpr end = (gpr)MEM_W(0, (gpr)MEM_W(list_slot, ctx->r29));
    gpr camera = 0;
    for (gpr command = picture_matrix + 72; command < end; command += 8) {
        if ((uint32_t)MEM_W(0, command) == projection_load) {
            camera = (gpr)(int32_t)((uint32_t)MEM_W(4, command) | 0x80000000);
        }
    }
    if (camera != 0) {
        for (int word = 0; word < 16; word++) {
            MEM_W(4 * word, picture_matrix) = MEM_W(4 * word, camera);
        }
        fill_height(rdram, picture_matrix);
    } else {
        MEM_W(64, picture_matrix) = 0;
        MEM_W(68, picture_matrix) = 0;
    }
    picture_matrix = 0;
}
