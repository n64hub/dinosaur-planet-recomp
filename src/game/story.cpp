// --story and --bits (development, src/main/main.cpp reads them): start a run from a chosen point of the story, for what
// a warp can't set. docs/story-points.md lists the points and what each bit is, with its evidence from the ROM.
//
// The story is the game's bits: func_80014670 sets one and func_800147EC reads one, a number that the table in ROM file
// 55 turns into a place in one of the save's four banks. Which spirits Krystal has brought back and which SpellStones
// are placed are bits, and so are what a level has opened and each pickup taken. dll29 holds the save: its export 1
// clears the save to a new game's (slot -1 is no slot, so nothing touches the Flash), 29 sets the character, 6 stores
// where play starts and 7 starts the world there. That is what func_800142F0 does for the game's own new game;
// story_start does the same with the bits set before export 6, since export 7 keeps a copy that a retry restores.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "recomp.h"

#include "dino.h"

extern "C" uint32_t dino_rom_word(uint32_t offset);

namespace {
    constexpr int32_t dll29_handle = 0x8008C9F4; // the pointer the game keeps to its loaded dll29
    constexpr int32_t save_pointer = 0x800AE2A0; // the save in dll29's bss, which func_80014670 writes to
    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t map_at = 0x80044810; // the map at a position: x in f12, z in f14
    constexpr int32_t set_paused = 0x8001440C;
    constexpr uint32_t warp_table_rom = 0x020E023A; // ROM file 30

    // A point of the story: the bits that are set at it, the character played and the entry of the warp table that it
    // starts at, -1 for where a new game does.
    struct StoryBit {
        int bit;
        int value;
    };

    struct StoryPoint {
        const char* name;
        int warp;
        std::vector<StoryBit> bits;
    };

    constexpr int krystal = 1;

    const std::vector<StoryPoint> points = {
        { "new-game", -1, {} },
    };

    const StoryPoint* chosen = nullptr;
    std::vector<StoryBit> extra_bits;
    int chosen_warp = -1;

    int32_t word(uint8_t* rdram, int32_t address) {
        return MEM_W(0, (gpr)address);
    }

    void call(uint8_t* rdram, recomp_context* ctx, int32_t function) {
        LOOKUP_FUNC(function)(rdram, ctx);
    }

    // A dll's export n is the word n + 1 of the table its handle points at.
    void call_dll29(uint8_t* rdram, recomp_context* ctx, int export_index) {
        int32_t table = word(rdram, word(rdram, dll29_handle));
        call(rdram, ctx, word(rdram, table + 4 + 4 * export_index));
    }

    void set_game_bit(uint8_t* rdram, recomp_context* ctx, const StoryBit& b) {
        ctx->r4 = b.bit;
        ctx->r5 = b.value;
        call(rdram, ctx, set_bit);
    }
}

bool dino::story_choose(const char* name) {
    for (const StoryPoint& p : points) {
        if (strcmp(p.name, name) == 0) {
            chosen = &p;
            chosen_warp = p.warp;
            return true;
        }
    }
    fprintf(stderr, "--story: no point named %s, these are:", name);
    for (const StoryPoint& p : points) {
        fprintf(stderr, " %s", p.name);
    }
    fprintf(stderr, "\n");
    return false;
}

void dino::story_add_bits(const char* list) {
    for (const char* p = list; p != nullptr; p = strchr(p, ',') ? strchr(p, ',') + 1 : nullptr) {
        StoryBit b{ 0, 1 };
        if (sscanf(p, "%x=%d", &b.bit, &b.value) >= 1) {
            extra_bits.push_back(b);
        }
    }
}

void dino::story_set_warp(int index) {
    chosen_warp = index;
}

void dino::story_start(uint8_t* rdram, recomp_context* ctx) {
    static const StoryPoint no_point{ "new-game", -1, {} };
    const StoryPoint& point = chosen != nullptr ? *chosen : no_point;
    ctx->r4 = 0;
    call(rdram, ctx, set_paused);
    ctx->r4 = (uint32_t)-1;
    ctx->r5 = 0;
    call_dll29(rdram, ctx, 1);
    ctx->r4 = krystal;
    call_dll29(rdram, ctx, 29);
    for (const StoryBit& b : point.bits) {
        set_game_bit(rdram, ctx, b);
    }
    for (const StoryBit& b : extra_bits) {
        set_game_bit(rdram, ctx, b);
    }
    if (chosen_warp >= 0) {
        // Export 6 takes where (a vector in the game's memory, here under its stack), the facing, flags and the layer.
        gpr sp = ctx->r29;
        ctx->r29 -= 0x40;
        gpr position = ctx->r29 + 0x20;
        uint32_t entry = warp_table_rom + 16 * chosen_warp;
        int32_t layer = (int32_t)dino_rom_word(entry + 12);
        for (int i = 0; i < 3; i++) {
            MEM_W(i * 4, position) = dino_rom_word(entry + i * 4);
        }
        ctx->r4 = position;
        ctx->r5 = 0;
        ctx->r6 = 0;
        ctx->r7 = layer;
        call_dll29(rdram, ctx, 6);
        ctx->r29 = sp;
    }
    call_dll29(rdram, ctx, 7);
    fprintf(stderr, "--story: started %s, %d bits, at warp %d\n", point.name, (int)(point.bits.size() + extra_bits.size()), chosen_warp);
}

// Once a second, what an unattended run needs to tell where the player is and whether the player lives.
void dino::story_report(uint8_t* rdram, recomp_context* ctx, double seconds) {
    int32_t save = word(rdram, save_pointer);
    if (save == 0) {
        return;
    }
    call(rdram, ctx, player_object);
    int32_t object = (int32_t)ctx->r2;
    uint8_t character = MEM_BU(0x2F7, (gpr)save);
    gpr stats = (gpr)save + 12 * character;
    if (object == 0) {
        fprintf(stderr, "story: %.1f s no player, health %d/%d\n", seconds, MEM_BU(1, stats), MEM_BU(2, stats));
        return;
    }
    uint32_t x = MEM_W(0x18, (gpr)object), y = MEM_W(0x1C, (gpr)object), z = MEM_W(0x20, (gpr)object);
    float fx, fy, fz;
    memcpy(&fx, &x, 4);
    memcpy(&fy, &y, 4);
    memcpy(&fz, &z, 4);
    ctx->f12.fl = fx;
    ctx->f14.fl = fz;
    call(rdram, ctx, map_at);
    fprintf(stderr, "story: %.1f s map %d at %.0f %.0f %.0f health %d/%d\n", seconds, (int32_t)ctx->r2, fx, fy, fz, MEM_BU(1, stats), MEM_BU(2, stats));
}
