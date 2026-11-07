#ifndef __DINO_H__
#define __DINO_H__

#include <cstdint>
#include <string_view>

#include "librecomp/config.hpp"
#include "recomp.h"

namespace dino {
    inline constexpr std::u8string_view program_id = u8"DinosaurPlanetPrototype";
    inline constexpr std::string_view program_name = "Dinosaur Planet: AI Slop Recomp";

    // The game's code sections (register_overlays.cpp) and the patches over it (register_patches.cpp).
    void register_overlays();
    void register_patches();

    // --story, --place, --bits, --report and --watch-bits (story.cpp, development).
    bool story_choose(const char* name);
    void story_add_bits(const char* list);
    void story_watch_bits(const char* list);
    void story_set_warp(int index);
    void story_set_place(const char* list);
    void story_forget();
    void story_start(uint8_t* rdram, recomp_context* ctx);
    void story_report(uint8_t* rdram, recomp_context* ctx, double seconds);
    void story_timed_bits(uint8_t* rdram, recomp_context* ctx, double seconds);
    bool story_steer(uint8_t* rdram, recomp_context* ctx, int oid, float to_x, float to_z, float reach, float* stick_x, float* stick_y);
    // DINO_CHECK_HEAP (heap.cpp, development).
    void heap_check(uint8_t* rdram, double seconds);

    // The settings (config.cpp): RecompFrontend's tabs.
    void init_config();

    // The free camera (camera.cpp).
    void init_camera();
    void add_camera_options(recomp::config::Config& general);
    void add_mouse_buttons(uint16_t* buttons);
    // Which way the camera looked at its last tick, on the ground (x, z), false before its first.
    bool camera_forward(float* x, float* z);
    // The Krazoa shrines' and Cape Claw's Krazoa tablets' settings (krazoa.cpp).
    void add_krazoa_options(recomp::config::Config& general);
    // Drakor's fight's end setting (drakor.cpp).
    void add_drakor_options(recomp::config::Config& general);
    // The right stick of --input (src/main/main.cpp, development).
    void set_scripted_camera(float x, float y);
}

#endif
