// Diamond Bay's part of the third SpellStone's stretch: the Pointback mother's second egg, in the map's act 2 (from
// the first SpellStone, 0x4E9; acts.cpp has how a map's act is kept).
//
// DBPointMum (dll658) walks a table of 16-byte steps for each act (its data +0x2810 on: flags at +0, the step's bit at
// +4, its talk sequence at +6, its give sequence at +8, a callback at +C), on to the next step once a step's bit is set
// (dll658_0720, 0x780-0x7C8). A step without flag 2 goes on only as she is used: A within 80 units of her use point,
// at her head (dll2 0xE58-0xF84). Flag 0x40 then sets the step's bit, and flag 0x20 has her take the egg as it is used
// from the items menu in that frame (dll1.7 of 0x86D, 0x874-0x8F4). Act 2's steps are 0x3FF (flags 0xC0: talking to
// her sets it, and DBseqobject 41A50 plays her sequence 3, "The final SpellStone. Your quest is almost at an end.", on
// it and sets 0x400), the egg (0x39E, 0xA0, give sequence 2) and a last talk (sequence 2). Both play on the prototype's
// own code once Sabre stands at her head, the far side from the way in.
//
// The egg (DB_egg, dll662, uid 41A4E in group 9: its +0x1C 0x86D, set as A takes it within 25 units, with 0x3C4 while
// it is carried) sits on a rock over the river, reached by the bridge DBbridgeanim (dll678, 415CA) raises on 0x3D9:
// each light beam (DBlgtbeam, dll677, 415C8, 415C9, 415CD, 415CE) sets 0x82B to its number as Sabre stands on it, which
// shows the next for 350 ticks, and Sabre leaps from one to the next by himself at a walk (a run goes over the edge).
// Three things stop it on Original:
//
// - DB_FlameSwitch (dll689, 41818, group 8) sets 0x3D9 when Tricky's flame (a hit of type 0x1A) lights it (0x1C8-0x1E0),
//   but it stands 250 units (+0x1C) below where it works, and disabled, until 0x83E is set (0x8C-0x104, read once at
//   its load, and its update re-reads only the disabled flag, 0x17C), and nothing sets 0x83E. Its setup's +0x20 is
//   0x3FF, which its code never reads. On Restored the switch takes that bit for 0x83E: once 0x3FF is set it sets 0x83E
//   and rises, as its load would have raised it, and its update enables it.
// - Tricky breathes his flame from the curve 418C9 beside it, flameblasts (0x4F0) level at his own height, about 140
//   below the risen switch's hit sphere. On Restored a flameblast in the column it rose through is a flame's hit.
// - DBlgtbeam's test of Sabre standing on it (dll677_1164) multiplies his distance on x by the cosine of its yaw and on
//   z by the sine, with no absolute value, against 22.5: for a beam whose cosine or sine is negative, one of the two
//   always passes, so beams 2 and 3 (yaw 0x7100) take Sabre anywhere within about 60 on z and 15 on y as on them.
//   A leap from beam 3 to 4 falls through that and sets 0x82B to 2, which steps the bridge back and takes beam 4 from
//   under him. On Restored both are taken without their sign.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "recomp.h"
#include "dino.h"

namespace {
    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t get_bit = 0x800147EC;

    constexpr int switch_raised = 0x83E;

    // The game's objects and Tricky's flameblasts among them, a hit of their type, and how near on x and z a flameblast
    // lights the switch.
    constexpr int32_t objects = 0x800B1920;
    constexpr int32_t object_count = 0x800B1924;
    constexpr int flame_object = 0x4F0;
    constexpr int32_t flame_hit = 0x1A;
    constexpr float flame_reach = 40.0f;

    // DINO_LOG_DIAMOND_BAY set (development): a flame lighting the switch is printed.
    const bool logging = std::getenv("DINO_LOG_DIAMOND_BAY") != nullptr;

    float real(uint8_t* rdram, gpr address) {
        float f;
        int32_t w = MEM_W(0, address);
        memcpy(&f, &w, 4);
        return f;
    }

    bool bit(uint8_t* rdram, recomp_context* ctx, int b) {
        ctx->r4 = b;
        LOOKUP_FUNC(get_bit)(rdram, ctx);
        return (int32_t)ctx->r2 != 0;
    }
}

// us.toml's hook at the start of DB_FlameSwitch's update (dll689_0130), the object in $a0.
extern "C" void dino_diamond_bay_flame_switch(uint8_t* rdram, recomp_context* ctx) {
    if (!dino::third_stone_restored()) {
        return;
    }
    gpr object = ctx->r4;
    gpr setup = (gpr)(int32_t)MEM_W(0x4C, object);
    if (setup == 0) {
        return;
    }
    recomp_context saved = *ctx;
    ctx->r29 -= 0x40;
    if (!bit(rdram, ctx, switch_raised) && bit(rdram, ctx, MEM_H(0x20, setup))) {
        ctx->r4 = switch_raised;
        ctx->r5 = 1;
        LOOKUP_FUNC(set_bit)(rdram, ctx);
        float y;
        memcpy(&y, &MEM_W(0x10, object), 4);
        y += (float)(int16_t)MEM_H(0x1C, setup);
        memcpy(&MEM_W(0x10, object), &y, 4);
    }
    *ctx = saved;
}

// us.toml's hook at DB_FlameSwitch's test of what hit it (dll689 0x1C8), the hit's type in $v0 and the switch at
// sp+0x38. On Restored a flameblast in the column the switch rose through (on x and z within flame_reach of it, below
// its origin by no more than its rise) is a flame's hit, 0x1A.
extern "C" void dino_diamond_bay_flame_switch_hit(uint8_t* rdram, recomp_context* ctx) {
    if ((int32_t)ctx->r2 == flame_hit || !dino::third_stone_restored()) {
        return;
    }
    gpr object = (gpr)(int32_t)MEM_W(0x38, ctx->r29);
    gpr setup = (gpr)(int32_t)MEM_W(0x4C, object);
    if (setup == 0) {
        return;
    }
    float sx = real(rdram, object + 0x18), sy = real(rdram, object + 0x1C), sz = real(rdram, object + 0x20);
    float rise = (float)(int16_t)MEM_H(0x1C, setup);
    gpr all = (gpr)MEM_W(0, (gpr)objects);
    for (int i = 0; all != 0 && i < (int32_t)MEM_W(0, (gpr)object_count); i++) {
        gpr o = (gpr)(int32_t)MEM_W(i * 4, all);
        if (o == 0 || (int16_t)MEM_H(0x46, o) != flame_object) {
            continue;
        }
        float dx = real(rdram, o + 0x18) - sx, dy = real(rdram, o + 0x1C) - sy, dz = real(rdram, o + 0x20) - sz;
        if (dx * dx + dz * dz < flame_reach * flame_reach && dy <= 0.0f && dy >= -rise) {
            if (logging) {
                fprintf(stderr, "diamond bay: a flame lights the switch\n");
            }
            ctx->r2 = flame_hit;
            return;
        }
    }
}

// us.toml's hooks at DBlgtbeam's two multiplies (dll677_1164 0x1EC and 0x27C), the cosine or sine of its yaw in $f0. On
// Restored it is taken without its sign, so the test is a box round the beam.
extern "C" void dino_diamond_bay_beam_reach(uint8_t* rdram, recomp_context* ctx) {
    if (dino::third_stone_restored()) {
        ctx->f0.fl = std::fabs(ctx->f0.fl);
    }
}
