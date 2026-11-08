// The game's objects freed while another still holds them, or read before they are made. us.toml's hooks call these.
//
// func_80022F94 frees an object: it calls the object's own free, unloads its DLL, clears the links other objects keep
// to it (+0x30, +0xC0, and the state of objects of class 0x10) and gives back its memory. It doesn't clear the player's
// hold. The player (dll210) keeps what it carries at +0x868 of its state, written by its message handler for 0x100010,
// which a SmallBasket or SmallCrate (dll295) sends as it is lifted, and its draw (dll210_307C) puts that object in the
// player's hands each frame, calling through it, with no test but for 0. A group a TriggerPlane turns off frees its
// objects (func_8004A800, through the deferred free of func_80022338), so a basket carried out of its group is freed in
// the player's hands, and the next frame's draw calls into the freed basket and its unloaded DLL. A warp frees every
// object at once, the player too, so it doesn't.
//
// The player's weapon, Krystal's staff or Sabre's sword (object 105 or 35 by the character at +0x8B4 of its state, the
// halfwords at +0x24 of the table dll210's GOT word 0x8 points at), is an object of its own at +0xC8 of the player,
// which the player's update (dll210_0934) makes when it has none, at 0x900E8C88, and its free (dll210_3E50) frees. That
// update runs the player's state machine (dll210_11A0, through dll18) before it gets there, so in a new player's first
// update the states run with no weapon. The state the A button's swing starts (dll210_18630) reads the weapon's +0x44
// as it starts, with no test: a swing pressed on the frame a player comes back after a death, or comes into a map, read
// address 0x44 and crashed. Pressed every other frame through falls into the Volcano Force Point's lava it crashed in
// the first run; a swing a frame later finds its weapon.
//
// An effect (dll13, each 0x90 bytes) keeps the object it belongs to in a table of its own, 16 bytes an entry, that the
// effect's +0x11A >> 9 indexes, and the object free doesn't clear that table. The effects' draw (dll13_1080) reads the
// object's +0x18 position and, for its parent at +0x30 (0 for none), the parent's first halfword: an effect drawn after
// its object is freed reads what the freed memory holds, and a word there that isn't an address read outside memory
// and crashed, about a second into the Desert Force Point's place's sequence (0x373) on some plays of it. The draw now
// takes a parent outside the game's memory for none.
//
// An object's def: func_80022AA4 takes a def's index, from the table at D_800B1908 by the object id (func_80021450,
// func_800212F4), and gives the def it loaded from ROM file 66, counting who holds it in a byte each (D_800B1904). It
// tests the index only against the number of defs (D_800B18F0), signed, and an object id whose def was taken out of
// the game has -1 there, so it reads the byte and the word before its tables: with the byte not 0 it gives that word,
// and with it 0 it loads a def from the offsets before file 66's own. Walled City places object 0x336, which has none,
// before each temple's gap (uids 41afc and 41b35): its first load made nothing of it, and the load
// a fall into the moon temple's pit plays read a byte of 0 and made the placement from what it loaded, and crashed in
// func_80026AB8 reading the table at that def's +0x24. An index outside the tables now gets 0, which both callers take
// for an object with no def and make nothing of.

#include <cstdio>
#include <cstdlib>

#include "recomp.h"

namespace {
    // 2 while the game plays; 0 while func_80020C48 frees every object at once.
    constexpr int32_t objects_live = (int32_t)0x80091660;
    // Each object list's start and end (two halfwords each) in the table of list entries; list 0 is the player.
    constexpr int32_t object_lists = (int32_t)0x800B28B0;
    constexpr int32_t object_list_entries = (int32_t)0x800B2938;
    // The number of object defs, which func_80022AA4 tests a def's index against.
    constexpr int32_t def_count = (int32_t)0x800B18F0;
}

// func_80022F94, about to free the object at $a0: a player that holds it lets go of it, as dll210's drop (its export
// at 0x9010511C) does: the held object at +0x868 and the byte at +0x870 cleared.
extern "C" void dino_object_freed(uint8_t* rdram, uint32_t obj) {
    if (MEM_W(0, objects_live) == 0) {
        return;
    }
    int32_t start = MEM_H(0, object_lists);
    int32_t end = MEM_H(2, object_lists);
    if (end <= start) {
        return;
    }
    gpr player = MEM_W(start * 4, object_list_entries);
    if (player == 0) {
        return;
    }
    gpr state = MEM_W(0xB8, player);
    if (state != 0 && (uint32_t)MEM_W(0x868, state) == obj) {
        MEM_W(0x868, state) = 0;
        MEM_B(0x870, state) = 0;
    }
}

// dll210_0934 at 0x900E8A70, before either of its calls to the state machine, with the player in $s2, its state in $s1
// and dll210's GOT in $gp: a player with no weapon gets it made there, as the update makes it later (0x900E8C94):
// func_800233F4 (GOT 0x38) sets up object 105 or 35, func_800213EC (GOT 0x3C) makes it with the player's +0x30 as its
// fifth argument, and it goes at +0xC8, where the update then finds it.
extern "C" void dino_player_weapon(uint8_t* rdram, recomp_context* ctx) {
    gpr player = ctx->r18;
    gpr state = ctx->r17;
    if (MEM_W(0xC8, player) != 0) {
        return;
    }
    recomp_context call = *ctx;
    call.r29 = ctx->r29 - 0x20;
    gpr table = MEM_W(0x8, ctx->r28) + 0x24;
    call.r4 = 0x18;
    call.r5 = MEM_H(MEM_BU(0x8B4, state) * 2, table);
    LOOKUP_FUNC(MEM_W(0x38, ctx->r28))(rdram, &call);
    call.r4 = call.r2;
    call.r5 = 4;
    call.r6 = -1;
    call.r7 = -1;
    call.r29 = ctx->r29 - 0x20;
    MEM_W(0x10, call.r29) = MEM_W(0x30, player);
    LOOKUP_FUNC(MEM_W(0x3C, ctx->r28))(rdram, &call);
    MEM_W(0xC8, player) = call.r2;
}

// dll13_1080 at 0x90034A34, which has read the parent of an effect's object (at its +0x30) into $v0 and goes on when
// it isn't 0, the object in $s3: a parent outside the game's memory is a freed object's, so none. DINO_LOG_EFFECTS set
// (development) prints each.
extern "C" void dino_effect_parent(uint8_t* rdram, recomp_context* ctx) {
    uint32_t parent = (uint32_t)ctx->r2;
    if (parent == 0 || (parent >= 0x80000000u && parent < 0x80800000u)) {
        return;
    }
    static const bool logging = std::getenv("DINO_LOG_EFFECTS") != nullptr;
    if (logging) {
        fprintf(stderr, "effects: object 0x%08X has parent 0x%08X, freed (its +0x44 %04X, +0x46 %04X)\n",
            (uint32_t)ctx->r19, parent, (uint16_t)MEM_H(0x44, ctx->r19), (uint16_t)MEM_H(0x46, ctx->r19));
    }
    ctx->r2 = 0;
}

// func_80022AA4, about to give the def of index $a0: an index outside its tables (-1, an object id with no def) gets
// 0, as a def that couldn't be loaded does.
extern "C" int dino_def_missing(uint8_t* rdram, recomp_context* ctx) {
    int32_t index = (int32_t)ctx->r4;
    if (index >= 0 && index < MEM_W(0, def_count)) {
        return 0;
    }
    ctx->r2 = 0;
    return 1;
}
