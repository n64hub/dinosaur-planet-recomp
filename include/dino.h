#ifndef __DINO_H__
#define __DINO_H__

#include <cstdint>
#include <string_view>

#include "recomp.h"

namespace dino {
    inline constexpr std::u8string_view program_id = u8"DinosaurPlanetPrototype";
    inline constexpr std::string_view program_name = "Dinosaur Planet (prototype)";

    // The game's code sections (register_overlays.cpp) and the patches over it (register_patches.cpp).
    void register_overlays();
    void register_patches();

    // --story, --place, --bits, --report and --watch-bits (story.cpp, development).
    bool story_choose(const char* name);
    void story_add_bits(const char* list);
    void story_watch_bits(const char* list);
    void story_set_warp(int index);
    void story_set_place(const char* list);
    void story_start(uint8_t* rdram, recomp_context* ctx);
    void story_report(uint8_t* rdram, recomp_context* ctx, double seconds);
    void story_timed_bits(uint8_t* rdram, recomp_context* ctx, double seconds);

    // The settings (config.cpp): RecompFrontend's tabs.
    void init_config();
}

#endif
