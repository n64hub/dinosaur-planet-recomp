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
//   within its +0x1A (0x07A0): 0x901 counts it, its bit is set, and it flies to its golden perch (0x2FC-0x300). Nothing
//   in the ROM sets that flag of Kyte's, so no baby waiting to be found ever comes out.
// - A found baby asks again each frame (0x0924): no and it flies home and back to the state it had.
//
// What Restored changes: both asks answer yes while Kyte is within 400 of the baby and it isn't a rescued one. 400 is
// Kyte flying beside Krystal on the floor below the baby on a ledge (29FA, 300 below and beside).

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
    constexpr float kyte_finds = 400.0f;

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

// us.toml's hook at CFCloudBaby's update (dll373 0x01A0, $a0 the baby), with DINO_LOG_CLOUDRUNNER: each baby's state as
// it changes, with how far Krystal and Kyte are in steps of 100.
extern "C" void dino_cloud_baby_update(uint8_t* rdram, recomp_context* ctx) {
    if (!logging) {
        return;
    }
    static gpr babies[8];
    static int32_t seens[8];
    gpr baby = ctx->r4;
    int32_t state = (int32_t)MEM_W(0xAC, MEM_W(0xB8, baby));
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
    int32_t seen = state * 10000 + (int32_t)(krystal_away / 100.0f) * 100 + (int32_t)(kyte_is / 100.0f);
    if (babies[i] == baby && seens[i] == seen) {
        return;
    }
    babies[i] = baby;
    seens[i] = seen;
    fprintf(stderr, "cloudrunner: baby %08X at %.0f %.0f %.0f state %d, Krystal %.0f away, Kyte %.0f\n", (uint32_t)baby,
        read_float(rdram, baby + 0x18), read_float(rdram, baby + 0x1C), read_float(rdram, baby + 0x20), state,
        krystal_away, kyte_is);
}

// us.toml's hook in CFCloudBaby's test for Kyte (dll373 0x2050), which a baby waiting to be found (state 0, 1 or 4)
// runs each frame: after the sidekick's export 24 answered ($v0), yes brings it out to face her (state 5). Export 24 is
// her state's +0x1C8 & 0x200000, which nothing in the ROM sets, so on Original no baby waiting to be found ever comes
// out. On Restored it answers yes while Kyte is within 400 of the baby ($sp+0x20) and the baby isn't a rescued one (hidden,
// +0x6's 0x4000, as it loads with its bit set; its states still run).
extern "C" void dino_cloud_baby_found(uint8_t* rdram, recomp_context* ctx) {
    if (ctx->r2 != 0 || !babies_restored()) {
        return;
    }
    gpr baby = MEM_W(0x20, ctx->r29);
    if ((MEM_H(0x6, baby) & 0x4000) != 0) {
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
    float d = kyte_away(rdram, kyte, baby);
    if (d >= kyte_finds) {
        return;
    }
    if (logging) {
        fprintf(stderr, "cloudrunner: baby %08X found, Kyte %.0f away\n", (uint32_t)baby, d);
    }
    ctx->r2 = 1;
}

// us.toml's hook in CFCloudBaby's update (dll373 0x0924, $s1 the baby): a found baby (state 5) asks export 24 again
// each frame ($v0), yes and it stays out facing Kyte (0x09C8), no and it flies home and back to the state it had
// (0x0930). Krystal rescues it by pressing A within its +0x1A (0x07A0). On Restored the answer is the first ask's.
extern "C" void dino_cloud_baby_follows(uint8_t* rdram, recomp_context* ctx) {
    if (ctx->r2 != 0 || !babies_restored()) {
        return;
    }
    gpr baby = ctx->r17;
    gpr kyte;
    {
        Calls calls(ctx);
        call(rdram, ctx, sidekick_object);
        kyte = ctx->r2;
    }
    if (kyte != 0 && kyte_away(rdram, kyte, baby) < kyte_finds) {
        ctx->r2 = 1;
    }
}
