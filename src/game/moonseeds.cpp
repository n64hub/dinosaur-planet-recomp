// Moon Mountain Pass's part of the third SpellStone's stretch: the moonseeds Krystal plants, which nothing in the
// prototype gives, and the meteor in front of the test of fear's entrance, which nothing moves.
//
// The task list has Sabre collect them for Krystal once the third SpellStone is placed: task 91, "COLLECTED MOONSEEDS
// FOR KRYSTAL", with the SwapStone's clue "SEARCH IN DARK, WET PLACES", between 90 (the stone placed) and 92 (the swap),
// and Krystal plant one to reach the test of fear (ROM file 19). A ThornTail in SwapStone Hollow says "Moonseeds grow
// in dark damp places.", and the item is "MOON SEEDS". Their count is bit 0x86A, 3 bits in bank 2, which both
// inventories show (dll1's data, rows at +0x2B4 and +0x440: 086A ffff 040D, the moonseed icon). Each of Moon Mountain
// Pass's four MMP_mseedrecept (dll722, map 18 groups 2, 3, 4 and 6) takes one as the item is used beside it, lowers the
// count (0x350) and sets its planted bit (0x866-0x869). Nothing raises it: no code but dll722 names 0x86A, no placement
// in any map holds it, the Swap Store's items (dll768) don't list it, MMP_moonseed (object 0x54B, def 528) is dll214, a
// sequence's actor placed in no map, and WM_MoonSeedMoun is dll331's scenery in Warlock Mountain.
//
// So with Third SpellStone on Restored, once the stone is placed (0x878, VFP_SpellPlace dll637 0x388) four moonseeds
// lie in the dark, wet cave under the Hollow (map 8, SwapStone Hollow 2), one for each receptacle. Each is the game's
// own collectable, dll272, as the boss door's teeth and the keys are: as it is picked up it sets the bit in its
// setup's +0x1C and adds one to the bit in its +0x2C (0x1388, 0x13C0), here 0x86A, after the player's pickup
// (message 0x7000A, 0x96C), as SHbluemushroom (dll502) counts 0xC1. Its def is Cape Claw's gold nugget
// (CCgoldnuggetPic, object 0x5AF), a plain one of dll272's (its item record's type 1 with no sequence, and an id dll272
// tests for nowhere), drawn with MMP_moonseed's model (0xF7) at its scale, 1: func_80021450 loads an object's models
// from its def's list (0x80021774-0x800217C8), and the hook there gives a seed placed here the moonseed's. As it is
// picked up the player opens a scroll with the text its def names at +0xA2 (dll210_4910 0x900EDD78), the nugget's
// 0x103, "SHINEY NUGGET| Cape Claw": a seed's is 0x62, "MOON SEEDS| Grow Moon Seeds", the item's own text in ROM
// file 19 that no def names. Their picked-up bits are 0x6AB-0x6AE, saved bits in ROM file 55 (bank 1) that nothing in
// the code or the maps uses. On Third SpellStone's Original (acts.cpp) none lie there, as the prototype.
//
// The meteor (task 94, "MOVED METEOR TO REACH SHRINE ENTRANCE"): three MMP_moonrock (dll445, uids 425FA-425FC, in no
// group and every act) are carried and dropped into MMP_gyservent's vents (dll446, group 6); a rock dropped within 20 of
// a vent plugs it, and dll445_0628 counts the rocks in vents 3, 4 and 6 into 0x88C and sets 0x89B at three (0x9D4-0x9EC).
// The meteor's MMP_seqobject (dll309, uid 412AF) plays its sequence 5 on its trigger, 0x803, and sets its done bit
// 0x804, but nothing sets 0x803: no code, no placement and no event of ROM file 13 names it, and only a HitAnimator
// (uid 4267A, map animation 7) reads 0x89B. So with Third SpellStone on Restored, the three rocks in place set 0x803
// too, and the meteor's sequence plays.

#include <atomic>
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
    constexpr int32_t new_placement = 0x800233F4; // (size, object id): a zeroed placement record
    constexpr int32_t spawn_object = 0x800213EC;  // (placement, 5, map, -1, [sp+0x10] parent): the object
    constexpr int32_t map_structs = 0x800B5328;   // each loaded map's header, by map number (0 while not loaded)

    constexpr int third_stone_placed = 0x878;
    constexpr int moonseeds = 0x86A;
    constexpr int rocks_in_vents = 0x89B;
    constexpr int meteor_trigger = 0x803;

    constexpr int cave = 8;
    constexpr int seed_object = 0x5AF;
    constexpr int seed_model = 0xF7;
    constexpr int seed_text = 0x62;

    struct Seed {
        float x, y, z;
        uint8_t yaw;
        int bit;
    };
    // World positions, map 8's cell at x -8320, z -2560.
    constexpr Seed seeds[] = {
        { -8220.0f, -1085.0f, -2225.0f, 0x00, 0x6AB },
        { -7300.0f, -1090.0f, -2050.0f, 0x40, 0x6AC },
        { -7990.0f, -1075.0f, -1600.0f, 0x80, 0x6AD },
        { -7860.0f, -1075.0f, -1440.0f, 0xC0, 0x6AE },
    };

    bool placed = false;
    // Set while spawn_object makes a seed, for the model hook.
    std::atomic<bool> spawning{ false };
    const bool logging = getenv("DINO_LOG_MOONSEEDS") != nullptr;

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

    int get(uint8_t* rdram, recomp_context* ctx, int bit) {
        ctx->r4 = bit;
        LOOKUP_FUNC(get_bit)(rdram, ctx);
        return (int32_t)ctx->r2;
    }

    void place(uint8_t* rdram, recomp_context* ctx, const Seed& seed) {
        // As the boss door's teeth (map 13, 413b1): the collectable's setup from +0x18, with the seed's picked-up bit
        // in +0x1C and the moonseeds' count in +0x2C.
        const uint8_t setup[] = { 0x0A, 0x00, 0x05, seed.yaw, (uint8_t)(seed.bit >> 8), (uint8_t)seed.bit, 0xFF, 0xFF,
            0x00, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, moonseeds >> 8, moonseeds & 0xFF };
        constexpr int size = 0x30;
        ctx->r4 = size;
        ctx->r5 = seed_object;
        LOOKUP_FUNC(new_placement)(rdram, ctx);
        gpr p = ctx->r2;
        MEM_B(2, p) = size / 4;
        // Flag 1, as krazoa.cpp's extras: kept however far the camera goes, and freed with its map.
        MEM_B(4, p) = 1;
        memcpy(&MEM_W(0x8, p), &seed.x, 4);
        memcpy(&MEM_W(0xC, p), &seed.y, 4);
        memcpy(&MEM_W(0x10, p), &seed.z, 4);
        for (size_t i = 0; i < sizeof(setup); i++) {
            MEM_B(0x18 + i, p) = setup[i];
        }
        ctx->r4 = p;
        ctx->r5 = 5;
        ctx->r6 = cave;
        ctx->r7 = -1;
        MEM_W(0x10, ctx->r29) = 0;
        spawning = true;
        LOOKUP_FUNC(spawn_object)(rdram, ctx);
        spawning = false;
        // dll272's init gives it its def's scale (0x00BC), the nugget's; the moonseed's is MMP_moonseed's.
        if (ctx->r2 != 0) {
            constexpr float seed_scale = 1.0f;
            memcpy(&MEM_W(0x8, ctx->r2), &seed_scale, 4);
        }
        if (logging) {
            fprintf(stderr, "moonseeds: seed 0x%X placed at (%.0f, %.0f, %.0f): 0x%08X\n", seed.bit, seed.x, seed.y,
                seed.z, (uint32_t)ctx->r2);
        }
    }
}

// us.toml's hook after func_80046BC8 places the loaded maps' objects, each frame: the seeds not yet picked up, once the
// cave is loaded with the third SpellStone placed.
extern "C" void dino_moonseeds_frame(uint8_t* rdram, recomp_context* ctx) {
    if (MEM_W(0, (gpr)(map_structs + 4 * cave)) == 0) {
        placed = false;
        return;
    }
    if (placed || !dino::third_stone_restored()) {
        return;
    }
    Calls calls(ctx);
    LOOKUP_FUNC(player_object)(rdram, ctx);
    if (ctx->r2 == 0 || !get(rdram, ctx, third_stone_placed)) {
        return;
    }
    placed = true;
    for (const Seed& seed : seeds) {
        if (!get(rdram, ctx, seed.bit)) {
            place(rdram, ctx, seed);
        }
    }
}

// us.toml's hook in func_80021450 before it loads a model of the object it makes (0x80021780), the model's number from
// its def's list in $a0, the object's id at $sp+0x130: a seed gets the moonseed's.
extern "C" void dino_moonseeds_model(uint8_t* rdram, recomp_context* ctx) {
    if (spawning && MEM_W(0x130, ctx->r29) == seed_object) {
        ctx->r4 = seed_model;
    }
}

// us.toml's hook in dll210_4910 once it has read the scroll's text (0x900EDD78) from the def of the object the player
// holds, $v0: a seed, a nugget counting the moonseeds, gets the moonseeds' text.
extern "C" void dino_moonseeds_scroll(uint8_t* rdram, recomp_context* ctx) {
    gpr held = ctx->r2;
    if (MEM_H(0x46, held) == seed_object && MEM_H(0x2C, (gpr)MEM_W(0x4C, held)) == moonseeds) {
        ctx->r4 = seed_text;
    }
}
// us.toml's hook in dll445_0628 (0xA14), where it has set or cleared 0x89B by the rocks in vents 3, 4 and 6.
extern "C" void dino_moonseeds_meteor(uint8_t* rdram, recomp_context* ctx) {
    if (!dino::third_stone_restored()) {
        return;
    }
    recomp_context saved = *ctx;
    ctx->r29 -= 0x40;
    ctx->r4 = rocks_in_vents;
    LOOKUP_FUNC(get_bit)(rdram, ctx);
    bool in_vents = (int32_t)ctx->r2 != 0;
    ctx->r4 = meteor_trigger;
    LOOKUP_FUNC(get_bit)(rdram, ctx);
    if (in_vents && (int32_t)ctx->r2 == 0) {
        ctx->r4 = meteor_trigger;
        ctx->r5 = 1;
        LOOKUP_FUNC(set_bit)(rdram, ctx);
    }
    *ctx = saved;
}
