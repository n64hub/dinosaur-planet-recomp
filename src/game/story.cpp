// --story, --place, --bits and --time (development, src/main/main.cpp reads them): start a run from a chosen point of
// the story, for what a warp can't set. --watch-bits says each time the game sets a bit, for what a test changes. docs/story-points.md lists the points and what each bit is, with its evidence from
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
#include <cmath>
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
extern "C" void dino_input_bit_set(int bit, int value);

namespace {
    constexpr int32_t dll29_handle = 0x8008C9F4; // the pointer the game keeps to its loaded dll29
    // dll7, the sky and the time of day: its export 9 sets the time (f12, in seconds of a day of 86400, taken modulo a
    // day; dll7 0xEA0-0xF18), which it keeps at +0xC0 of its environment and copies into the save each frame
    // (dll29's export 38, dll7 0xBEC), export 4 writes it to the float its $a0 points at (dll7 0xCDC), and export 8
    // says night from 21:00 to 5:00 (75600 and 18000, dll7 0xDF4).
    constexpr int32_t dll7_handle = 0x8008C980;
    constexpr int dll7_get_time = 4;
    constexpr int dll7_set_time = 9;
    constexpr int32_t save_pointer = 0x800AE2A0; // the save in dll29's bss, which func_80014670 writes to
    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t map_at = 0x80044810; // the map at a position: x in f12, z in f14
    constexpr int32_t set_paused = 0x8001440C;
    constexpr int32_t current_menu = 0x800A7D84;
    // Every object: the array this points at, and how many it holds (func_80020C48 frees each from there).
    constexpr int32_t objects = 0x800B1920;
    constexpr int32_t object_count = 0x800B1924;
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
        int facing = 0; // a 16-bit angle, of which the save keeps the top byte: 0 faces -z
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
        { 0x1D7, 1 }, { 0x40, 1 },
    };
    const Bits first_spirit = { { 0x316, 1 }, { 0x21F, 1 } };
    const Bits first_stone = { { 0x104, 1 }, { 0x22E, 1 }, { 0x4E9, 1 }, { 0x877, 1 } };
    const Bits second_spirit = { { 0x277, 1 }, { 0x21B, 1 } };
    const Bits third_spirit = { { 0x317, 1 }, { 0x21C, 1 } };
    const Bits fourth_spirit = { { 0x318, 1 }, { 0x21D, 1 } };
    const Bits four_spirits = to_first_shrine + first_spirit + first_stone + second_spirit + third_spirit + fourth_spirit;
    // The SpellStones after the first, each by the SpellPlace that takes it: a placement's +0x1E is its placed bit and
    // +0x20 the bit of the stone carried (VFP_SpellPlace 637 in map 4, DFP_SpellPlace in map 21). CloudRunner's, won in
    // the race (CRSpellstone, map 43, 0x49F), is carried as 0x2E8 to the Desert Force Point (placed, 0x5F3); Walled
    // City's is carried as 0x83B to the Volcano Force Point's second place (0x878).
    // SwapStone Circle's act, the 4-bit field 0xE7, moves a step each time the player enters map 14 (SC_levelcontrol,
    // dll519): to 3 with 0x2C5, to 8 carrying the first spirit, to 9 once it is released (0x316). Its chief
    // (SC_chieflightfoot, dll518) gives the Moon Mountain Pass key only at act 9: talked to, he sets 0x60E, which
    // starts the sequence that sets the key, 0x611, the bit Moon Mountain Pass's padlock needs.
    const Bits circle_act_9 = { { 0xE7, 9 } };
    const Bits to_second_stone = to_first_shrine + first_spirit + first_stone + second_spirit + circle_act_9;
    const Bits second_stone_carried = { { 0x49F, 1 }, { 0x2E8, 1 } };
    const Bits second_stone = { { 0x49F, 1 }, { 0x5F3, 1 } };
    // Golden Plains' PortalSpell (dll272, map 9) gives the portal spell, 0x5BD, before the Desert Force Point (task 70);
    // Dragon Rock's DR_SupPortal (754) opens only to it.
    const Bits portal_spell = { { 0x5BD, 1 } };
    const Bits third_stone_carried = { { 0x83B, 1 } };
    // Tricky's commands (his export 13, dll211 0x10D4, offers none without it): DarkIce Mines' boss (dll565) sets it as
    // Sabre rescues him, and Northern Wastes' Tricky (dll497) as he joins.
    const Bits tricky_commands = { { 0x4E4, 1 } };
    // Tricky's flame (0x245, his command 4), which the same level gives: its SnowClaw (dll215, uid 1602) sets 0x172 as
    // it is beaten, and the SnowHorn it guards (DIMSnowHorn1 1a5e, dll712 0x2470) then talks, setting 0x245 and 0x27.
    const Bits darkice_flame = { { 0x172, 1 }, { 0x245, 1 }, { 0x27, 1 } };
    const Bits third_stone = { { 0x878, 1 } };
    // The fourth, which the Kamerian Dragon leaves with Kamerian Dragon on Restored (dragon_rock.cpp), is carried as
    // 0x7A8 to the Desert Force Point's other place (0x5F4).
    const Bits fourth_stone_carried = { { 0x7A8, 1 } };

    // BOSS Drakor (map 44) is in no way of the story's (docs/story-points.md, The story after the first SpellStone):
    // its setuppoint, at grid cell (7, 8).
    constexpr Place drakor_place = { 4810.0f, 6.0f, 7109.0f, 0 };
    // Dragon Rock - Top (map 2): its setuppoint 2. The map's setuppoint 0 is in a cell of the map's that holds no
    // geometry, and a start in a cell no map fills loads nothing (func_80045DC0 finds no map there).
    constexpr Place dragon_rock_place = { -3464.0f, -1502.0f, -4482.0f, 0 };
    // Golden Plains (map 9, grid cell (-6, 19)): its setuppoint 0, where the way from SwapStone Circle comes in. Its
    // pad's entry, 31, stands under the sand: a start there falls through the map and is put back, forever.
    constexpr Place golden_plains_place = { -940.2f, 116.3f, 15302.0f, 0 };
    // Walled City (map 13): its setuppoint 0, where the way from SwapStone Hollow comes in, outside the city's gate.
    constexpr Place walled_city_place = { -8634.0f, -733.0f, -4413.0f, 0 };

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
        { "before-desert-force-point", krystal, -1, golden_plains_place, to_second_stone + second_stone_carried },
        { "before-walled-city", sabre, -1, walled_city_place,
            to_second_stone + second_stone + portal_spell + tricky_commands + darkice_flame },
        { "before-third-stone", sabre, 66, {},
            to_second_stone + second_stone + portal_spell + tricky_commands + darkice_flame + third_stone_carried },
        { "before-dragon-rock", krystal, -1, dragon_rock_place, four_spirits + second_stone + portal_spell + third_stone },
        { "after-dragon-rock", krystal, -1, golden_plains_place,
            four_spirits + second_stone + portal_spell + third_stone + fourth_stone_carried },
        { "before-drakor", krystal, -1, drakor_place, four_spirits + second_stone + portal_spell + third_stone },
    };

    const StoryPoint* chosen = &points[0];
    std::vector<StoryBit> extra_bits;
    std::vector<StoryBit> timed_bits;
    bool time_due = false;
    float time_hours = 0.0f;
    double time_at = 0.0;
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
        ctx->r5 = place.facing;
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
    dino_input_bit_set(bit, value);
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

// --time <hour>[@<seconds>]: the time of day from then on, once the --input clock reaches that time (the story's start
// by default) and there is a player in a level, for a test of what plays only at night.
void dino::story_set_time(const char* arg) {
    time_hours = (float)atof(arg);
    const char* at = strchr(arg, '@');
    time_at = at != nullptr ? atof(at + 1) : 0.0;
    time_due = true;
}

void dino::story_timed_time(uint8_t* rdram, recomp_context* ctx, double seconds) {
    int32_t handle = word(rdram, dll7_handle);
    if (!time_due || seconds < time_at || handle == 0 || word(rdram, save_pointer) == 0) {
        return;
    }
    call(rdram, ctx, player_object);
    if (ctx->r2 == 0) {
        return;
    }
    time_due = false;
    int32_t table = word(rdram, handle);
    ctx->f12.fl = time_hours * 3600.0f;
    call(rdram, ctx, word(rdram, table + 4 + 4 * dll7_set_time));
    fprintf(stderr, "--time: %.2f h at %.1f s\n", time_hours, seconds);
}

void dino::story_set_warp(int index) {
    chosen_warp = index;
}

void dino::story_set_place(const char* list) {
    place_given = sscanf(list, "%f,%f,%f,%d,%d", &given_place.x, &given_place.y, &given_place.z, &given_place.layer,
        &given_place.facing) >= 3;
}

// The bits and place a start was given, forgotten before the next is given: a page gives a run one start after another
// (src/main/main.cpp, take_web_options), each with its own.
void dino::story_forget() {
    extra_bits.clear();
    timed_bits.clear();
    place_given = false;
    time_due = false;
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
    // A new game's save gives each character no magic (12 bytes each from the save's start, magic at +4 and its most
    // at +6), and a spell is cast only with some (dll210_6DD8 returns at once without): a point past the opening
    // starts both full, as health is.
    if (!point.bits.empty()) {
        gpr save = (gpr)word(rdram, save_pointer);
        for (int character : { sabre, krystal }) {
            MEM_H(4, save + 12 * character) = MEM_H(6, save + 12 * character);
        }
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

// --input's to and chase lines (src/main/main.cpp): the stick that walks the player toward (to_x, to_z), or with oid
// other than -1 toward the nearest object of that id (its halfword at +0x46) that is drawn, as the player's code reads
// the stick: up the way the camera looks at the player. An enemy beaten stays where it fell with its alpha (+0x36)
// faded to 0 (ScorpionRobot's death state, dll234 0x1CB0), so the chase goes on to the next. With sphere other than -1,
// toward that hit sphere of the object instead: its model instance's (+0x7C, the one at +0xAD) spheres at +0x24, 16
// bytes each with x, y and z at +4, from the world's origin. 0 within reach. False with
// no player, no camera or no such object.
bool dino::story_steer(uint8_t* rdram, recomp_context* ctx, int oid, int sphere, float to_x, float to_z, float reach,
    float* stick_x, float* stick_y) {
    *stick_x = 0.0f;
    *stick_y = 0.0f;
    float fx, fz;
    if (word(rdram, save_pointer) == 0 || !dino::camera_forward(&fx, &fz)) {
        return false;
    }
    call(rdram, ctx, player_object);
    gpr player = (gpr)(int32_t)ctx->r2;
    if (player == 0) {
        return false;
    }
    auto position = [&](gpr object, float* x, float* z) {
        uint32_t wx = MEM_W(0x18, object), wz = MEM_W(0x20, object);
        memcpy(x, &wx, 4);
        memcpy(z, &wz, 4);
    };
    float x, z;
    position(player, &x, &z);
    if (oid != -1) {
        float best = -1.0f;
        gpr all = (gpr)MEM_W(0, (gpr)objects);
        for (int i = 0; all != 0 && i < MEM_W(0, (gpr)object_count); i++) {
            gpr object = MEM_W(i * 4, all);
            if (object == 0 || object == player || (int16_t)MEM_H(0x46, object) != oid || MEM_BU(0x36, object) == 0) {
                continue;
            }
            float ox, oz;
            position(object, &ox, &oz);
            float d = (ox - x) * (ox - x) + (oz - z) * (oz - z);
            if (best < 0.0f || d < best) {
                best = d;
                to_x = ox;
                to_z = oz;
                gpr model = (gpr)MEM_W(0x7C, object);
                gpr instance = model != 0 ? (gpr)MEM_W(4 * (int8_t)MEM_B(0xAD, object), model) : 0;
                gpr spheres = instance != 0 ? (gpr)MEM_W(0x24, instance) : 0;
                if (sphere >= 0 && spheres != 0) {
                    uint32_t sx = MEM_W(16 * sphere + 4, spheres), sz = MEM_W(16 * sphere + 0xC, spheres);
                    uint32_t ox_ = MEM_W(0, (gpr)(int32_t)0x80092A70), oz_ = MEM_W(0, (gpr)(int32_t)0x80092A74);
                    float fsx, fsz, fox, foz;
                    memcpy(&fsx, &sx, 4);
                    memcpy(&fsz, &sz, 4);
                    memcpy(&fox, &ox_, 4);
                    memcpy(&foz, &oz_, 4);
                    to_x = fsx + fox;
                    to_z = fsz + foz;
                }
            }
        }
        if (best < 0.0f) {
            return false;
        }
    }
    float dx = to_x - x, dz = to_z - z;
    float distance = std::sqrt(dx * dx + dz * dz);
    if (distance <= reach) {
        return true;
    }
    dx /= distance;
    dz /= distance;
    *stick_y = dx * fx + dz * fz;
    *stick_x = dz * fx - dx * fz;
    return true;
}

// --input's sync near lines (src/main/main.cpp): whether a drawn object of that id (as story_steer chases) stands
// within reach of (x, z) on the ground.
bool dino::story_object_near(uint8_t* rdram, int oid, float x, float z, float reach) {
    gpr all = (gpr)MEM_W(0, (gpr)objects);
    for (int i = 0; all != 0 && i < MEM_W(0, (gpr)object_count); i++) {
        gpr object = MEM_W(i * 4, all);
        if (object == 0 || (int16_t)MEM_H(0x46, object) != oid || MEM_BU(0x36, object) == 0) {
            continue;
        }
        uint32_t wx = MEM_W(0x18, object), wz = MEM_W(0x20, object);
        float ox, oz;
        memcpy(&ox, &wx, 4);
        memcpy(&oz, &wz, 4);
        if ((ox - x) * (ox - x) + (oz - z) * (oz - z) <= reach * reach) {
            return true;
        }
    }
    return false;
}

// --input's sync act lines: whether a drawn object of that id is in that act. dll18, the state machine the game's
// characters run on, keeps the act in the object's state (obj+0xB8) at +0x26C, set by its export 4 as a state starts
// one (and the act before it at +0x26E).
bool dino::story_object_act(uint8_t* rdram, int oid, int act) {
    gpr all = (gpr)MEM_W(0, (gpr)objects);
    for (int i = 0; all != 0 && i < MEM_W(0, (gpr)object_count); i++) {
        gpr object = MEM_W(i * 4, all);
        if (object == 0 || (int16_t)MEM_H(0x46, object) != oid || MEM_BU(0x36, object) == 0) {
            continue;
        }
        gpr state = (gpr)MEM_W(0xB8, object);
        if (state != 0 && (int16_t)MEM_H(0x26C, state) == act) {
            return true;
        }
    }
    return false;
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
    int32_t map = (int32_t)ctx->r2;
    // The player's act in dll18's state machine (state +0x26C, as story_object_act reads an enemy's): a jump, a fall or
    // a climb tells by it.
    gpr player_state = (gpr)(int32_t)MEM_W(0xB8, (gpr)object);
    int act = player_state != 0 ? (int16_t)MEM_H(0x26C, player_state) : -1;
    // The time of day, in hours (-1 with no sky loaded).
    float hours = -1.0f;
    int32_t sky = word(rdram, dll7_handle);
    if (sky != 0) {
        gpr sp = ctx->r29;
        ctx->r29 -= 0x20;
        ctx->r4 = ctx->r29 + 0x10;
        MEM_W(0x10, ctx->r29) = 0;
        call(rdram, ctx, word(rdram, word(rdram, sky) + 4 + 4 * dll7_get_time));
        uint32_t t = MEM_W(0x10, ctx->r29);
        memcpy(&hours, &t, 4);
        hours /= 3600.0f;
        ctx->r29 = sp;
    }
    fprintf(stderr, "story: %.1f s map %d at %.0f %.0f %.0f menu %d, health %d/%d, magic %d/%d, %.2f h, act %d\n", seconds, map,
        fx, fy, fz, word(rdram, current_menu), MEM_BU(1, stats), MEM_BU(2, stats), (int16_t)MEM_H(4, stats),
        (int16_t)MEM_H(6, stats), hours, act);
    // DINO_REPORT_OBJECT=<object id hex>: where each object of that id is too, for an enemy a script times by, with
    // its act (dll18's, at state +0x26C, for a character that runs on it) and its use flags (obj+0xAF), and "unseen"
    // while its byte +0x36 is 0: the game's targeting (dll2 0x1FA0) passes such an object over, so A never uses it.
    // A line after it says where A does.
    static const char* report_object = getenv("DINO_REPORT_OBJECT");
    if (report_object == nullptr) {
        return;
    }
    int oid = (int)strtol(report_object, nullptr, 16);
    gpr all = (gpr)MEM_W(0, (gpr)objects);
    for (int i = 0; all != 0 && i < MEM_W(0, (gpr)object_count); i++) {
        gpr other = MEM_W(i * 4, all);
        if (other == 0 || (int16_t)MEM_H(0x46, other) != oid) {
            continue;
        }
        uint32_t ox = MEM_W(0x18, other), oz = MEM_W(0x20, other);
        memcpy(&fx, &ox, 4);
        memcpy(&fz, &oz, 4);
        gpr state = (gpr)MEM_W(0xB8, other);
        fprintf(stderr, "story: %.1f s object 0x%X at %.0f %.0f act %d use 0x%02X%s\n", seconds, oid, fx, fz,
            state != 0 ? (int16_t)MEM_H(0x26C, state) : -1, MEM_BU(0xAF, other),
            MEM_BU(0x36, other) == 0 ? " unseen" : "");
        // Its focus point, where A uses it: the player within 4 x the reach byte of it, across the ground and 100 up or
        // down (dll2 0xE58): obj+0x74's points, 24 bytes each with the world place at +0xC, obj+0x78's 5-byte records
        // with that reach at +0, and obj+0xD4 the one in use.
        gpr points = (gpr)MEM_W(0x74, other), records = (gpr)MEM_W(0x78, other);
        if (points != 0 && records != 0) {
            int k = MEM_BU(0xD4, other);
            uint32_t px = MEM_W(k * 24 + 0xC, points), pz = MEM_W(k * 24 + 0x14, points);
            memcpy(&fx, &px, 4);
            memcpy(&fz, &pz, 4);
            fprintf(stderr, "story: %.1f s object 0x%X focus at %.0f %.0f reach %d\n", seconds, oid, fx, fz,
                MEM_BU(k * 5, records) * 4);
        }
    }
}
