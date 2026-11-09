// CloudRunner Fortress (map 12): the way down the treasure shaft. Past CFBlastedWall the corridor opens onto a shaft
// whose bottom leads north into the treasure caves (map 15) and east to the drop into the race (map 43), and the
// shaft's wind lift, CFTreasWindLift, only ever starts the way up for a player who comes from the top: she hovers at
// the corridor's height and no button lets her down (docs/story-points.md, CloudRunner Fortress). With Treasure Lift on
// Restored (the default) a player who steps into it from the top is lowered to the bottom; on Original it plays as the
// console does. And its baby CloudRunners, whom Kyte never finds: with Baby CloudRunners on Restored (the default) she
// finds them, and on Original they stay hidden as in the prototype.
//
// The lift, read from the ROM (offsets are a DLL's text; map 12's placement at 0x51A4, at (-12902, 1840, 18886)):
// - CFTreasWindLift (dll370) runs while its on-bit, the placement's +0x1E (0x30D), is set, and blows the way its
//   direction bit, +0x1C (0x8C8), says: clear, up, holding the player about 213 above its base (0x0A84-0x0C34), level
//   with the corridor; set, down, letting her sink until she is under 20 above its base, where it lets her go (0x0D74).
//   Its update (0x00F8) marks the moment the wind starts, the latch at its state's +0x170 (0x0474-0x0494).
// - 0x30D is flipped by the TriggerPlanes around the shaft (dll325, params 0F 12 830D: either way, every crossing): in
//   the hole in the wall, on the east lip, and at the bottom's south and north (the north one in map 15). So the wind
//   blows while the player is inside the shaft.
// - Only the two CFWindSwitch levers (dll386) flip 0x8C8 (their sequence's callback, 0x038C), and both stand at the
//   bottom, about 165 below the corridor. Nothing on the top's side sets it, so a player who comes from the top always
//   finds the wind blowing up: a bridge across the shaft to the courtyard on its east, never the way down.
//
// What Restored changes: when the wind starts with the player at the top (more than 150 above the lift's base), 0x8C8
// is set, so the lift lowers her and lets her go at the bottom. Either lever there turns it back to up for the way
// back, as the ROM has it, and a player who steps in at the bottom rides up to the top and steps off on either side. A
// player who carries something (a barrel, for the courtyard roof: cloudrunner_barrels.cpp) is left to the wind.
//
// The baby CloudRunners (setting Baby CloudRunners), read from the ROM:
// - CFCloudBaby (dll373) keeps its state at its state's +0xAC, from its placement's +0x1C (or +0x20 once the bit at
//   +0x1E is set). Its rescue bit is the placement's +0x22 (0x24F-0x253): set, it loads hidden (+0x6's 0x4000).
// - Waiting to be found (states 0, 1 and 4) it asks Kyte, the sidekick (0x8002394C), every frame (0x2050): her export
//   24, her state's +0x1C8 & 0x200000. Yes brings it out to face her (state 5), and Krystal rescues it by pressing A
//   within its +0x1A (0x07A0): 0x901 counts it, its bit is set, and it flies to its golden perch (0x2FC-0x300). With
//   Krystal within its +0x18 (0x0410: 70 or 85) a waiting baby starts its call (sequence 1, in state 0).
// - A found baby asks again each frame (0x0924): no and it flies home and back to the state it had.
// - Kyte's flag is the curve node she flies to: as she reaches one (dll212_319C) her +0x1C8 takes its node type's flags
//   (dll212's D_48B8), and types 8, 9, 21 and 22 carry 0x200000. She flies along map curves of type 0x22 inside one
//   group, 0x46E (dll212_0CE8; a curve's +0x4, as dll25's export 12 reads it), which TriggerPlanes (action 0x1C),
//   FindKyteObject (dll720) and PerchObject (dll718) set. Each waiting baby has find nodes beside it in a group of
//   their own (236A 0x10, 154 away; 29FA 0x28 on its ledge, 70; 29B3 0x1B, 40), and nothing in the ROM sets 0x46E to
//   one of them, so no baby waiting to be found ever comes out. FindKyteObject 31858 by 236A sends her to 0xF, a node
//   of type 5.
//
// What Restored changes: as Krystal comes within a waiting baby's +0x18, 0x46E is set to the group of the find node
// nearest the baby (within 400; dll26's table holds the map's curves), and Kyte's own path code flies her there, where
// her flag brings it out. Once the baby is rescued 0x46E goes back to what it was, unless something has set it since.

#include <cstdint>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string treasure_lift = "treasure_lift";

    enum class TreasureLift {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> treasure_lift_options = {
        { TreasureLift::Original, "Original" },
        { TreasureLift::Restored, "Restored" },
    };

    bool restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(treasure_lift)) ==
            (uint32_t)TreasureLift::Restored;
    }

    const std::string cloud_babies = "cloud_babies";

    bool babies_restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(cloud_babies)) ==
            (uint32_t)TreasureLift::Restored;
    }

    const bool logging = std::getenv("DINO_LOG_CLOUDRUNNER") != nullptr;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t sidekick_object = 0x8002394C;
    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t dll26_handle = 0x8008C9E4;  // the curves' DLL: export 3 its table of the map's curves
    constexpr int kyte_group = 0x46E;
    constexpr int32_t rescued = 0xC;
    constexpr float find_node_reach = 400.0f;

    constexpr int32_t set_bit = 0x80014670;
    constexpr int wind_on = 0x30D;
    constexpr int wind_down = 0x8C8;
    constexpr float from_the_top = 150.0f;

    void call(uint8_t* rdram, recomp_context* ctx, int32_t function) {
        LOOKUP_FUNC(function)(rdram, ctx);
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

    float read_float(uint8_t* rdram, gpr address) {
        uint32_t bits = MEM_W(0, address);
        float value;
        memcpy(&value, &bits, sizeof(value));
        return value;
    }

    int32_t word(uint8_t* rdram, int32_t address) {
        return MEM_W(0, (gpr)address);
    }

    float distance(uint8_t* rdram, gpr a, gpr b) {
        float dx = read_float(rdram, a) - read_float(rdram, b);
        float dy = read_float(rdram, a + 4) - read_float(rdram, b + 4);
        float dz = read_float(rdram, a + 8) - read_float(rdram, b + 8);
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    // The group of the find node (a curve of type 0x22 whose node type, +0x1A, is 8, 9, 21 or 22) nearest the baby,
    // or -1 with none within find_node_reach. dll26's export 3 returns its table of the map's curves, 8 bytes each with
    // the curve's placement at +4, and writes their count to $a0's word (as dll25's export 12 reads it).
    int find_group(uint8_t* rdram, recomp_context* ctx, gpr baby) {
        Calls calls(ctx);
        gpr count_at = ctx->r29 + 0x10;
        ctx->r4 = count_at;
        call(rdram, ctx, word(rdram, word(rdram, word(rdram, dll26_handle)) + 4 + 4 * 3));
        gpr table = ctx->r2;
        int32_t count = (int32_t)MEM_W(0, count_at);
        int group = -1;
        float nearest = find_node_reach;
        for (int32_t i = 0; table != 0 && i < count; i++) {
            gpr curve = MEM_W(4 + 8 * i, table);
            if (curve == 0 || MEM_B(0x19, curve) != 0x22) {
                continue;
            }
            int type = (int8_t)MEM_B(0x1A, curve);
            if (!(type == 8 || type == 9 || type == 21 || type == 22)) {
                continue;
            }
            float d = distance(rdram, curve + 8, baby + 0x18);
            if (d < nearest) {
                nearest = d;
                group = MEM_HU(0x4, curve);
            }
        }
        return group;
    }

    int32_t get(uint8_t* rdram, recomp_context* ctx, int bit) {
        Calls calls(ctx);
        ctx->r4 = bit;
        call(rdram, ctx, get_bit);
        return (int32_t)ctx->r2;
    }

    void set(uint8_t* rdram, recomp_context* ctx, int bit, int32_t value) {
        Calls calls(ctx);
        ctx->r4 = bit;
        ctx->r5 = value;
        call(rdram, ctx, set_bit);
    }

    // The baby Kyte was sent to, the group she was sent to and what 0x46E held before.
    struct Sent {
        gpr baby = 0;
        int32_t group = 0;
        int32_t before = 0;
    } sent;

    float kyte_away(uint8_t* rdram, gpr kyte, gpr baby) {
        float dx = read_float(rdram, kyte + 0x18) - read_float(rdram, baby + 0x18);
        float dy = read_float(rdram, kyte + 0x1C) - read_float(rdram, baby + 0x1C);
        float dz = read_float(rdram, kyte + 0x20) - read_float(rdram, baby + 0x20);
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
}

void dino::add_cloudrunner_options(recomp::config::Config& general) {
    general.add_enum_option(
        treasure_lift,
        "Treasure Lift",
        "In CloudRunner Fortress the wind lift past the blasted wall lowers a player who steps in from the top, the way "
        "down to the treasure caves. <b>Original is the prototype as it was left</b>: from the top the wind only ever "
        "blows up, and the caves can't be reached.",
        treasure_lift_options,
        TreasureLift::Restored
    );
    general.add_enum_option(
        cloud_babies,
        "Baby CloudRunners",
        "In CloudRunner Fortress the baby CloudRunners Kyte's mother hid come out to be rescued once Kyte finds them. "
        "<b>Original is the prototype as it was left</b>: four of the five never come out.",
        treasure_lift_options,
        TreasureLift::Restored
    );
}

// us.toml's hook in CFTreasWindLift's update (dll370 0x0484) as its wind starts: $s3 its state (+0x4 the on-bit, +0x8
// the direction bit), $s1 its placement (+0xC its y), $s0 the player.
extern "C" void dino_treasure_lift_starts(uint8_t* rdram, recomp_context* ctx) {
    gpr state = ctx->r19, placement = ctx->r17, player = ctx->r16;
    if (!restored() || player == 0 || (int32_t)MEM_W(0x4, state) != wind_on || (int32_t)MEM_W(0x8, state) != wind_down ||
        dino::player_carries(rdram, (int32_t)player)) {
        return;
    }
    float base = read_float(rdram, placement + 0xC);
    float y = read_float(rdram, player + 0x1C);
    if (y - base <= from_the_top) {
        return;
    }
    Calls calls(ctx);
    ctx->r4 = wind_down;
    ctx->r5 = 1;
    call(rdram, ctx, set_bit);
}

// us.toml's hook at CFCloudBaby's update (dll373 0x01A0, $a0 the baby). On Restored, a waiting baby (state 0, 1 or 4,
// not hidden as a rescued one loads) with Krystal within its +0x18 sends Kyte to its find node's group (0x46E), and once
// it is rescued (state 0xC, as it flies to its perch, which then stops its update) 0x46E goes back. With DINO_LOG_CLOUDRUNNER, each baby's state as it
// changes, with how far Krystal is (in steps of 100) and Kyte (of 50), and whether she is on a find node.
extern "C" void dino_cloud_baby_update(uint8_t* rdram, recomp_context* ctx) {
    gpr baby = ctx->r4;
    int32_t state = (int32_t)MEM_W(0xAC, MEM_W(0xB8, baby));
    bool hidden = (MEM_H(0x6, baby) & 0x4000) != 0;
    if (babies_restored()) {
        gpr setup = MEM_W(0x4C, baby);
        gpr player;
        {
            Calls calls(ctx);
            call(rdram, ctx, player_object);
            player = ctx->r2;
        }
        bool waiting = !hidden && (state == 0 || state == 1 || state == 4);
        if (sent.baby == baby && state == rescued) {
            if (get(rdram, ctx, kyte_group) == sent.group) {
                set(rdram, ctx, kyte_group, sent.before);
            }
            if (logging) {
                fprintf(stderr, "cloudrunner: baby %08X rescued, 0x46E back to %X\n", (uint32_t)baby, sent.before);
            }
            sent = Sent{};
        } else if (waiting && player != 0 &&
            kyte_away(rdram, player, baby) < (float)(int16_t)MEM_H(0x18, setup)) {
            int group = find_group(rdram, ctx, baby);
            int32_t now = get(rdram, ctx, kyte_group);
            if (group >= 0 && now != group) {
                sent = Sent{ baby, group, sent.baby == baby ? sent.before : now };
                set(rdram, ctx, kyte_group, group);
                if (logging) {
                    fprintf(stderr, "cloudrunner: baby %08X calls, Kyte sent to group %X (0x46E was %X)\n",
                        (uint32_t)baby, group, now);
                }
            }
        }
    }
    if (!logging) {
        return;
    }
    static gpr babies[8];
    static int32_t seens[8];
    int i = 0;
    while (i < 8 && babies[i] != 0 && babies[i] != baby) {
        i++;
    }
    if (i == 8) {
        return;
    }
    gpr player, kyte;
    {
        Calls calls(ctx);
        call(rdram, ctx, player_object);
        player = ctx->r2;
        call(rdram, ctx, sidekick_object);
        kyte = ctx->r2;
    }
    float krystal_away = player != 0 ? kyte_away(rdram, player, baby) : -1.0f;
    float kyte_is = kyte != 0 ? kyte_away(rdram, kyte, baby) : -1.0f;
    // Kyte's export 24, her state's +0x1C8 & 0x200000: she is on a find node.
    bool finds = kyte != 0 && (MEM_W(0x1C8, MEM_W(0xB8, kyte)) & 0x200000) != 0;
    int32_t seen = state * 100000 + (int32_t)(krystal_away / 100.0f) * 1000 + (int32_t)(kyte_is / 50.0f) * 2 + finds;
    if (babies[i] == baby && seens[i] == seen) {
        return;
    }
    babies[i] = baby;
    seens[i] = seen;
    fprintf(stderr, "cloudrunner: baby %08X at %.0f %.0f %.0f state %d, Krystal %.0f away, Kyte %.0f at %.0f %.0f %.0f%s\n",
        (uint32_t)baby, read_float(rdram, baby + 0x18), read_float(rdram, baby + 0x1C), read_float(rdram, baby + 0x20),
        state, krystal_away, kyte_is, kyte != 0 ? read_float(rdram, kyte + 0x18) : 0.0f,
        kyte != 0 ? read_float(rdram, kyte + 0x1C) : 0.0f, kyte != 0 ? read_float(rdram, kyte + 0x20) : 0.0f,
        finds ? ", on a find node" : "");
}
