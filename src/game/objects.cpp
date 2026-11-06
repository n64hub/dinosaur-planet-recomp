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

#include "recomp.h"

namespace {
    // 2 while the game plays; 0 while func_80020C48 frees every object at once.
    constexpr int32_t objects_live = (int32_t)0x80091660;
    // Each object list's start and end (two halfwords each) in the table of list entries; list 0 is the player.
    constexpr int32_t object_lists = (int32_t)0x800B28B0;
    constexpr int32_t object_list_entries = (int32_t)0x800B2938;
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
