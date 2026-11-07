// The settings: RecompFrontend's General, Controls, Graphics, Sound and Mods tabs, with its default bindings. Its own
// Graphics options carry what a recomp needs: Resolution and MS Anti-Aliasing (high resolution), Framerate
// (interpolation to the display's rate) and Aspect Ratio and HUD Placement (widescreen), with its stock bindings. General
// adds the free camera's options (src/game/camera.cpp), which also change the pad's default bindings.

#include <filesystem>

#include "recompui/config.h"
#include "util/file.h"
#include "dino.h"

void dino::init_config() {
    std::filesystem::path app_dir = recompui::file::get_app_folder_path();
    if (!app_dir.empty()) {
        std::filesystem::create_directories(app_dir);
    }

    recompui::config::GeneralTabOptions general_options{};
    general_options.has_rumble_strength = true;
    general_options.has_gyro_sensitivity = true;
    general_options.has_mouse_sensitivity = false;
    recomp::config::Config& general = recompui::config::create_general_tab(general_options);
    dino::add_camera_options(general);
    dino::add_krazoa_options(general);
    dino::add_drakor_options(general);
    dino::add_dragon_rock_options(general);
    dino::add_force_point_options(general);
    dino::add_cloudrunner_options(general);
    recompui::config::create_controls_tab();
    recompui::config::create_graphics_tab();
    recompui::config::create_sound_tab();
    recompui::config::create_mods_tab();
    recompui::config::finalize();
}
