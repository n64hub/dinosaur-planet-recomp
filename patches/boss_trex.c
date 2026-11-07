// BOSS TRex's arena (map 48), the fight for the third SpellStone, reached from Walled City's warp (entry 90): its
// progression fix, which us.toml's hook in the TRex's last state calls.

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
