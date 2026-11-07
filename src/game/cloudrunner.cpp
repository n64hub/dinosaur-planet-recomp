// CloudRunner Fortress (map 12): the way down the treasure shaft. Past CFBlastedWall the corridor opens onto a shaft
// whose bottom leads north into the treasure caves (map 15) and east to the drop into the race (map 43), and the
// shaft's wind lift, CFTreasWindLift, only ever starts the way up for a player who comes from the top: she hovers at
// the corridor's height and no button lets her down (docs/story-points.md, CloudRunner Fortress). With Treasure Lift on
// Restored (the default) a player who steps into it from the top is lowered to the bottom; on Original it plays as the
// console does.
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
// back, as the ROM has it, and a player who steps in at the bottom rides up to the top and steps off on either side.

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
    constexpr float kyte_finds = 150.0f;

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

// us.toml's hooks in CFCloudBaby's test for Kyte (dll373 0x2050), which brings a hidden baby out to be rescued when the
// sidekick's export 24 answers yes. The first, as the sidekick is looked up ($v0, 0 with none out), only logs; the
// second, after export 24 answered ($v0), answers yes on Restored when Kyte is within reach of the baby ($sp+0x20).
extern "C" void dino_cloud_baby_sidekick(uint8_t* rdram, recomp_context* ctx) {
    if (logging && ctx->r2 == 0) {
        gpr baby = MEM_W(0x20, ctx->r29);
        fprintf(stderr, "cloudrunner: baby %08X state %d: no sidekick out\n", (uint32_t)baby,
            (int32_t)MEM_W(0xAC, MEM_W(0xB8, baby)));
    }
}

// us.toml's hook at CFCloudBaby's update (dll373 0x01A0, $a0 the baby), with DINO_LOG_CLOUDRUNNER: each baby's state as it
// changes, with how far Krystal and Kyte are.
extern "C" void dino_cloud_baby_update(uint8_t* rdram, recomp_context* ctx) {
    if (!logging) {
        return;
    }
    static gpr babies[8];
    static int32_t states[8];
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
    auto away = [&](gpr o) {
        if (o == 0) {
            return -1.0f;
        }
        float dx = read_float(rdram, o + 0x18) - read_float(rdram, baby + 0x18);
        float dy = read_float(rdram, o + 0x1C) - read_float(rdram, baby + 0x1C);
        float dz = read_float(rdram, o + 0x20) - read_float(rdram, baby + 0x20);
        return sqrtf(dx * dx + dy * dy + dz * dz);
    };
    // The state, and Kyte's distance in steps of 100 while Krystal is within 600.
    int32_t seen = state * 100 + (away(player) < 600.0f ? (int32_t)(away(kyte) / 100.0f) : 99);
    if (babies[i] == baby && states[i] == seen) {
        return;
    }
    babies[i] = baby;
    states[i] = seen;
    fprintf(stderr, "cloudrunner: baby %08X at %.0f %.0f %.0f state %d, Krystal %.0f away, Kyte %.0f\n", (uint32_t)baby,
        read_float(rdram, baby + 0x18), read_float(rdram, baby + 0x1C), read_float(rdram, baby + 0x20), state,
        away(player), away(kyte));
}

extern "C" void dino_cloud_baby_found(uint8_t* rdram, recomp_context* ctx) {
    gpr baby = MEM_W(0x20, ctx->r29);
    gpr answer = ctx->r2;
    gpr kyte;
    {
        Calls calls(ctx);
        call(rdram, ctx, sidekick_object);
        kyte = ctx->r2;
    }
    float d = -1.0f;
    if (kyte != 0) {
        float dx = read_float(rdram, kyte + 0x18) - read_float(rdram, baby + 0x18);
        float dy = read_float(rdram, kyte + 0x1C) - read_float(rdram, baby + 0x1C);
        float dz = read_float(rdram, kyte + 0x20) - read_float(rdram, baby + 0x20);
        d = sqrtf(dx * dx + dy * dy + dz * dz);
    }
    // A baby rescued (its bit set as it loads) is hidden, +0x6's 0x4000, and its states still run: never find it again.
    bool hidden = (MEM_H(0x6, baby) & 0x4000) != 0;
    bool found = answer == 0 && babies_restored() && kyte != 0 && !hidden && d < kyte_finds;
    if (logging && found) {
        fprintf(stderr, "cloudrunner: baby %08X at %.0f %.0f %.0f state %d, Kyte %.0f away, export 24 %d%s\n",
            (uint32_t)baby, read_float(rdram, baby + 0x18), read_float(rdram, baby + 0x1C),
            read_float(rdram, baby + 0x20), (int32_t)MEM_W(0xAC, MEM_W(0xB8, baby)), d, (int)answer,
            found ? ", found" : "");
    }
    if (found) {
        ctx->r2 = 1;
    }
}

// us.toml's hook at CFPrisonGuard's update (dll375 0x00D0, $a0 the guard), with DINO_LOG_CLOUDRUNNER: the player's state
// (dll210's export 50, her state's +0x87C) as it changes, with how far she is from the guard, who catches her within
// 150 unless she holds the prison key (0x44) or that state is 0x40.
extern "C" void dino_prison_guard_update(uint8_t* rdram, recomp_context* ctx) {
    if (!logging) {
        return;
    }
    gpr guard = ctx->r4, player;
    static bool once = false;
    if (!once) {
        once = true;
        fprintf(stderr, "cloudrunner: the guard is here\n");
    }
    {
        Calls calls(ctx);
        call(rdram, ctx, player_object);
        player = ctx->r2;
    }
    if (player == 0) {
        return;
    }
    static int32_t last = -0x10000;
    int32_t state = (int16_t)MEM_H(0x87C, MEM_W(0xB8, player));
    if (state == last) {
        return;
    }
    last = state;
    float dx = read_float(rdram, player + 0x18) - read_float(rdram, guard + 0x18);
    float dz = read_float(rdram, player + 0x20) - read_float(rdram, guard + 0x20);
    fprintf(stderr, "cloudrunner: player state 0x%X, %.0f from the guard\n", state, sqrtf(dx * dx + dz * dz));
}
