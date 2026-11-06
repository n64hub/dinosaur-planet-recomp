// BOSS TRex's arena (map 48), the fight for the third SpellStone, reached from Walled City's warp (entry 90): its
// progression fixes, which us.toml's hooks beside the arena's DLLs call, each with the registers of the function it
// is in.

#include "patches.h"

// What a patch reads of an object: its setup, the placement it was made from.
typedef struct {
    u8 pad[0x4C];
    u8* setup;
} Object;

extern s32 func_800147EC(s32 bit); // a bit's value
extern void func_80014670(s32 bit, s32 value); // sets a bit

// The arena is a ring of four corridors with a laser wall halfway along each (KT_Lazerwall, dll704: setup +0x1A the
// corridor's charge bit, +0x1C the charge at which its beam shows, +0x1E its fire bit) and a floor switch at each
// end (KT_RexFloorSwit, dll703: +0x1A the charge bit, +0x1C its enable bit). The two switches of one side are live
// while their enable bit is 2 (KT_RexLevel, dll701, swaps the sides): standing on one counts the charge up a step
// every 0.2 s, and at 15 the side swaps and the charge goes back to 0 (dll703 0x52C-0x6AC). A wall fires (120 frames,
// dll704 0x144-0x200) only while its fire bit is set with the charge at 15, and clears it when done; its whole update
// is skipped while the fire bit is clear (0x13C). KT_Rex (dll702) is stunned when its corridor's fire bit is set and
// it is within 150 of that corridor's wall (dll702 0xEF0, the bits at data +0xDC). Nothing in the ROM sets the fire
// bits 0x560-0x563: no object, DLL, trigger or sequence, so a charged wall never fired and the TRex could not be hurt.
// Here a wall sets its fire bit as its charge reaches 15, so a player who times the charge to the TRex passing the
// wall fires it, as task 82's hint has it ("IT'S ALL DOWN TO TIMING"). One wall of the north corridor (charge bit
// 0x56D, at 640,96,-1468) names 0x573 as its fire bit where the corridor's other three walls and its light
// (KT_Lazerlight, dll705) name 0x563: it takes 0x563. The hook is at the start of the wall's update (dll704_0090).
#define WALL_CHARGED 15
#define NORTH_FIRE_BIT 0x563
#define NORTH_FIRE_BIT_MISTYPED 0x573

void dino_trex_wall(Object* wall) {
    s16* fire_bit = (s16*)(wall->setup + 0x1E);
    s16 charge_bit = *(s16*)(wall->setup + 0x1A);

    if (*fire_bit == NORTH_FIRE_BIT_MISTYPED) {
        *fire_bit = NORTH_FIRE_BIT;
    }
    if (func_800147EC(charge_bit) >= WALL_CHARGED && !func_800147EC(*fire_bit)) {
        func_80014670(*fire_bit, 1);
    }
}

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
