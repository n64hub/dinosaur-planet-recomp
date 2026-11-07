// SwapStone Hollow's progression fixes that take more than a hook's line: us.toml's hooks beside the hollow's others call
// these, each with the registers of the function it is in.

#include "patches.h"

// What a patch reads of an object: its position in the world.
typedef struct {
    u8 pad[0x18];
    f32 x, y, z;
} Object;

extern Object* func_80023914(void); // the player
extern Object* func_8002394C(void); // the sidekick, the first object of list 1, or 0 while none is out
extern s32 func_800147EC(s32 bit); // a bit's value
extern void func_80014670(s32 bit, s32 value); // sets a bit

#define TRICKY_RUN_BIT 0xC3
#define QUEEN_STATE_BIT 0xB0
#define TRICKY_RUN_RADIUS 300.0f
#define TRICKY_RUN_HEIGHT 200.0f

// Tricky's run to his mother, which task 10's clue ("watch where Tricky runs") points to. Bit 0xC3 (bank 0, so not
// saved) starts it: SHtricky (dll501, by the Queen) plays his sequence 3 on the sidekick when it is set, and SHseqobject
// 34841 (dll309, at 1175,-631,1257 in map 7, waiting on and setting 0xC3) plays its sequence 4 beside it. Both are in
// the map's group 6. Nothing sets 0xC3: no trigger, object or sequence in any map. SHtricky's setup takes the Queen's
// state bit 0xB0 being 0 to mean she hasn't been met (else it goes straight to its last state), and she writes it as
// soon as she loads (group 1), so this sets 0xC3 only while that is 0, with Tricky out, once the player comes within
// 300 of the sequence object. The hook is at the start of dll309's update, for a sequence object waiting on 0xC3.
void dino_tricky_run(Object* sequence_object) {
    Object* player;
    Object* tricky;
    f32 dx, dy, dz;

    if (func_800147EC(TRICKY_RUN_BIT) || func_800147EC(QUEEN_STATE_BIT)) {
        return;
    }
    player = func_80023914();
    tricky = func_8002394C();
    if (player == 0 || tricky == 0) {
        return;
    }
    dx = player->x - sequence_object->x;
    dy = player->y - sequence_object->y;
    dz = player->z - sequence_object->z;
    if (dx * dx + dz * dz < TRICKY_RUN_RADIUS * TRICKY_RUN_RADIUS && dy < TRICKY_RUN_HEIGHT && -dy < TRICKY_RUN_HEIGHT) {
        func_80014670(TRICKY_RUN_BIT, 1);
    }
}
