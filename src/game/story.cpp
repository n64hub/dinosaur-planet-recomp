// --story, --place and --bits (development, src/main/main.cpp reads them): start a run from a chosen point of the
// story, for what a warp can't set. --watch-bits says each time the game sets a bit, for what a test changes. docs/story-points.md lists the points and what each bit is, with its evidence from
// the ROM.
//
// The story is the game's bits: func_80014670 sets one and func_800147EC reads one, a number that the table in ROM file
// 55 turns into a place in one of the save's four banks. Which spirits Krystal has brought back and which SpellStones
// are placed are bits, and so are what a level has opened and each pickup taken. dll29 holds the save: its export 1
// clears the save to a new game's (slot -1 is no slot, so nothing touches the Flash), 29 sets the character, 6 stores
// where play starts and 7 starts the world there. The game's own New Game is dll64's name screen calling export 1, then
// dll63 dropping to play's video mode, letting go of what the title holds, calling the two play flags and export 7;
// story_start does the same with the bits set and the place stored before export 7, since export 7 keeps a copy that a
// retry restores. func_800142F0 is the
// rolling demo, not a new game.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "recomp.h"

#include "dino.h"

extern "C" uint32_t dino_rom_word(uint32_t offset);
extern "C" double dino_script_seconds();

namespace {
    constexpr int32_t dll29_handle = 0x8008C9F4; // the pointer the game keeps to its loaded dll29
    constexpr int32_t save_pointer = 0x800AE2A0; // the save in dll29's bss, which func_80014670 writes to
    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t map_at = 0x80044810; // the map at a position: x in f12, z in f14
    constexpr int32_t set_paused = 0x8001440C;
    constexpr int32_t current_menu = 0x800A7D84;
    // The game boots in a 640x480 mode for its logos, and dll63 and dll65 drop to play's 320x240 with this, mode 1 and
    // the settings func_80014604 returns. In the boot's mode the Z buffer (0x80093010) is the first framebuffer, so a
    // level played in it shows depth as colour in a third of its frames.
    constexpr int32_t set_video_mode = 0x8005D410;
    constexpr int32_t video_settings = 0x80014604;
    // What dll63, the front end, calls with 1 before it starts play: each changes a flag in the word at 0x80092A98,
    // and without them the objects aren't drawn and the opening's textures are wrong.
    constexpr int32_t play_flag_2000 = 0x80041D20;
    constexpr int32_t play_flag_10 = 0x80041C6C;
    // What the title takes for the menus that follow it, which dll63 lets go of as it starts play (dll63_0E28 with 1,
    // after the video mode): dll60's constructor holds texture groups 0, 4 and 5 (func_8000CB00, each a count that
    // func_8000CBC8 drops) and loads DLLs 75 and 76 (func_80014B1C, kept at 0x8008C9EC and 0x8008CA04, unloaded by
    // func_80014B6C). Both drops do nothing to what isn't held. Kept through play, they leave the opening's first room,
    // NewIceMt1 (warps 2 and 26), crashing as it loads.
    constexpr int32_t drop_texture_group = 0x8000CBC8;
    constexpr int32_t unload_title_dlls = 0x80014B6C;
    constexpr int title_texture_groups[] = { 4, 5, 0 };
    constexpr uint32_t warp_table_rom = 0x020E023A; // ROM file 30

    struct StoryBit {
        int bit;
        int value;
        double at = -1.0; // with --bits <hex>@<seconds>, when on the --input clock; -1 is at the story's start
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

    // Cape Claw's sand worm has no entry in the warp table. Map 29's record in ROM file 21 is at grid cell (2, 26) of
    // 640 units, so the worm's object at (2155, -276, -206) is at (3435, -276, 16434), and this is the setuppoint
    // object the map has beside it at (2340, -265, -435).
    constexpr Place sand_worm_place = { 3620.0f, -265.0f, 16205.0f, 0 };

    using Bits = std::vector<StoryBit>;

    Bits operator+(Bits a, const Bits& b) {
        a.insert(a.end(), b.begin(), b.end());
        return a;
    }

    // What the game has set by each step of its task list (docs/story-points.md has each bit's evidence).
    const Bits to_swapstone_hollow = {
        { 0xD0, 1 }, { 0xB4, 1 }, { 0x2B2, 1 }, { 0x70, 1 }, { 0x37A, 1 }, { 0x4D3, 1 }, { 0xAB, 1 },
    };
    const Bits to_first_shrine = to_swapstone_hollow + Bits {
        { 0xBF, 1 }, { 0x15A, 1 }, { 0x3A0, 1 }, { 0x88, 1 }, { 0x3B0, 1 }, { 0x2C5, 1 }, { 0x615, 1 }, { 0x2D, 1 },
        { 0x1D7, 1 },
    };
    const Bits first_spirit = { { 0x316, 1 }, { 0x21F, 1 } };
    const Bits first_stone = { { 0x104, 1 }, { 0x22E, 1 }, { 0x877, 1 } };
    const Bits second_spirit = { { 0x277, 1 }, { 0x21B, 1 } };
    const Bits third_spirit = { { 0x317, 1 }, { 0x21C, 1 } };
    const Bits fourth_spirit = { { 0x318, 1 }, { 0x21D, 1 } };
    const Bits four_spirits = to_first_shrine + first_spirit + first_stone + second_spirit + third_spirit + fourth_spirit;
    // The SpellStones after the first, each by the SpellPlace that takes it: a placement's +0x1E is its placed bit and
    // +0x20 the bit of the stone carried (VFP_SpellPlace 637 in map 4, DFP_SpellPlace in map 21). CloudRunner's, won in
    // the race (CRSpellstone, map 43, 0x49F), is carried as 0x2E8 to the Desert Force Point (placed, 0x5F3); Walled
    // City's is carried as 0x83B to the Volcano Force Point's second place (0x878).
    const Bits to_second_stone = to_first_shrine + first_spirit + first_stone + second_spirit;
    const Bits second_stone_carried = { { 0x49F, 1 }, { 0x2E8, 1 } };
    const Bits second_stone = { { 0x49F, 1 }, { 0x5F3, 1 } };
    const Bits third_stone_carried = { { 0x83B, 1 } };
    const Bits third_stone = { { 0x878, 1 } };

    // BOSS Drakor (map 44) is in no way of the story's (docs/story-points.md, The story after the first SpellStone):
    // its setuppoint, at grid cell (7, 8).
    constexpr Place drakor_place = { 4810.0f, 6.0f, 7109.0f, 0 };
    // Dragon Rock - Top (map 2): its setuppoint 2. The map's setuppoint 0 is in a cell of the map's that holds no
    // geometry, and a start in a cell no map fills loads nothing (func_80045DC0 finds no map there).
    constexpr Place dragon_rock_place = { -3464.0f, -1502.0f, -4482.0f, 0 };

    const std::vector<StoryPoint> points = {
        { "new-game", krystal, no_place, {}, {} },
        { "swapstone-hollow", sabre, 15, {}, to_swapstone_hollow },
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
        { "before-cloudrunner", krystal, 73, {}, to_second_stone },
        { "before-desert-force-point", krystal, 31, {}, to_second_stone + second_stone_carried },
        { "before-walled-city", sabre, 91, {}, to_second_stone + second_stone },
        { "before-third-stone", sabre, 66, {}, to_second_stone + second_stone + third_stone_carried },
        { "before-dragon-rock", krystal, -1, dragon_rock_place, four_spirits + second_stone + third_stone },
        { "before-drakor", krystal, -1, drakor_place, four_spirits + second_stone + third_stone },
    };

    const StoryPoint* chosen = &points[0];
    std::vector<StoryBit> extra_bits;
    std::vector<StoryBit> timed_bits;
    int chosen_warp = -1;
    std::vector<int> watched_bits;
    bool watch_all_bits = false;
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
            const char* at = strchr(p, '@');
            const char* next = strchr(p, ',');
            if (at != nullptr && (next == nullptr || at < next)) {
                b.at = atof(at + 1);
                timed_bits.push_back(b);
            }
            else {
                extra_bits.push_back(b);
            }
        }
    }
}

void dino::story_watch_bits(const char* list) {
    watch_all_bits = strcmp(list, "all") == 0;
    for (const char* p = list; !watch_all_bits && p != nullptr; p = strchr(p, ',') ? strchr(p, ',') + 1 : nullptr) {
        int bit;
        if (sscanf(p, "%x", &bit) == 1) {
            watched_bits.push_back(bit);
        }
    }
}

// us.toml's hook at the start of func_80014670, which sets bit $a0 to $a1: a watched bit's line says the value it had,
// and with "all" only a bit that changes has one.
extern "C" void dino_bit_setting(uint8_t* rdram, recomp_context* ctx) {
    int bit = (int32_t)ctx->r4, value = (int32_t)ctx->r5;
    // func_80014670 returns at once for bit -1 (0x80014694), which objects with no bit of their own pass every frame.
    if (bit == -1) {
        return;
    }
    if (!watch_all_bits &&std::find(watched_bits.begin(), watched_bits.end(), bit) == watched_bits.end()) {
        return;
    }
    recomp_context saved = *ctx;
    call(rdram, ctx, get_bit);
    int old = (int32_t)ctx->r2;
    *ctx = saved;
    if (!watch_all_bits || old != value) {
        fprintf(stderr, "bits: %.1f s 0x%X = %d, was %d\n", dino_script_seconds(), bit, value, old);
    }
}

// Each pass of the main loop: the --bits given a time, once it has come, as a test's own object would set them.
void dino::story_timed_bits(uint8_t* rdram, recomp_context* ctx, double seconds) {
    for (StoryBit& b : timed_bits) {
        if (b.at >= 0.0 && seconds >= b.at) {
            b.at = -1.0;
            set_game_bit(rdram, ctx, b);
            fprintf(stderr, "--bits: 0x%X = %d at %.1f s\n", b.bit, b.value, seconds);
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
    call(rdram, ctx, video_settings);
    ctx->r5 = ctx->r2;
    ctx->r4 = 1;
    ctx->r6 = 0;
    call(rdram, ctx, set_video_mode);
    for (int group : title_texture_groups) {
        ctx->r4 = group;
        call(rdram, ctx, drop_texture_group);
    }
    call(rdram, ctx, unload_title_dlls);
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
        fprintf(stderr, "story: %.1f s no player, menu %d, health %d/%d\n", seconds, word(rdram, current_menu), MEM_BU(1, stats), MEM_BU(2, stats));
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
    fprintf(stderr, "story: %.1f s map %d at %.0f %.0f %.0f menu %d, health %d/%d\n", seconds, (int32_t)ctx->r2, fx, fy, fz, word(rdram, current_menu), MEM_BU(1, stats), MEM_BU(2, stats));
}
