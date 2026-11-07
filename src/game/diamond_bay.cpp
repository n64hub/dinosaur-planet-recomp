// Diamond Bay's part of the third SpellStone's stretch: the Pointback mother's second egg, in the map's act 2 (from
// the first SpellStone, 0x4E9; acts.cpp has how a map's act is kept).
//
// DBPointMum (dll658) walks a table of 16-byte steps for each act (its data +0x2810 on: flags at +0, the step's bit at
// +4, its talk sequence at +6, its give sequence at +8, a callback at +C), on to the next step once a step's bit is set
// (dll658_0720, 0x780-0x7C8). Flag 0x20 has her take the egg from the items menu (dll1.7 of 0x86D, 0x874-0x8F4) and
// flag 0x80 marks a step she waits on, set elsewhere: act 1's first, 0x39B, comes from the TriggerPlane 41A35, which
// sets it as Sabre walks by about 150 units from her. Act 2's steps are 0x3FF (flags 0xC0), the egg (0x39E, 0xA0, give
// sequence 2) and a last talk (sequence 2), and nothing sets 0x3FF: no code, and no trigger in Diamond Bay, so
// DBseqobject 41A50 (dll309), which plays her act-2 sequence 3 ("The final SpellStone. Your quest is almost at an
// end.", which shows the egg on its rock arch) on 0x3FF and sets 0x400, never plays, and she never takes the egg. So
// with Third SpellStone on Restored, 0x3FF is set as Sabre comes within 150 units of her while she waits on it, as
// 0x39B is in act 1.
//
// The egg (DB_egg, dll662, uid 41A4E in group 9: its +0x1C 0x86D, set as A takes it within 25 units, with 0x3C4 while
// it is carried) sits on a rock arch over the river, reached by the bridge DBbridgeanim (dll678, 415CA) raises on
// 0x3D9. DB_FlameSwitch (dll689, 41818, group 8) sets 0x3D9 when Tricky's flame (a hit of type 0x1A) lights it
// (0x1C8-0x1E0), but it stands 250 units (+0x1C) below where it works, and disabled, until 0x83E is set (0x8C-0x104,
// read once at its load, and its update re-reads only the disabled flag, 0x17C), and nothing sets 0x83E. Its setup's
// +0x20 is 0x3FF, which its code never reads. So here, on Restored, the switch takes that bit for 0x83E: once 0x3FF is
// set it sets 0x83E and rises, as its load would have raised it, and its update enables it.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "recomp.h"
#include "dino.h"

namespace {
    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t get_bit = 0x800147EC;

    constexpr int32_t player_object = 0x80023914;

    // DBPointMum's act-2 first step, set within talk_reach of her (on x and z).
    constexpr uint16_t talk_step_bit = 0x3FF;
    constexpr float talk_reach = 150.0f;

    constexpr int switch_raised = 0x83E;

    // DINO_LOG_DIAMOND_BAY set (development): each change of DBPointMum's step, its flags or her being used is printed.
    const bool logging = std::getenv("DINO_LOG_DIAMOND_BAY") != nullptr;

    bool bit(uint8_t* rdram, recomp_context* ctx, int b) {
        ctx->r4 = b;
        LOOKUP_FUNC(get_bit)(rdram, ctx);
        return (int32_t)ctx->r2 != 0;
    }
}

// us.toml's hook in dll658_0720 (0x800), where DBPointMum reads the flags of her current step, the step in $s0.
extern "C" void dino_diamond_bay_mother_step(uint8_t* rdram, recomp_context* ctx) {
    gpr step = ctx->r16;
    if (logging) {
        static uint32_t last = 0xFFFFFFFF;
        gpr mother = (gpr)(int32_t)MEM_W(0x38, ctx->r29);
        gpr state = (gpr)(int32_t)MEM_W(0xB8, mother);
        uint32_t now = ((uint16_t)MEM_H(4, step) << 16) | ((MEM_W(0, step) & 0xFF) << 8) |
            ((MEM_BU(0xAF, mother) & 1) << 4) | (MEM_BU(0x499, state) ? 1 : 0);
        if (now != last) {
            fprintf(stderr, "diamond bay: mother step %X flags %X used %d, +0x499 %d, +0x524 %d\n", now >> 16,
                (now >> 8) & 0xFF, (now >> 4) & 1, now & 1, MEM_BU(0x524, state));
            last = now;
        }
    }
    if ((uint16_t)MEM_H(4, step) != talk_step_bit || !dino::third_stone_restored()) {
        return;
    }
    gpr mother = (gpr)(int32_t)MEM_W(0x38, ctx->r29);
    recomp_context saved = *ctx;
    ctx->r29 -= 0x40;
    LOOKUP_FUNC(player_object)(rdram, ctx);
    gpr player = ctx->r2;
    if (player != 0 && !bit(rdram, ctx, talk_step_bit)) {
        float mx, mz, px, pz;
        memcpy(&mx, &MEM_W(0x18, mother), 4);
        memcpy(&mz, &MEM_W(0x20, mother), 4);
        memcpy(&px, &MEM_W(0x18, player), 4);
        memcpy(&pz, &MEM_W(0x20, player), 4);
        if ((px - mx) * (px - mx) + (pz - mz) * (pz - mz) < talk_reach * talk_reach) {
            ctx->r4 = talk_step_bit;
            ctx->r5 = 1;
            LOOKUP_FUNC(set_bit)(rdram, ctx);
        }
    }
    *ctx = saved;
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
