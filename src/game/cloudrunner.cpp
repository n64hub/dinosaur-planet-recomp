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
// player who carries something (a barrel, for the courtyard roof: cloudrunner_barrels.cpp) is left to the wind until
// the roof is blown (0x2E1), and lowered from then on: the roof needs no more barrels, and the treasure caves need one.
// There CFBlastedTunnel (dll389, map 15's 31B7A, by the treasure chest) takes only a hit of type 5, a CFbarrel's blast,
// and sets 0x330; CFExplodeTunnel (dll390, 31B76) then blows and sets 0x33A, which frees the chest (dll399, 31B9B) the
// GuardClaw beside it holds (dll399 0x0410-0x0468; no hit hurts the guard: dll217's damage table D_3058 is zeros), and
// A at the chest gives item 0x336, the master chest's (dll396), whose baby is the fifth. Map 15 places no barrel, and a
// CFbarrel comes back where it blew about 10 s later (dll388 0x039C-0x04B4), so one is carried down from the fortress.
// On Original a player carrying one falls to the bottom (cloudrunner_barrels.cpp), so there it reaches the caves.
//
// The baby CloudRunners (setting Baby CloudRunners), read from the ROM:
// - CFCloudBaby (dll373) keeps its state at its state's +0xAC, from its placement's +0x1C (or +0x20 once the bit at
//   +0x1E is set). Its rescue bit is the placement's +0x22 (0x24F-0x253): set, it loads hidden (+0x6's 0x4000).
// - Waiting to be found (states 0, 1 and 4) it asks Kyte, the sidekick (0x8002394C), every frame (0x2050): her export
//   24, her state's +0x1C8 & 0x200000. Yes brings it out to face her (state 5), and Krystal rescues it by pressing A
//   within its +0x1A (0x07A0): 0x901 counts it, its bit is set, and it flies to its golden perch (0x2FC-0x300). With
//   Krystal within its +0x18 (0x0410: 70 or 85) a waiting baby plays its call (sequence 1, in state 0) in place of
//   asking, so it comes out only once she is further away. Kyte's yes is any find node's, so every waiting baby loaded
//   comes out while she is on one, and goes back once she leaves.
// - A found baby asks again each frame (0x0924): no and it flies home and back to the state it had.
// - Kyte's flag is the curve node she flies to: as she reaches one (dll212_319C) her +0x1C8 takes its node type's flags
//   (dll212's D_48B8), and types 8, 9, 21 and 22 carry 0x200000. She flies along map curves of type 0x22 inside one
//   group, 0x46E (dll212_0CE8; a curve's +0x4, as dll25's export 12 reads it), which TriggerPlanes (action 0x1C),
//   FindKyteObject (dll720) and PerchObject (dll718) set. Each waiting baby has find nodes beside it in a group of
//   their own (236A 0x10, 154 away; 29FA 0x28 on its ledge, 70; 29B3 0x1B, a loop round its crate that she circles,
//   40), and nothing in the ROM sets 0x46E to one of them, so no baby waiting to be found ever comes out.
//   FindKyteObject 31858 by 236A sends her to 0xF, a node of type 5.
//
// What Restored changes: as Krystal comes within a waiting baby's +0x18, 0x46E is set to the group of the find node
// nearest the baby (within 400; dll26's table holds the map's curves), and Kyte's own path code flies her there, where
// her flag brings it out once Krystal steps back from its call. Once the baby is rescued 0x46E goes back to what it
// was, unless something has set it since.
// - 29FA waits on a balcony of the hall east of the courtyard, 183 above its floor (local (-1491, 2049, 44)), walled on
//   its east and open on its other three sides; nothing in the ROM reaches it (no ladder, lift or ledge joins it: the
//   hall's wind lifts, CFWindLift 964 and 95D, rise to 2080 and 2200). TriggerPoint 31BF3 there (radius 120, 100 high)
//   is what loads its group 21, so on the prototype it never even loads. On Restored the baby and the TriggerPoint are
//   placed on the hall's floor just west of the balcony (ledge_moves), where every other part works as the ROM has it:
//   Kyte's find nodes on the balcony (0x28) are 200 from it.
//
// The deck (always on), read from the ROM:
// - Kyte flies only along curves joined to the one her path started from. Her path starts (dll212_0CE8, while her
//   state's +0x4 is 0) from the curve nearest her of any group, whose group becomes 0x46E, or with 0x454 set from the
//   nearest of 0x46E's group, where she is put. Each node after (dll212_1404) is on a path through the links to 0x46E's
//   group (nearest the player), and with none she takes a neighbour and 0x46E becomes its group: what she was sent to
//   is dropped. A curve's group is its placement's +0x5.
// - Shut in on the courtyard's roof (0x41 without 0x43) CFLevelControl (dll400_0914) holds 0x46E at 0x11 and sets
//   0x454, so she is on the deck's six curves (deck_curves), which join none of the fortress's others, and nothing moves
//   her off once 0x43 frees her: she circles the deck whatever sends her elsewhere, PerchObject 429D8's star to the gold
//   perch (0xC0, whose curve sets 0x8D1 as she lands) included.
//
// What it changes: once 0x43 is set, a path start passes over the deck's curves for the nearest curve off it, and
// with 0x46E still at 0x11 and Kyte out her path starts again.

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
    constexpr int kyte_freed = 0x43;
    constexpr int kyte_to_start = 0x454;
    constexpr int deck_group = 0x11;
    // The deck's curves (map 12, group 0x11), by their unique ids (+0x14): they join none of the fortress's others.
    constexpr uint32_t deck_curves[] = { 0x31861, 0x31862, 0x31863, 0x31864, 0x318AC, 0x42D10 };
    constexpr int16_t kyte_object = 0x25;

    constexpr int32_t set_bit = 0x80014670;
    constexpr int wind_on = 0x30D;
    constexpr int wind_down = 0x8C8;
    constexpr float from_the_top = 150.0f;
    constexpr int roof_blown = 0x2E1;

    constexpr int fortress = 12;
    constexpr int32_t loading_map = 0x800B96AC;  // the header func_80045BF4 has just loaded
    // 29FA and its TriggerPoint, by their unique ids (+0x14), put on the hall's floor west of their balcony: +0x8, +0xC
    // and +0x10 a placement's position, which func_80045BF4 has made world units (local (-1570, 1866, 44)).
    struct Move {
        uint32_t uid;
        float x, y, z;
    };
    constexpr Move ledge_moves[] = {
        { 0x29FA, -11170.0f, 1866.0f, 18604.0f },
        { 0x31BF3, -11170.0f, 1866.0f, 18604.0f },
    };

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

    // The curve of type 0x22 nearest the position that want() takes, or 0 with none within reach. dll26's export 3
    // returns its table of the map's curves, 8 bytes each with the curve's placement at +4, and writes their count to
    // $a0's word (as dll25's export 12 reads it); a curve's group is its placement's +0x4.
    template <typename Want>
    gpr nearest_curve(uint8_t* rdram, recomp_context* ctx, gpr position, float reach, Want want) {
        Calls calls(ctx);
        gpr count_at = ctx->r29 + 0x10;
        ctx->r4 = count_at;
        call(rdram, ctx, word(rdram, word(rdram, word(rdram, dll26_handle)) + 4 + 4 * 3));
        gpr table = ctx->r2;
        int32_t count = (int32_t)MEM_W(0, count_at);
        gpr found = 0;
        float nearest = reach;
        for (int32_t i = 0; table != 0 && i < count; i++) {
            gpr curve = MEM_W(4 + 8 * i, table);
            if (curve == 0 || MEM_B(0x19, curve) != 0x22 || !want(curve)) {
                continue;
            }
            float d = distance(rdram, curve + 8, position);
            if (d < nearest) {
                nearest = d;
                found = curve;
            }
        }
        return found;
    }

    // The group of the find node (a curve of type 0x22 whose node type, +0x1A, is 8, 9, 21 or 22) nearest the baby,
    // or -1 with none within find_node_reach.
    int find_group(uint8_t* rdram, recomp_context* ctx, gpr baby) {
        gpr curve = nearest_curve(rdram, ctx, baby + 0x18, find_node_reach, [rdram](gpr curve) {
            int type = (int8_t)MEM_B(0x1A, curve);
            return type == 8 || type == 9 || type == 21 || type == 22;
        });
        return curve != 0 ? MEM_HU(0x4, curve) : -1;
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
    if (!restored() || player == 0 || (int32_t)MEM_W(0x4, state) != wind_on || (int32_t)MEM_W(0x8, state) != wind_down) {
        return;
    }
    if (dino::player_carries(rdram, (int32_t)player) && get(rdram, ctx, roof_blown) == 0) {
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
            int32_t now = get(rdram, ctx, kyte_group);
            if (now == sent.group) {
                set(rdram, ctx, kyte_group, sent.before);
            }
            if (logging) {
                fprintf(stderr, now == sent.group ? "cloudrunner: baby %08X rescued, 0x46E back to %X\n" :
                    "cloudrunner: baby %08X rescued, 0x46E left at %X, set since\n", (uint32_t)baby,
                    now == sent.group ? sent.before : now);
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

namespace {
    bool on_the_deck(uint8_t* rdram, gpr curve) {
        uint32_t uid = (uint32_t)MEM_W(0x14, curve);
        for (uint32_t deck : deck_curves) {
            if (uid == deck) {
                return true;
            }
        }
        return false;
    }
}

// us.toml's hook at the start of CFLevelControl's Kyte routine (dll400_0914), every frame map 12 is loaded. Always on:
// with Kyte freed (0x43), out, and her group (0x46E) still the deck's, her path code starts again (her state's +0x4 back
// to 0, so dll212_0CE8 runs next), from a curve off the deck (dino_kyte_start_curve).
extern "C" void dino_cloudrunner_kyte_freed(uint8_t* rdram, recomp_context* ctx) {
    if (get(rdram, ctx, kyte_freed) == 0 || get(rdram, ctx, kyte_group) != deck_group) {
        return;
    }
    gpr kyte;
    {
        Calls calls(ctx);
        call(rdram, ctx, sidekick_object);
        kyte = ctx->r2;
    }
    if (kyte == 0 || MEM_H(0x46, kyte) != kyte_object || MEM_B(0x4, MEM_W(0xB8, kyte)) != 1) {
        return;
    }
    MEM_B(0x4, MEM_W(0xB8, kyte)) = 0;
    if (logging) {
        fprintf(stderr, "cloudrunner: Kyte freed with 0x46E at the deck's 0x11, her path started again\n");
    }
}

// us.toml's hook in Kyte's path start (dll212_0CE8 0x0DE0, $v0 the curve nearest her, whose group becomes 0x46E, the
// curve her path starts from). Always on: once she is freed (0x43), a curve of the deck is passed over for the nearest
// curve off it.
extern "C" void dino_kyte_start_curve(uint8_t* rdram, recomp_context* ctx) {
    gpr curve = ctx->r2;
    if (curve == 0 || !on_the_deck(rdram, curve) || get(rdram, ctx, kyte_freed) == 0) {
        return;
    }
    gpr kyte;
    {
        Calls calls(ctx);
        call(rdram, ctx, sidekick_object);
        kyte = ctx->r2;
    }
    if (kyte == 0) {
        return;
    }
    gpr off = nearest_curve(rdram, ctx, kyte + 0x18, 1e9f, [rdram](gpr c) { return !on_the_deck(rdram, c); });
    if (off == 0) {
        return;
    }
    if (logging) {
        fprintf(stderr, "cloudrunner: Kyte starts off the deck, at curve %X (group %X) in place of %X\n",
            (uint32_t)MEM_W(0x14, off), MEM_HU(0x4, off), (uint32_t)MEM_W(0x14, curve));
    }
    ctx->r2 = off;
}

// us.toml's hook at the end of func_80045BF4 (beside dino_krazoa_map_loaded), which has loaded map $s0 and placed none
// of its objects yet. On Baby CloudRunners' Restored, map 12's 29FA and its TriggerPoint are put on the hall's floor
// (ledge_moves).
extern "C" void dino_cloudrunner_map_loaded(uint8_t* rdram, recomp_context* ctx) {
    if ((int32_t)ctx->r16 != fortress || !babies_restored()) {
        return;
    }
    gpr header = (gpr)word(rdram, loading_map);
    gpr list = (gpr)MEM_W(0x20, header);
    int bytes = MEM_HU(0x8, header);
    for (int at = 0; at < bytes;) {
        gpr p = list + at;
        int size = MEM_BU(2, p) * 4;
        if (size == 0) {
            break;
        }
        for (const Move& move : ledge_moves) {
            if ((uint32_t)MEM_W(0x14, p) == move.uid) {
                const float position[] = { move.x, move.y, move.z };
                for (int i = 0; i < 3; i++) {
                    uint32_t value;
                    memcpy(&value, &position[i], sizeof(value));
                    MEM_W(0x8 + 4 * i, p) = value;
                }
                if (logging) {
                    fprintf(stderr, "cloudrunner: placement %X put on the hall's floor\n", move.uid);
                }
            }
        }
        at += size;
    }
}
