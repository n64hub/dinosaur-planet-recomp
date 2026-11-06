#include "dino.h"
#include "../../RecompiledFuncs/recomp_overlays.inl"

#include "librecomp/overlays.hpp"

// The game's code sections, from N64Recomp's section table (syms/, tools/gen_syms.py): the main segment, which the
// runtime maps at start where the IPL3 copies it (the MB after ROM 0x1000, at 0x80000400), and the DLLs, which are
// mapped where the game loads them as it plays (src/game/dlls.cpp).
void dino::register_overlays() {
    recomp::overlays::overlay_section_table_data_t sections {
        .code_sections = section_table,
        .num_code_sections = ARRLEN(section_table),
        .total_num_sections = num_sections,
    };

    recomp::overlays::overlays_by_index_t overlays {
        .table = overlay_sections_by_index,
        .len = ARRLEN(overlay_sections_by_index),
    };

    recomp::overlays::register_overlays(sections, overlays);
}
