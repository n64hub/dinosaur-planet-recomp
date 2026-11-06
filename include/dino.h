#ifndef __DINO_H__
#define __DINO_H__

#include <cstdint>
#include <string_view>

namespace dino {
    inline constexpr std::u8string_view program_id = u8"DinosaurPlanetPrototype";
    inline constexpr std::string_view program_name = "Dinosaur Planet (prototype)";

    // The game's code sections (register_overlays.cpp) and the patches over it (register_patches.cpp).
    void register_overlays();
    void register_patches();

    // The settings (config.cpp): RecompFrontend's tabs.
    void init_config();
}

#endif
