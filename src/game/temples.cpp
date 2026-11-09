// Walled City's sun and moon temples, the third SpellStone's stretch: the night the Moon Stone shows in, in the map's
// act 2 (acts.cpp).
//
// The moon temple stands north of the east courtyard (its group 7, which the TriggerPlane 4130e at the courtyard's west
// entrance turns on). The courtyard's push-block puzzle (four WCPushBlocks on WCLevelControl's 8 by 8 grid, dll779's
// exports 14, 18 and 20), solved by night, counts 0x811 to 4 (dll782 adds one as a block lands on its moon tile), and
// WCLevelControl sets 0x813 then; the seqobject 41458 plays the temple's door on it and sets 0x25D. The lift at the foot
// of the stairs (WCMoonTempleLif, dll786, 413a7) carries Sabre up to the ring at the top as he steps on it, waits 300
// ticks and carries him down again: its update (dll786_0030) moves only for a rider of object id 0, Sabre, standing on
// its floor's type 0x21 (dll210.70), so Krystal rides nothing. At the top the moon symbol (WCApertureSymbo, dll787,
// 41463) takes the moonlight: in its phase 1 (state +2) its update (dll787_00FC) brightens it while the player stands on
// that floor within 200 of it (the lift's top is 196 from it) in the first person view (C-up, camera mode 0x56), by the
// time of day between 77000 and 81500 s (21:23 to 22:38; the sun's symbol 66000 to 74000). How bright (dll787_0490) is
// 255 times how near the screen's middle the symbol is drawn, times how near the window's middle the time is, so the
// player looks up at it a little at about 22:00. Above 200 (its setup's +0x1A) it sets 0x829 (+0x1E), and the seqobject
// 41474 plays the moon in the ring and opens the way in at the foot of the stairs, setting 0x244. All of that is the
// ROM's own.
//
// The symbol's phase 1 comes only from its init, with its door's bit (setup +0x20, 0x25D; the sun's 0x25C) set as it
// loads, or from an event 1 of a sequence it acts in (dll787_043C), and the door's sequence (41458's 8) has it in no
// part: its actors are the door, the seqobject, the player and the camera. It loads with the courtyard's group 7,
// before the puzzle is solved, and stays loaded up the corridor and the lift, so a player who opens the door and stays
// finds the symbol dark however long they look, until it loads again. A plain bug, so always: its update takes the bit
// while it is loaded, as its init does as it loads.
//
// Each temple's stone lies across a gap from the way in, over a bridge of light (WCTempleBridge, dll790: the moon's
// 41659, 0x2D5, the sun's 414bc, 0x2D4) that shows once the dial's three ProjectileSwitches are hit in their order and
// the dial's seqobject has played. While shown it is solid (func_8002674C sets bit 0 of its hit record's +0x58), and
// the floor test (func_80053750 0x8005390C) collides with the model its definition names for that, slot 2 (0x3CB, flat
// at its height, 102 wide and 383 long). But its export 5 (dll790_04B8) returns 0x400 with the setup's model index
// (+0x19) at bit 11, which has func_80021450 (0x800216E4-0x8002174C) load that model alone, the one it draws: slot 2
// stays empty, the floor test passes it over, and the player falls through into the pit. A plain bug, so always: the
// export returns 0, as the moon lift's (dll786) does, so that all three of its models load; the bridge's init still
// draws the one its setup names (obj+0xAD).

#include <cstdint>

#include "recomp.h"
#include "dino.h"

namespace {
    constexpr int32_t get_bit = 0x800147EC;
}

// us.toml's hook at the end of WCTempleBridge's export 5 (dll790_04B8 0x4E4), the models to load in $v0.
extern "C" void dino_temples_bridge_models(uint8_t* rdram, recomp_context* ctx) {
    ctx->r2 = 0;
}

// us.toml's hook in WCApertureSymbo's update (dll787_00FC 0x120), the symbol in $a0: in phase 0, its phase 1 once its
// door's bit is set.
extern "C" void dino_temples_symbol_door(uint8_t* rdram, recomp_context* ctx) {
    gpr symbol = ctx->r4;
    gpr state = (gpr)(int32_t)MEM_W(0xB8, symbol);
    gpr setup = (gpr)(int32_t)MEM_W(0x4C, symbol);
    if (state == 0 || setup == 0 || MEM_BU(0x2, state) != 0) {
        return;
    }
    recomp_context saved = *ctx;
    ctx->r4 = (int16_t)MEM_H(0x20, setup);
    LOOKUP_FUNC(get_bit)(rdram, ctx);
    bool open = (int32_t)ctx->r2 != 0;
    *ctx = saved;
    if (open) {
        MEM_B(0x2, state) = 1;
    }
}
