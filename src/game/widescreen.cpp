// Widescreen: what the game draws in rectangles, placed for a picture wider than 4:3. RT64's Aspect Ratio Expand widens
// the 3D world by itself; a rectangle stays where the game put it, in the 4:3 middle, unless a gEXSetRectAlign
// (rt64_extended_gbi.h) says otherwise. us.toml's hooks call these with the function's own display list pointer, and
// with Aspect Ratio Original none of it moves anything.
//
// dino_hud_side, around the parts of the HUD: the rectangles and scissors after it are placed from the picture's left
// or right edge, or as the game has them, and RT64 takes those edges from HUD Placement (Original keeps 4:3's). The
// parts are told by where the game draws them: dll59_0148, the minimap, at the bottom left; dll1_6B74's panel at x 22,
// y 176 under it; dll1_3D28's C button cluster at x 245-275, y 17-51 and dll1_27D8's column at x 253-282, y 121-180,
// with its scissored panels at x 262-292, on the right. Dialogue, subtitles and the rest of dll1 stay centred. Every
// change of side sends gEXEnable first and the frame's scissor last, so the list needs nothing else from RT64 and the
// part isn't cut off at 4:3's edge by the scissor the game set before it.

#include <initializer_list>

#include "recomp.h"

extern "C" void func_8005D6C8(uint8_t* rdram, recomp_context* ctx);

namespace {
    // gEXEnable on F3DEX2: RT64 reads none of these commands until a list has sent it, and forgets it when a list ends.
    constexpr uint32_t hook_magic = (0xE0u << 24) | 0x525464u; // RT64_HOOK_OPCODE, RT64_HOOK_MAGIC_NUMBER
    constexpr uint32_t hook_enable = (0x1u << 28) | 0x64u;     // RT64_HOOK_OP_ENABLE, RT64_EXTENDED_OPCODE
    constexpr uint32_t opcode_set_scissor = 0xEDu << 24;                     // G_SETSCISSOR
    constexpr uint32_t opcode_set_rect_align = (0x64u << 24) | 0x000006u;    // RT64_EXTENDED_OPCODE, G_EX_SETRECTALIGN_V1
    constexpr uint32_t opcode_set_scissor_align = (0x64u << 24) | 0x000008u; // G_EX_SETSCISSORALIGN_V1
    constexpr uint32_t origin_left = 0x0;                                    // G_EX_ORIGIN_LEFT
    constexpr uint32_t origin_right = 0x400;                                 // G_EX_ORIGIN_RIGHT
    constexpr uint32_t origin_none = 0x800;                                  // G_EX_ORIGIN_NONE

    // In quarter pixels: 4:3's width.
    constexpr int32_t screen_width = 320 * 4;

    uint32_t high_half(int32_t value) {
        return ((uint32_t)value & 0xFFFF) << 16;
    }

    // gfx_pointer is the address of the display list pointer, which moves past the command.
    void write_words(uint8_t* rdram, gpr gfx_pointer, std::initializer_list<uint32_t> words) {
        gpr gfx = (gpr)MEM_W(0, gfx_pointer);
        int32_t offset = 0;
        for (uint32_t word : words) {
            MEM_W(offset, gfx) = (int32_t)word;
            offset += 4;
        }
        MEM_W(0, gfx_pointer) = (int32_t)((uint32_t)gfx + offset);
    }

    void write_rect_align(uint8_t* rdram, gpr gfx_pointer, uint32_t origin, int32_t left, int32_t right) {
        write_words(rdram, gfx_pointer, { opcode_set_rect_align, origin | (origin << 12), high_half(left), high_half(right) });
    }

    // Each rectangle and scissor from here on is placed from side's edge (no bounds: the game's are never negative).
    // An alignment holds for the next scissor, so the frame's is sent again: frame_size is func_8005D6C8's.
    void write_side(uint8_t* rdram, gpr gfx_pointer, int side, uint32_t frame_size) {
        uint32_t origin = (side == 1) ? origin_left : (side == 2) ? origin_right : origin_none;
        // A right edge's x is then from the right edge, so 4:3's right edge is subtracted.
        int32_t offset = (side == 2) ? -screen_width : 0;
        write_words(rdram, gfx_pointer, { hook_magic, hook_enable });
        write_rect_align(rdram, gfx_pointer, origin, offset, offset);
        write_words(rdram, gfx_pointer, { opcode_set_scissor_align, origin | (origin << 12), high_half(offset),
            high_half(offset), 0, 0xFFFFFFFFu });
        uint32_t width = (frame_size & 0xFFFF) * 4;
        uint32_t height = (frame_size >> 16) * 4;
        write_words(rdram, gfx_pointer, { opcode_set_scissor, ((width & 0xFFF) << 12) | (height & 0xFFF) });
    }
}

// side: 0 as the game places it, 1 from the left edge, 2 from the right.
extern "C" void dino_hud_side(uint8_t* rdram, recomp_context* ctx, gpr gfx_pointer, int side) {
    recomp_context saved = *ctx;
    func_8005D6C8(rdram, ctx);
    uint32_t frame_size = (uint32_t)ctx->r2;
    *ctx = saved;
    write_side(rdram, gfx_pointer, side, frame_size);
}
