// Example mod: hooks a function of the main segment and one of a DLL the game loads as it plays, by their names in
// syms/dinoplanet.syms.toml, and calls a game function. What each address is comes from syms/names.toml.
#include "dino_api.h"

// bit_get (syms/names.toml): the value of one of the story's bits.
s32 func_800147EC(s32 bit);

// Runs as the game enters bit_set (bit, value), before the bit changes: prints the bits whose value it changes.
RECOMP_HOOK("func_80014670") void hello_on_bit_set(s32 bit, s32 value) {
    s32 old = func_800147EC(bit);
    if (old != value) {
        recomp_printf("[hello mod] bit 0x%X: %d -> %d\n", bit, old, value);
    }
}

// Runs as the player's DLL (dll210, loaded once a game starts) enters its update, export 1, once a frame for Krystal
// or Sabre: fills their magic (state +0x34C points at their stats, +4 the magic, +6 the most it holds).
RECOMP_HOOK("dll210_0934") void hello_on_player_update(u8* player) {
    u8* state = *(u8**)(player + 0xB8);
    s16* stats = *(s16**)(state + 0x34C);
    if (stats != NULL && stats[2] < stats[3]) {
        recomp_printf("[hello mod] magic %d -> %d\n", stats[2], stats[3]);
        stats[2] = stats[3];
    }
}
