// A texture too big for TMEM, drawn in 64-texel tiles: the rows its tiles loaded. A plain bug fix, always on.
//
// dll76_18A8 draws such a texture tile by tile, a row of tiles at a time (the boot's sketched picture behind the
// Rareware and Nintendo logos: 320x192 I8, 5 tiles across and 3 down). For each it loads the tile into TMEM and sets
// the tile the mesh draws with, both from s = the tile's number counted from the first (its $s5) times the tile's
// width, and t = 63 times its row ($s4). The mesh's s follows the same count, so across a row it draws right. But the
// load's address is the image's at (s, t), and an s past the image's width runs on into its next row: a tile in row r
// loaded from r rows below the t it was given. The mesh's t are the image's own rows, 64 a row of tiles, so with the
// tile at 63r the drawn rows were 64r + r to 64r + 63 + r, the last r of them past what was loaded: at the seams
// between the rows of tiles the picture jumped a texel and then two (on a console as well, where a texel is two of the
// 480 lines), which a window's resolution shows as a step through the drawing.
//
// us.toml calls dino_tiled_texture_rows where a tile's commands are written: it moves its load to the column it is in
// and to the rows the mesh reads, 64r on (its height times r), and the tile's t with it; the tile's s stays as the mesh
// has it. A list that isn't those seven commands, or a tile not at 63r, is left alone.

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
    uint32_t s = column * width, t = row * height;
    MEM_W(0, load) = (int32_t)first_word(0xF4, s, t);
    MEM_W(4, load) = (int32_t)last_word(load_last >> 24, s + width - 4, t + height - 4);
    MEM_W(0, size) = (int32_t)first_word(0xF2, uls, t);
    MEM_W(4, size) = (int32_t)last_word(size_last >> 24, (size_last >> 12) & 0xFFF, t + height - 4);
}
