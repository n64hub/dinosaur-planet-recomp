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
// back, as the ROM has it, and a player who steps in at the bottom rides up to the top and steps off on either side. A
// player who carries something (a barrel, for the courtyard roof: cloudrunner_barrels.cpp) is left to the wind.

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
