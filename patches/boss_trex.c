// BOSS TRex's arena (map 48), the fight for the third SpellStone, reached from Walled City's warp (entry 90): the
// fixes that let it be won, always on (the floor switches' pads and the TRex's corners and facing, below, and the
// player's collision after the intro, us.toml beside dll210_0934), and its progression fix, which us.toml's hook in the
// TRex's last state calls with BOSS TRex on Restored (src/game/boss_trex.cpp).

#include "patches.h"

extern void func_80014670(s32 bit, s32 value); // sets a bit

// The TRex's last state (dll702_2D98, the state table's eleventh entry) ends the fight by calling func_80013FB4,
// which loads map 1, the front end (func_800141A4(1, 0, 1, -1)): the prototype's end of the stretch, with no
// SpellStone given and no way back. Walled City's SpellStone is carried as bit 0x83B (to the Volcano Force Point's
// second SpellPlace, dll637, which places it as 0x878), and the arena has its own way back, a WarpPoint (dll275, at
// 1428,0,-1833 beyond the player's door) to entry 91 by the WarpPoint Walled City sends the player in from, shown by
// bit 0x81C, which nothing sets. The player's door (KT_RexDoorPlaye, dll307) opens on bit 0x56E, which KT_RexLevel
// sets as the player comes in (dll701 0x114) and which reads 0 again once the intro has shut the player in. So the
// call is skipped, and the TRex's defeat gives the SpellStone, opens the door and shows the WarpPoint, which takes the
// player back to Walled City's corridor and clears 0x81C as it does.
#define THIRD_STONE_CARRIED_BIT 0x83B
#define RETURN_WARP_BIT 0x81C
#define PLAYER_DOOR_BIT 0x56E

void dino_trex_beaten(void) {
    func_80014670(THIRD_STONE_CARRIED_BIT, 1);
    func_80014670(RETURN_WARP_BIT, 1);
    func_80014670(PLAYER_DOOR_BIT, 1);
}

// What a patch reads of an object: its yaw, its position in its map and its hit data (obj+0x58), whose +0x10F counts
// what touches it this frame.
typedef struct {
    s16 yaw;
    u8 pad[0xC - 0x2];
    f32 x, y, z;
    u8 pad2[0x58 - 0x18];
    u8* hits;
} Object;

extern Object* func_80023914(void);                    // the player
extern void func_80026184(Object* object, Object* by); // adds a contact to the object's hit data

#define SWITCH_RADIUS 28.0f
#define SWITCH_HEIGHT 20.0f

// The arena's floor switches (KT_RexFloorSwit, dll703, eight, two at the ends of each corridor) charge their
// corridor's laser wall while the player stands on one: its update (dll703 0x298-0x2B8) keeps a count of 5 frames
// while its hit data has a contact (+0x10F) and its side is live, and charges the corridor's bit (0x56A-0x56D) a step
// each 10 frames of it, to 15, when the wall fires; a frame without one sets the bit back to 0 (0x6BC-0x6D4). The
// player's only contact is its floor check (dll27_214C), from the surface it stands on. But the switch's model has its
// collision mesh off its drawing: its top is the square 9 to 47 from its place in x and z, 5 up, with a slope to 0 and
// 56 on each side, so it lies 28 north-east of the pad drawn round the switch's place, and a player standing on the
// pad stands on the floor beside it and never charges it (the floor check under her found the map's floor alone).
// Here a player standing on the pad as it is drawn, within 28 of its place, is a contact, at the start of the
// switch's update (the hook at dll703_00A4's start); the floor check adds none twice.
void dino_trex_switch(Object* floor_switch) {
    Object* player = func_80023914();
    f32 dx, dy, dz;

    if (player == 0 || floor_switch->hits == 0) {
        return;
    }
    dx = player->x - floor_switch->x;
    dy = player->y - floor_switch->y;
    dz = player->z - floor_switch->z;
    if (dx * dx + dz * dz <= SWITCH_RADIUS * SWITCH_RADIUS && dy > -SWITCH_HEIGHT && dy < SWITCH_HEIGHT) {
        func_80026184(floor_switch, player);
    }
}

// The TRex runs the ring on a path of four corridors (dll702's path data, D_3F40 in the DLL): +0x8 is how far along
// its corridor it is, 0 to 1, +0xFA's bits 1-2 the corridor and bit 0 the way round, and +0xFC its speed (0 for a
// lap, 1 for a charge, 2 for the fast lap after a hit, dll702_3828). Its step (dll702_0FE4) moves the fraction on,
// and once it leaves the speed's range (dll702's data: from {0, 0.025, 0.025} at D_3E50 to {1, 0.975, 0.975} at
// D_3E5C) moves to the next corridor, holds the fraction at the range's end and returns 1, so the state calls the
// corner (dll702_3160), which sets the fraction from where the TRex stands on the new corridor (dll702_0C74), about 0
// (or about 1 the other way round). At speed 0 that is in range; at speeds 1 and 2 it can be under 0.025 (over 0.975),
// so the next step leaves the range at once and skips that corridor too, and the TRex goes round its corners in place:
// after the first hit it never reaches the end of its fast lap, where the hit counts (bit 0x572) and the next round's
// floor switches come on (dll701 0x254-0x434). Here the fraction is brought into the speed's range before each step
// (the hook at dll702_0FE4's start), as the step itself leaves it.
typedef struct {
    u8 pad[0x8];
    f32 fraction;
    u8 pad2[0xFA - 0xC];
    u16 flags;
    u8 speed;
} TrexPath;

void dino_trex_path(Object* trex, TrexPath* path) {
    f32 min = path->speed != 0 ? 0.025f : 0.0f;
    f32 max = path->speed != 0 ? 0.975f : 1.0f;

    if (path->fraction < min) {
        path->fraction = min;
    } else if (path->fraction > max) {
        path->fraction = max;
    }
}

// The TRex's run (act 1, dll702_2454, the state table's second entry) sets its place from the path (+0xE8, +0xF0) and
// never its yaw: only a corner (act 2, dll702_2644) turns it, a quarter turn over the corner's animation, and a
// turnaround (act 3, dll702_28BC) half a turn. A run that changes corridor with no corner played, as a charge that no
// wall stopped does at the corridor's end, keeps the facing it had, so it runs on sideways or backwards and every
// corner after turns from the wrong facing: a corner then swings it out of the ring, and a stun leaves its head, where
// a hit counts (its hit spheres 2 and 3, dll702 0x2010), beyond the corridor's outer wall, out of the player's reach.
// A corridor's facing is fixed: a quarter turn back for each corridor from the west one's 0 (south), and half a turn
// the other way round, as every corner and turnaround that played whole leaves it. Here the run faces its corridor
// (the hook at dll702_2454's start, the state in $a1 and its path data at +0x3F4).
#define TREX_PATH 0x3F4

void dino_trex_facing(Object* trex, u8* state) {
    TrexPath* path = *(TrexPath**)(state + TREX_PATH);

    if (path == 0) {
        return;
    }
    trex->yaw = (s16)(-0x4000 * ((path->flags >> 1) & 3) + ((path->flags & 1) ? 0x8000 : 0));
}
