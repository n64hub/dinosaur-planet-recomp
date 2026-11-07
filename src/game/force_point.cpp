// The Desert Force Point's way up to the second SpellStone's place, and Golden Plains leaving that place empty. The
// prototype's Golden Plains fills the place on every load, and no surface joins the force point's floor to the place
// (docs/story-points.md, Golden Plains and the Desert Force Point). With Desert Force Point on Restored (the default) a
// player can place the stone and go on to the swap to Sabre; on Original it plays as the prototype was left.
//
// What the ROM holds, read from it (offsets are a DLL's text; local means map 21's, world x +640, z +10240):
// - The place (DFP_SpellPlace, uid 35111) stands at y -1255, on a walkway joined to a closed duct at y -1249 that rings
//   the rooms: two C shapes, with zone planes, setup points and lights along them, so it is a route for the player.
//   Eighteen stubs leave the duct and open, with no collision on their mouths, high in the corridors' walls: a stub's
//   lip is at y -1286, the corridor's floor at -1459 and the slope under the lip stops at -1395. Nothing a player can
//   stand on reaches a lip, in any room.
// - The west corridor is behind DFP_RoundDoor (uid 3393d), which opens on 0x5E8, the end of the power's links, and its
//   stubs are the nearest the place. Standing in one, the duct leads to the place with no door.
// - DFPLift1 (dll641, object 0x42F) is the lift between the top and the bottom. It waits for the bit in its +0x20, comes
//   to a player within 230, carries one standing within 50, and is called back by a player above or below its
//   placement's y. It runs from that y + 60 to that y - 1228, the 1228 a constant of the DLL's.
//
// What Restored changes:
// - GP_LevelControl's update (dll547 0xC8) keeps 0x5F3 as it is: only the place's sequence 0x373 sets it, and the
//   stone (0x2E8) stays to be placed.
// - A second DFPLift1 stands in the west corridor, under the stub at local z -960, on 0x5E8. Its travel is the
//   corridor's: the lift's two reads of the constant (dll641 0x444, 0x528) give it its own, and the object starts at
//   the bottom. Being a lift both ways, a player who falls from the duct or the walkway gets back up.
// - The place's group (7 of map 21) is turned on by planes on the floor that this way up crosses neither of, so the
//   player being up in the duct turns it on.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string desert_force_point = "desert_force_point";

    enum class DesertForcePoint {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> desert_force_point_options = {
        { DesertForcePoint::Original, "Original" },
        { DesertForcePoint::Restored, "Restored" },
    };

    bool restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(desert_force_point)) ==
            (uint32_t)DesertForcePoint::Restored;
    }

    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t new_placement = 0x800233F4; // (size, object id): a zeroed placement record
    constexpr int32_t spawn_object = 0x800213EC;  // (placement, 5, map, -1, [sp+0x10] parent): the object
    constexpr int32_t map_structs = 0x800B5328;   // each loaded map's header, by map number (0 while not loaded)
    constexpr int32_t dll29_handle = 0x8008C9F4;  // its export at +0x4C reads (map, group), at +0x50 sets (.., value)
    constexpr int force_point_bottom = 21;
    constexpr int place_group = 7;
    constexpr int stone_placed = 0x5F3;
    constexpr int round_door_open = 0x5E8;
    constexpr int lift_object = 0x42F;
    constexpr float lift_x = -275.0f;
    constexpr float lift_z = 9280.0f;
    // The lift's y: its surface is 27 over it, level with the stub's lip at the top and the floor at the bottom.
    constexpr float lift_top = -1313.0f;
    constexpr float lift_bottom = -1486.0f;
    constexpr float above_placement = 60.0f; // DFPLift1's top, over its placement's y
    constexpr float duct_low = -1300.0f;     // y: between these, the player is up in the duct
    constexpr float duct_high = -1200.0f;

    // DINO_LOG_FORCE_POINT set (development): the lift's placing and the group's turning on are printed.
    const bool logging = std::getenv("DINO_LOG_FORCE_POINT") != nullptr;

    bool lift_placed = false;
    int32_t lift_placement = 0;

    void call(uint8_t* rdram, recomp_context* ctx, int32_t function) {
        LOOKUP_FUNC(function)(rdram, ctx);
    }

    int32_t word(uint8_t* rdram, int32_t address) {
        return (int32_t)MEM_W(0, (gpr)address);
    }

    float real(uint8_t* rdram, gpr address) {
        int32_t bits = MEM_W(0, address);
        float value;
        memcpy(&value, &bits, 4);
        return value;
    }

    void set_real(uint8_t* rdram, gpr address, float value) {
        int32_t bits;
        memcpy(&bits, &value, 4);
        MEM_W(0, address) = bits;
    }

    // A hook's calls run below the hooked function's frame, and put back every register it had.
    struct Calls {
        recomp_context* ctx;
        recomp_context saved;
        explicit Calls(recomp_context* ctx) : ctx(ctx), saved(*ctx) {
            ctx->r29 -= 0x40;
        }
        ~Calls() {
            *ctx = saved;
        }
    };

    void place_lift(uint8_t* rdram, recomp_context* ctx) {
        ctx->r4 = 0x24;
        ctx->r5 = lift_object;
        call(rdram, ctx, new_placement);
        gpr p = ctx->r2;
        MEM_B(2, p) = 0x24 / 4;
        set_real(rdram, p + 0x8, lift_x);
        set_real(rdram, p + 0xC, lift_top - above_placement);
        set_real(rdram, p + 0x10, lift_z);
        MEM_H(0x1E, p) = -1;
        MEM_H(0x20, p) = round_door_open;
        ctx->r4 = p;
        ctx->r5 = 5;
        ctx->r6 = force_point_bottom;
        ctx->r7 = -1;
        MEM_W(0x10, ctx->r29) = 0;
        call(rdram, ctx, spawn_object);
        gpr lift = ctx->r2;
        if (lift != 0) {
            lift_placement = (int32_t)p;
            set_real(rdram, lift + 0x10, lift_bottom);
            // Its state (+0xB8): idle (2) with no wait (+0xE), as the lift is once it has first risen. Its first state
            // rises by itself for a player within 230, waits at the top and is called down again before it takes one.
            gpr state = (gpr)MEM_W(0xB8, lift);
            MEM_H(0x8, state) = 2;
            MEM_H(0xE, state) = 0;
        }
        if (logging) {
            fprintf(stderr, "force point: lift placed (object 0x%08X)\n", (uint32_t)lift);
        }
    }

    void turn_place_group_on(uint8_t* rdram, recomp_context* ctx) {
        call(rdram, ctx, player_object);
        gpr player = ctx->r2;
        if (player == 0) {
            return;
        }
        float y = real(rdram, player + 0x1C);
        if (y < duct_low || y > duct_high) {
            return;
        }
        int32_t table = word(rdram, word(rdram, dll29_handle));
        ctx->r4 = force_point_bottom;
        ctx->r5 = place_group;
        call(rdram, ctx, word(rdram, table + 0x4C));
        if ((int32_t)ctx->r2 != 0) {
            return;
        }
        ctx->r4 = force_point_bottom;
        ctx->r5 = place_group;
        ctx->r6 = 1;
        call(rdram, ctx, word(rdram, table + 0x50));
        if (logging) {
            fprintf(stderr, "force point: group %d on\n", place_group);
        }
    }
}

void dino::add_force_point_options(recomp::config::Config& general) {
    general.add_enum_option(
        desert_force_point,
        "Desert Force Point",
        "The prototype's Golden Plains fills the second SpellStone's place on every load, and nothing leads up to it. "
        "<b>Restored</b> leaves the place empty until the stone is placed and adds a lift to the duct that leads there, "
        "in the corridor behind the round door. <b>Original is the prototype as it was left</b>.",
        desert_force_point_options,
        DesertForcePoint::Restored);
}

// us.toml's hook at GP_LevelControl's BIT_SET(0x5F3, 1) (dll547 0xC8).
extern "C" void dino_force_point_golden_plains(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return;
    }
    int value;
    {
        Calls calls(ctx);
        ctx->r4 = stone_placed;
        call(rdram, ctx, get_bit);
        value = (int32_t)ctx->r2;
    }
    ctx->r5 = value;
}

// us.toml's hooks in DFPLift1's update (dll641 0x444, 0x528), which has read its travel below the placement into $f12,
// and the placement into $t0.
extern "C" void dino_force_point_lift(uint8_t* rdram, recomp_context* ctx) {
    if (lift_placement != 0 && (int32_t)ctx->r8 == lift_placement) {
        ctx->f12.fl = lift_top - above_placement - lift_bottom;
    }
}

// us.toml's hook after func_80046BC8 places the loaded maps' objects, each frame.
extern "C" void dino_force_point_frame(uint8_t* rdram, recomp_context* ctx) {
    if (word(rdram, map_structs + 4 * force_point_bottom) == 0) {
        lift_placed = false;
        lift_placement = 0;
        return;
    }
    if (!restored()) {
        return;
    }
    Calls calls(ctx);
    if (!lift_placed) {
        lift_placed = true;
        place_lift(rdram, ctx);
    }
    turn_place_group_on(rdram, ctx);
}
