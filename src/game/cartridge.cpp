// Reads of the cartridge the game makes straight from its address on the bus, which the runtime doesn't map: the hooks
// in us.toml read the ROM the runtime holds instead.
#include <cstdint>
#include <span>

#include "librecomp/game.hpp"

// The big-endian word at offset `offset` of the ROM.
extern "C" uint32_t dino_rom_word(uint32_t offset) {
    std::span<const uint8_t> rom = recomp::get_rom();
    if (offset + 4 > rom.size()) {
        return 0;
    }
    return (uint32_t{rom[offset]} << 24) | (uint32_t{rom[offset + 1]} << 16) | (uint32_t{rom[offset + 2]} << 8) |
           uint32_t{rom[offset + 3]};
}
