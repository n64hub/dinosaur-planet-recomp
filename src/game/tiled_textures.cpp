// A texture too big for TMEM, drawn in 64-texel tiles: where its tiles load from. A plain bug fix, always on.
//
// dll76_18A8 draws such a texture tile by tile, a row of tiles at a time (the boot's sketched picture behind the
// Rareware and Nintendo logos: 320x192 I8, 5 tiles across and 3 down). Each tile's mesh is a 63-texel square of the
// picture's: its corners 63 texels apart on the screen, where the next tile's start. So the tiles overlap by a texel,
// each one's last row and column the next one's first, which a filtered texture needs to meet its neighbour with no
// seam: tile (c, r) shows the image's texels 63c to 63c + 63 across and 63r to 63r + 63 down.
//
// For each tile the function loads 64x64 texels into TMEM and sets the tile the mesh draws with, both from s = the
// tile's number counted from the first (its $s5) times 64 and t = 63 times its row ($s4), and the mesh's texture
// coordinates follow the same count, 64 a tile: s from 64 times the tile's number, t from 64r. So the tile's t (63r)
// is r less than the mesh's, and the load's address is the image's at (s, t), where an s past the image's width runs
// on into its next row: a tile in row r loaded from r rows below 63r, and each column from 64c, not 63c. On a console
// as well the picture skipped a texel at every column of tiles and jumped one, then two, at the rows, where the mesh
// read past what was loaded, which a window's resolution shows as steps through the drawing (the stones and the
// figure's head).
//
// us.toml calls dino_tiled_texture_rows where a tile's commands are written: it loads the tile from the texels it
// shows, (63c, 63r) on (its width and height less a texel, times c and r), and moves the tile's t to the mesh's, 64r
// (its height times r); the tile's s is already the mesh's. A list that isn't those seven commands, or a tile not at
// 63r, is left alone.

#include <cstdint>

#include "recomp.h"

namespace {
    // What each tile's commands are, from the first: settimg, settile (the load's tile 7), loadsync, loadtile,
    // pipesync, settile (the tile drawn with), settilesize. The display list pointer is past the last.
    constexpr uint8_t tile_commands[] = { 0xFD, 0xF5, 0xE6, 0xF4, 0xE7, 0xF5, 0xF2 };
    constexpr int load_command = 3;
    constexpr int size_command = 6;

    // dll76_18A8's locals: the display list pointer's address in $s3, the row in $s4, and the column at 0x100($sp).
    constexpr int32_t column_slot = 0x100;

    // A loadtile's or settilesize's coordinates, each in 10.2: s and t of the first texel in its first word, of the
    // last in its second, with the tile's number above them.
    uint32_t first_word(uint8_t opcode, uint32_t s, uint32_t t) { return (uint32_t(opcode) << 24) | (s << 12) | t; }
    uint32_t last_word(uint32_t tile, uint32_t s, uint32_t t) { return (tile << 24) | (s << 12) | t; }
}

extern "C" void dino_tiled_texture_rows(uint8_t* rdram, recomp_context* ctx) {
    gpr end = (gpr)MEM_W(0, ctx->r19);
    gpr first = end - 8 * (gpr)sizeof(tile_commands);
    for (int i = 0; i < (int)sizeof(tile_commands); i++) {
        if (((uint32_t)MEM_W(8 * i, first) >> 24) != tile_commands[i]) {
            return;
        }
    }
    gpr load = first + 8 * load_command;
    gpr size = first + 8 * size_command;
    uint32_t load_first = (uint32_t)MEM_W(0, load), load_last = (uint32_t)MEM_W(4, load);
    uint32_t size_first = (uint32_t)MEM_W(0, size), size_last = (uint32_t)MEM_W(4, size);
    uint32_t uls = (load_first >> 12) & 0xFFF, ult = load_first & 0xFFF;
    uint32_t lrs = (load_last >> 12) & 0xFFF, lrt = load_last & 0xFFF;
    uint32_t row = (uint32_t)ctx->r20;
    uint32_t column = (uint32_t)MEM_W(column_slot, ctx->r29);
    uint32_t width = lrs - uls + 4, height = lrt - ult + 4;
    if (lrs < uls || lrt < ult || ult != row * (height - 4) || size_first != first_word(0xF2, uls, ult)) {
        return;
    }
    uint32_t s = column * (width - 4), t = row * (height - 4);
    MEM_W(0, load) = (int32_t)first_word(0xF4, s, t);
    MEM_W(4, load) = (int32_t)last_word(load_last >> 24, s + width - 4, t + height - 4);
    uint32_t mesh_t = row * height;
    MEM_W(0, size) = (int32_t)first_word(0xF2, uls, mesh_t);
    MEM_W(4, size) = (int32_t)last_word(size_last >> 24, (size_last >> 12) & 0xFFF, mesh_t + height - 4);
}
