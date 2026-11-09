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

    // --story, --place, --bits, --report, --watch-bits, --time and --sidekick (story.cpp, development).
    bool story_choose(const char* name);
    void story_add_bits(const char* list);
    void story_watch_bits(const char* list);
    void story_set_warp(int index);
    void story_set_place(const char* list);
    void story_forget();
    void story_start(uint8_t* rdram, recomp_context* ctx);
    void story_report(uint8_t* rdram, recomp_context* ctx, double seconds);
    void story_timed_bits(uint8_t* rdram, recomp_context* ctx, double seconds);
    void story_set_time(const char* arg);
    void story_timed_time(uint8_t* rdram, recomp_context* ctx, double seconds);
    void story_set_sidekick(const char* name);
    void story_timed_sidekick(uint8_t* rdram, recomp_context* ctx, double seconds);
    bool story_steer(uint8_t* rdram, recomp_context* ctx, int oid, int sphere, float to_x, float to_z, float reach,
        bool ride, float* stick_x, float* stick_y, int match = -1, int match_oid = 0, int match_from = 0);
    // --input's sync near: whether a drawn object of that id is within reach of (x, z).
    bool story_object_near(uint8_t* rdram, recomp_context* ctx, int oid, float x, float z, float reach);
    // --input's sync act: whether a drawn object of that id is in that act of its state machine.
    bool story_object_act(uint8_t* rdram, int oid, int act);
    // --input's sync state: whether an object of that id within reach of (x, z) has that value in a byte of its state.
    bool story_object_state(uint8_t* rdram, int oid, float x, float z, float reach, int offset, int mask, int value);
    // --input's sync hour: the time of day in hours, -1 with no sky loaded.
    float story_hours(uint8_t* rdram, recomp_context* ctx);
    // --input's menu lines: the menu the game shows (9 on a game over).
    int story_menu(uint8_t* rdram);
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
    // SwapStone Hollow's Tricky's Run setting (swapstone_hollow.cpp).
    void add_swapstone_hollow_options(recomp::config::Config& general);
    // The Krazoa shrines', Cape Claw's Krazoa tablets' and Krazoa Palace's settings (krazoa.cpp), and whether the
    // palace's is on Restored (acts.cpp).
    void add_krazoa_options(recomp::config::Config& general);
    bool krazoa_palace_restored();
    // Drakor's fight's end setting (drakor.cpp).
    void add_drakor_options(recomp::config::Config& general);
    // The Kamerian Dragon's fight's end setting (dragon_rock.cpp).
    void add_dragon_rock_options(recomp::config::Config& general);
    void add_force_point_options(recomp::config::Config& general);
    // CloudRunner Fortress's treasure lift setting (cloudrunner.cpp).
    void add_cloudrunner_options(recomp::config::Config& general);
    // CloudRunner Fortress's wind bridge setting (cloudrunner_barrels.cpp), and whether the player object carries
    // something.
    void add_cloudrunner_barrel_options(recomp::config::Config& general);
    bool player_carries(uint8_t* rdram, int32_t player);
    // BOSS TRex's fight's setting (boss_trex.cpp).
    void add_boss_trex_options(recomp::config::Config& general);
    // The third SpellStone's setting (acts.cpp), and whether it is on Restored (moonseeds.cpp).
    void add_third_stone_options(recomp::config::Config& general);
    bool third_stone_restored();
    // Walled City's restored tasks' setting (walled_city.cpp).
    void add_walled_city_options(recomp::config::Config& general);
    // The right stick of --input (src/main/main.cpp, development).
    void set_scripted_camera(float x, float y);
}

#endif
