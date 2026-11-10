#ifndef __DINO_API_H__
#define __DINO_API_H__

#include "modding.h"
#include "types.h"

// What Dinosaur Planet: AI Slop Recomp gives every mod (README, Mods), from the executable (src/game/recomp_api.cpp)
// and the runtime. The game's own functions are not here: call or hook them by their names in
// syms/dinoplanet.syms.toml, and read its data by address.

// printf to the console.
RECOMP_IMPORT("*", int recomp_printf(const char* fmt, ...));

// Memory that lasts until freed, from the runtime's heap.
RECOMP_IMPORT("*", void* recomp_alloc(unsigned long size));
RECOMP_IMPORT("*", void recomp_free(void* memory));

#endif
