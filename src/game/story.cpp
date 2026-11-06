// --story and --bits (development, src/main/main.cpp reads them): start a run from a chosen point of the story, for what
// a warp can't set. docs/story-points.md lists the points and what each bit is, with its evidence from the ROM.
//
// The story is the game's bits: func_80014670 sets one and func_800147EC reads one, a number that the table in ROM file
// 55 turns into a place in one of the save's four banks. Which spirits Krystal has brought back and which SpellStones
// are placed are bits, and so are what a level has opened and each pickup taken. dll29 holds the save: its export 1
// clears the save to a new game's (slot -1 is no slot, so nothing touches the Flash), 29 sets the character, 6 stores
// where play starts and 7 starts the world there. The game's own New Game is dll64's name screen calling export 1, then
// dll63 calling the two play flags and export 7; story_start does the same with the bits set and the place stored
// before export 7, since export 7 keeps a copy that a retry restores. (func_800142F0 is the rolling demo, not a new game.)

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
    // What dll63, the front end, calls with 1 before it starts play: each changes a flag in the word at 0x80092A98,
    // and without them the objects aren't drawn and the opening's textures are wrong.
    constexpr int32_t play_flag_2000 = 0x80041D20;
    constexpr int32_t play_flag_10 = 0x80041C6C;
    constexpr uint32_t warp_table_rom = 0x020E023A; // ROM file 30

    struct StoryBit {
        int bit;
        int value;
    };

    struct Place {
        float x, y, z;
        int layer;
    };

    // A point of the story: the character played, where it starts (an entry of the warp table, or with -1 the place
    // beside it) and the bits that are set at it.
    struct StoryPoint {
        const char* name;
        int character;
        int warp;
        Place place;
        std::vector<StoryBit> bits;
    };

    constexpr int sabre = 0;
    constexpr int krystal = 1;

    // A warp of no_place keeps where dll29's new game puts the player, the start of the game's opening.
    constexpr int no_place = -2;

    // Cape Claw's sand worm has no entry in the warp table. Map 29's record in ROM file 21 is at grid cell (2, 26) of 640
    // units, so the worm's object at (2155, -276, -206) is at (3435, -276, 16434), and this is the setuppoint object the
    // map has beside it at (2340, -265, -435).
    constexpr Place sand_worm_place = { 3620.0f, -265.0f, 16205.0f, 0 };

    using Bits = std::vector<StoryBit>;

    Bits operator+(Bits a, const Bits& b) {
        a.insert(a.end(), b.begin(), b.end());
        return a;
    }

    // What the game has set by each step of its task list (docs/story-points.md has each bit's evidence).
    const Bits to_first_shrine = {
        { 0xD0, 1 }, { 0xB4, 1 }, { 0x2B2, 1 }, { 0x70, 1 }, { 0x37A, 1 }, { 0x4D3, 1 }, { 0xAB, 1 }, { 0xBF, 1 },
        { 0x15A, 1 }, { 0x3A0, 1 }, { 0x88, 1 }, { 0x3B0, 1 }, { 0x2C5, 1 }, { 0x615, 1 }, { 0x2D, 1 }, { 0x1D7, 1 },
    };
    const Bits first_spirit = { { 0x316, 1 }, { 0x21F, 1 } };
    const Bits first_stone = { { 0x104, 1 }, { 0x22E, 1 }, { 0x877, 1 } };
    const Bits second_spirit = { { 0x277, 1 }, { 0x21B, 1 } };
    const Bits third_spirit = { { 0x317, 1 }, { 0x21C, 1 } };
    const Bits fourth_spirit = { { 0x318, 1 }, { 0x21D, 1 } };
    const Bits four_spirits = to_first_shrine + first_spirit + first_stone + second_spirit + third_spirit + fourth_spirit;

    const std::vector<StoryPoint> points = {
        { "new-game", krystal, no_place, {}, {} },
        { "after-first-spellstone", sabre, 81, {}, to_first_shrine + first_spirit + first_stone },
        { "before-sand-worm", krystal, -1, sand_worm_place, to_first_shrine },
        { "before-df-shrine", krystal, 36, {}, to_first_shrine },
        { "before-db-shrine", sabre, 38, {}, to_first_shrine + first_spirit + first_stone },
        { "before-mm-shrine", krystal, 43, {}, to_first_shrine + first_spirit + first_stone + second_spirit },
        { "before-ec-shrine", sabre, 40, {}, to_first_shrine + first_spirit + first_stone + second_spirit + third_spirit },
        { "before-gp-shrine", krystal, 42, {}, four_spirits },
        { "before-nw-shrine", krystal, 37, {}, four_spirits },
        { "before-cc-shrine", krystal, 41, {}, four_spirits },
        { "before-wg-shrine", krystal, 39, {}, four_spirits },
    };

    const StoryPoint* chosen = &points[0];
    std::vector<StoryBit> extra_bits;
    int chosen_warp = -1;
    bool place_given = false;
    Place given_place{};

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

    Place warp_entry(int index) {
        uint32_t entry = warp_table_rom + 16 * index;
        uint32_t words[3] = { dino_rom_word(entry), dino_rom_word(entry + 4), dino_rom_word(entry + 8) };
        Place place;
        memcpy(&place, words, sizeof(words));
        place.layer = (int32_t)dino_rom_word(entry + 12);
        return place;
    }

    // Export 6 takes where (a vector in the game's memory, here under its stack), the facing, flags and the layer.
    void store_place(uint8_t* rdram, recomp_context* ctx, const Place& place) {
        gpr sp = ctx->r29;
        ctx->r29 -= 0x40;
        gpr position = ctx->r29 + 0x20;
        memcpy(&MEM_W(0, position), &place.x, 4);
        memcpy(&MEM_W(4, position), &place.y, 4);
        memcpy(&MEM_W(8, position), &place.z, 4);
        ctx->r4 = position;
        ctx->r5 = 0;
        ctx->r6 = 0;
        ctx->r7 = place.layer;
        call_dll29(rdram, ctx, 6);
        ctx->r29 = sp;
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

void dino::story_set_place(const char* list) {
    place_given = sscanf(list, "%f,%f,%f,%d", &given_place.x, &given_place.y, &given_place.z, &given_place.layer) >= 3;
}

void dino::story_start(uint8_t* rdram, recomp_context* ctx) {
    const StoryPoint& point = *chosen;
    ctx->r4 = 0;
    call(rdram, ctx, set_paused);
    ctx->r4 = (uint32_t)-1;
    ctx->r5 = 0;
    call_dll29(rdram, ctx, 1);
    ctx->r4 = point.character;
    call_dll29(rdram, ctx, 29);
    for (const StoryBit& b : point.bits) {
        set_game_bit(rdram, ctx, b);
    }
    for (const StoryBit& b : extra_bits) {
        set_game_bit(rdram, ctx, b);
    }
    if (place_given) {
        store_place(rdram, ctx, given_place);
    }
    else if (chosen_warp != no_place) {
        store_place(rdram, ctx, chosen_warp >= 0 ? warp_entry(chosen_warp) : point.place);
    }
    ctx->r4 = 1;
    call(rdram, ctx, play_flag_2000);
    ctx->r4 = 1;
    call(rdram, ctx, play_flag_10);
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
