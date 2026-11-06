// The game's DLLs (tools/gen_syms.py's header says what one is): the runtime maps a DLL's section where the game loaded
// it, and takes it out again before the game frees it. us.toml hooks the game's loader and unloader to call these.
//
// Each DLL is a relocatable section of its own in syms/, at a made-up address. load_overlays() maps the sections whose
// ROM lies in the range it is given at that range's RAM: the DLL's whole file, read from ROM file 70, holds its
// section, so the runtime puts the section's functions in its table of addresses at the DLL's text, and sets the
// section's address, which its $gp setups (HI16/LO16 relocations to its GOT) add to. The game then reaches its
// functions through its exports, its GOT and the pointers it hands out, all addresses in the memory it loaded it to.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>

#include "recomp.h"

extern "C" void load_overlays(uint32_t rom, int32_t ram_addr, uint32_t size);
extern "C" void unload_overlays(int32_t ram_addr, uint32_t size);

namespace {
    // Where ROM file 70, the DLLs, starts: the file table's base (0xA4AA0) plus its offset there.
    constexpr uint32_t dll_file_rom = 0x038317CC;

    // DINO_LOG_DLLS set (development): each load and unload is printed, a DLL by its offset in file 70 (tools/gen_syms.py
    // gives its number).
    const bool log_dlls = std::getenv("DINO_LOG_DLLS") != nullptr;

    // The size of each DLL loaded, by the address it was loaded to, for its unload.
    std::unordered_map<int32_t, uint32_t> loaded;
}

// func_8000C3CC, once it has read and relocated a DLL: $s0 is where, and its offset in file 70 and its size (bss not
// included) are on its stack, at 0x30 and 0x2C.
extern "C" void dino_dll_loaded(uint8_t* rdram, recomp_context* ctx) {
    int32_t ram = (int32_t)ctx->r16;
    uint32_t offset = MEM_W(0x30, ctx->r29);
    uint32_t size = MEM_W(0x2C, ctx->r29);
    // A DLL loaded at the same address as one the game freed without our hearing (none should be) replaces it.
    if (auto it = loaded.find(ram); it != loaded.end()) {
        unload_overlays(ram, it->second);
    }
    if (log_dlls) {
        fprintf(stderr, "DLL load: file 70 offset 0x%X at 0x%08X\n", offset, (uint32_t)ram);
    }
    load_overlays(dll_file_rom + offset, ram, size);
    loaded[ram] = size;
}

// func_8000C258, as it frees a DLL: $a0 is where it was loaded.
extern "C" void dino_dll_unloading(uint8_t* rdram, recomp_context* ctx) {
    int32_t ram = (int32_t)ctx->r4;
    auto it = loaded.find(ram);
    if (it == loaded.end()) {
        fprintf(stderr, "Dinosaur Planet: a DLL at 0x%08X freed that was never loaded\n", (uint32_t)ram);
        return;
    }
    if (log_dlls) {
        fprintf(stderr, "DLL unload: at 0x%08X\n", (uint32_t)ram);
    }
    unload_overlays(ram, it->second);
    loaded.erase(it);
}
