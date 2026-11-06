// The game's objects freed while another still holds them. us.toml's hooks call these.
//
// func_80022F94 frees an object: it calls the object's own free, unloads its DLL, clears the links other objects keep
// to it (+0x30, +0xC0, and the state of objects of class 0x10) and gives back its memory. It doesn't clear the player's
// hold. The player (dll210) keeps what it carries at +0x868 of its state, written by its message handler for 0x100010,
// which a SmallBasket or SmallCrate (dll295) sends as it is lifted, and its draw (dll210_307C) puts that object in the
// player's hands each frame, calling through it, with no test but for 0. A group a TriggerPlane turns off frees its
// objects (func_8004A800, through the deferred free of func_80022338), so a basket carried out of its group is freed in
// the player's hands, and the next frame's draw calls into the freed basket and its unloaded DLL. A warp frees every
// object at once, the player too, so it doesn't.

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
