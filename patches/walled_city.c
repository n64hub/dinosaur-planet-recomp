// Walled City's progression fixes (map 13) that take more than a hook's line: us.toml's hooks beside the city's DLLs
// call these, each with the registers of the function it is in.

#include "patches.h"

// What a patch reads of an object: its position in its map, its hit volume and its state.
typedef struct {
    u8 pad[0xC];
    f32 x, y, z;
    u8 pad2[0x54 - 0x18];
    void* hit;
    u8 pad3[0xB8 - 0x58];
    void* state;
} Object;

// A pressure switch's state (WCPressureSwitc, dll781): what stands on it, and where each stood when last seen.
typedef struct {
    u8 pad[0x8];
    Object* on[10];
    struct {
        f32 x, z;
    } at[10];
} PressureSwitch;

#define PRESSURE_SWITCH_SLOTS 10

// The city's two pressure switches arm its beacons (0x7ED, 0x7EE). Each frame the switch's update (dll781 0x1E0-0x270)
// adds every object touching it from above to its list with dll781_05DC, and dll781_0648 then drops each one that has
// moved since it was added: the switch is down while any stays where it was. The add never looks for the object
// already in the list, and its search for an empty slot has no end (0x5E4-0x618; it also passes over slot 9), so a
// player standing still fills a slot a frame, and from the tenth frame on writes past the list's ten into the object
// after it: its hit data, whose model index (+0xA0 of obj+0x54) then reads -58, and the next collision pass
// (func_80053750 0x800538F8, from the player's dll27) reads a model through it and crashes. Placing the player on a
// switch crashes the same way. Here the add is done in its place: an object already in the list has where it stands
// updated, a new one takes the first empty slot of the ten, and one that finds none is left out (the switch is down
// already). Returns 1, so the hook at dll781_05DC's start returns from it.
s32 dino_wc_switch_add(Object* pressure_switch, Object* object) {
    PressureSwitch* state = pressure_switch->state;
    s32 slot = -1;
    s32 i;

    for (i = 0; i < PRESSURE_SWITCH_SLOTS; i++) {
        if (state->on[i] == object) {
            slot = i;
            break;
        }
        if (state->on[i] == 0 && slot < 0) {
            slot = i;
        }
    }
    if (slot >= 0) {
        state->on[slot] = object;
        state->at[slot].x = object->x;
        state->at[slot].z = object->z;
    }
    return 1;
}

// What a patch reads of an object's hit volume (obj+0x54): its radius and the heights it spans, from the object's
// place up.
typedef struct {
    u8 pad[0x52];
    s16 radius;
    s16 bottom, top;
} HitVolume;

#define BEACON_RADIUS 60
#define BEACON_TOP 160

// The sun and moon beacons (WCBeacon, dll780, at 110,-793,-4092 and 1810,-793,-5508) light when hit while their
// pressure switch holds them ready (dll780 0x1A8, func_80025F40: any attack of any kind), and each lit one opens one
// of the slab door's two locks (WCLevelControl's sequence, dll779 0x12F8: 0x7F7, 0x802). Their hit volume from the
// object definition is a radius of 10 from 5 below their place to 10 above it, at y -793, and the dais the player
// stands on beside them is at y -749: the volume is inside the stone the beacon stands on, so no sword, fireball
// (projball, dll255) or flame (flameblast, dll278) ever reaches it, and the city can't be opened. Here a beacon's
// volume takes in the stone up to its sun, 160 above its place and 60 out, before its update looks for a hit (the
// hook at the start of dll780_011C).
void dino_wc_beacon(Object* beacon) {
    HitVolume* volume = beacon->hit;

    if (volume != 0 && volume->top < BEACON_TOP) {
        volume->radius = BEACON_RADIUS;
        volume->top = BEACON_TOP;
    }
}
