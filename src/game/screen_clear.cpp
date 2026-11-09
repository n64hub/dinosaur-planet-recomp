// The rows above and below the 3D picture, cleared to black each frame. func_80002130 gives the world's viewport the
// frame's rows from 6 + D_8008C524 to its height less that (D_8008C524 is a cutscene's letterbox), and func_80037A14,
// which starts each frame's list, fills the colour image only for a full clear (its flag 1, the menus') or, outside the
// viewport, while D_8008C528 is set (a cutscene's bars, in the game's clear colour). In play neither is, so the 6 rows
// at the top and the bottom are never drawn and keep whatever a screen before left in that framebuffer: broken tiles,
// half of the boot screens' "PUBLISHED BY" line. The VI shows all 240 rows (func_8005D75C, libultra's mode), so a
// console has them too, mostly in a TV's overscan; a window shows all of it. A plain bug fix, always on.
//
// us.toml calls dino_clear_borders where func_80037A14 has no full clear to do, after it set the colour image to the
// framebuffer in fill mode: it fills the rows outside the viewport black, as the game's own bars are filled, before
// anything else of the frame, and a cutscene's bars still fill them in the game's colour after it. Their right side is
// the frame's width, as the widescreen hook beside it makes the game's, so RT64 takes them to the window's sides.

#include <cstdint>

#include "recomp.h"

namespace {
    // func_80037A14's locals, on its stack: the frame's width and height (func_8005D6C8) and the viewport's first row
    // and the row after its last (func_80002130).
    constexpr int32_t frame_width_slot = 0x70;
    constexpr int32_t frame_height_slot = 0x6C;
    constexpr int32_t viewport_top_slot = 0x64;
    constexpr int32_t viewport_bottom_slot = 0x5C;

    // The game's record of the RDP's state, which func_80041728 reads to skip a fill colour already set: the colour at
    // +0x20, and at +0x28 whether a fill was drawn since the last pipe sync, which a change of colour sends first.
    constexpr gpr rdp_state_pointer = (gpr)(int32_t)0x80092A90;
    constexpr int32_t fill_colour_offset = 0x20;
    constexpr int32_t fill_drawn_offset = 0x28;

    constexpr uint32_t opcode_pipe_sync = 0xE7u << 24;      // G_RDPPIPESYNC
    constexpr uint32_t opcode_fill_colour = 0xF7u << 24;    // G_SETFILLCOLOR
    constexpr uint32_t opcode_fill_rectangle = 0xF6u << 24; // G_FILLRECT
    // Black with its coverage bit, twice: RGBA5551 as the game packs its own (func_80037A14 at 0x80037C6C).
    constexpr uint32_t black = 0x00010001;

    // gfx_pointer is the address of the display list pointer, which moves past the command.
    void write_command(uint8_t* rdram, gpr gfx_pointer, uint32_t high, uint32_t low) {
        gpr gfx = (gpr)MEM_W(0, gfx_pointer);
        MEM_W(0, gfx) = (int32_t)high;
        MEM_W(4, gfx) = (int32_t)low;
        MEM_W(0, gfx_pointer) = (int32_t)((uint32_t)gfx + 8);
    }

    // Rows top to bottom, both inclusive, as fill mode takes them.
    void write_fill(uint8_t* rdram, gpr gfx_pointer, int32_t right, int32_t top, int32_t bottom) {
        write_command(rdram, gfx_pointer, opcode_fill_rectangle | ((right & 0x3FF) << 14) | ((bottom & 0x3FF) << 2),
            (top & 0x3FF) << 2);
    }
}

// gfx_pointer is func_80037A14's $s0, sp its $sp.
extern "C" void dino_clear_borders(uint8_t* rdram, gpr gfx_pointer, gpr sp) {
    int32_t width = MEM_W(frame_width_slot, sp);
    int32_t height = MEM_W(frame_height_slot, sp);
    int32_t top = MEM_W(viewport_top_slot, sp);
    int32_t bottom = MEM_W(viewport_bottom_slot, sp);
    if (top <= 0 && bottom >= height) {
        return;
    }

    gpr state = (gpr)MEM_W(0, rdp_state_pointer);
    if (MEM_BU(fill_drawn_offset, state) != 0) {
        write_command(rdram, gfx_pointer, opcode_pipe_sync, 0);
    }
    write_command(rdram, gfx_pointer, opcode_fill_colour, black);
    if (top > 0) {
        write_fill(rdram, gfx_pointer, width, 0, top - 1);
    }
    if (bottom < height) {
        write_fill(rdram, gfx_pointer, width, bottom, height - 1);
    }
    MEM_W(fill_colour_offset, state) = (int32_t)black;
    MEM_B(fill_drawn_offset, state) = 1;
}
