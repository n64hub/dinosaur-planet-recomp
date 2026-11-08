// BOSS Kamerian Dragon (map 53), under Dragon Rock, and the fourth SpellStone. Nothing leads to the room in the ROM,
// and the fight is built but can't be won: the dragon's health runs down and nothing reads it, and no object anywhere
// gives the fourth SpellStone (0x7A8), which the Desert Force Point's other SpellPlace takes (docs/story-points.md,
// Dragon Rock). With Kamerian Dragon on Restored (the default) the Skeetla arena, once won, leads into the room, the
// dragon dies when its health runs out and leaves the fourth SpellStone, and the stone leads back out; on Original it
// plays as the console does.
//
// The room, read from the ROM:
// - Nothing joins it to anything: no warp entry lies in it (ROM file 30), no code warps to map 53, and its entrance, a
//   TriggerPlane at its north (uid 0x41AF3, local 729,-1769,-835) that sets 0x4D7 for the KD_Sequences there, faces cells
//   with no blocks, where the player falls and is put back.
// - It is the bottom's Skeetla arena (map 52, layer -1) on layer 0: both lie at the world cell (2, -8), over the same
//   ground at the same height (the arena's floor at y -1831, the room's at -1805), and where the arena has its
//   DRLavaControls and DR_Creators (1615,-6339 and 2240,-6492) the room has its KamColumns and KamSwitches' pits
//   (1623,-6482 and 2217,-6484).
// - The arena's fight is won when 0x628 is set (DR_EarthWarrior, dll713, once all three DRLavaControls are out, 0x789
//   at 3, and no Skeetla is left), which plays the DR_Seqobj uid 0x339C6's sequence 7, whose end sets 0x609 (on the top it
//   opens the CloudRunner's cage). A DRLavaControl (dll737) loses a point of its health (+0x1A) to each hit of kind
//   0x19 (export 2, 0x0170), and at 0 sets its bit (+0x1E) and raises 0x789.
// - Warp entries 84 and 89 are empty, 89 between Dragon Rock - Bottom's 88 and BOSS TRex's 90.
//
// The fight, read from the ROM (offsets are a DLL's text):
// - KamerianBoss (dll707, object 0x4D9) keeps its state in the word at its +0xB8 pointer, its health in bits 9-16,
//   10 at the start (0x3B8). Its update (0x0FC4) lowers it by one for each hit taken in its last stage (0x1B90-0x1BB4),
//   and nothing reads it: at 0 it wraps to 255 and the fight goes on. The update also fills the player's magic.
// - Its stages: a hit of any kind on a head's spheres (13 and 14 on the left, 8 and 9 on the right) lights it, a hit of
//   type 0xF within about 9 s provokes it, and a hit of type 0xF on sphere 0 or 1 finishes that head; then the last
//   stage, with its health bar and flame. Type 0xF is the projectile spell's (projball, dll255, the scroll's item
//   0x2D), cast from the spell scroll (C-left): A on it aims, A fires.
// - The Nurse (dll708) circles the player and knocks her back, into the pit beside the dragon too; a KamSwitch (dll285)
//   hit by the projectile raises the KamColumn (dll709) out of it.
// - CRSpellstone (dll730, object 0x390) is the SpellStone CloudRunner's race drops: +0x1E the bit set once picked up,
//   +0x20 a bit that hides it (both checked each update), and at its state 2 it is picked up by walking within 105
//   units of it (0x01A8). A stone already picked up when it is placed sets 0x49A (its table at 0x3F4), which the race
//   in map 43 has already set by then.
//
// What Restored changes:
// - The way in and out: in the arena on layer -1 once 0x609 is set and the dragon isn't beaten, the player is warped
//   by entry 89, filled with the room's setuppoint (uid 0x3534A); in the room 3 s after the stone is picked up, by entry
//   84, filled with the arena's middle (TriggerPlane uid 0x33F78), on layer -1. The main thread's loop asks each pass
//   (us.toml's hook on func_800145AC), and the warp fades as the game's own do. The same hook fills the arena's magic.
// - The health's store (0x1BB8): at 0 the dragon is beaten. A CRSpellstone is placed at the player, its +0x1E 0x7A8
//   and +0x20 0x5F4 (the stone placed at the Desert Force Point), and picked up there at once.
// - The dragon's and the Nurse's updates: once it is beaten (0x7A8 or 0x5F4 set, so on every later visit too) each is
//   hidden (+0x6 0x4000, as CRSpellstone hides itself) and taken out of the hit list (func_80022274), and its update
//   is skipped.
// - The ice spell (0x5CE), the only spell that puts out a DRLavaControl: its pickup on the top reaches far enough for
//   the player to take it, and the scroll aims and casts it as it does the projectile (below).
// - The arena's magic: while its fight is on, the player's magic is filled there as the dragon's fight fills it
//   (below), so its three DRLavaControls can be put out.
// - The arena's DR_EarthWarrior: fed the blue mushrooms placed by the arena, as his update feeds him item 0xC1 used
//   on him, which Krystal's item menu lists and he takes while the fight is on (below).

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string kamerian_dragon = "kamerian_dragon";

    enum class KamerianDragon {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> kamerian_dragon_options = {
        { KamerianDragon::Original, "Original" },
        { KamerianDragon::Restored, "Restored" },
    };

    bool restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(kamerian_dragon)) ==
            (uint32_t)KamerianDragon::Restored;
    }

    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t new_placement = 0x800233F4; // (size, object id): a zeroed placement record
    constexpr int32_t spawn_object = 0x800213EC;  // (placement, 5, map, -1, [sp+0x10] parent): the object
    constexpr int32_t leave_hit_list = 0x80022274;
    constexpr int dragon_map = 53;
    constexpr int dragon_object = 0x4D9;
    constexpr int spellstone = 0x390;
    constexpr int fourth_stone_carried = 0x7A8;
    constexpr int fourth_stone_placed = 0x5F4;
    constexpr int ice_spell = 0x5CE;

    // DINO_LOG_DRAGON set (development): the dragon's health as it falls, and its end, are printed.
    const bool logging = std::getenv("DINO_LOG_DRAGON") != nullptr;
    // DINO_DRAGON_HEALTH=<n> (development): the dragon starts the fight with n health, to play its end in a short run.
    const int start_health = std::getenv("DINO_DRAGON_HEALTH") ? std::atoi(std::getenv("DINO_DRAGON_HEALTH")) : -1;

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

    int get(uint8_t* rdram, recomp_context* ctx, int bit) {
        ctx->r4 = bit;
        call(rdram, ctx, get_bit);
        return (int32_t)ctx->r2;
    }

    bool beaten(uint8_t* rdram, recomp_context* ctx) {
        return get(rdram, ctx, fourth_stone_carried) || get(rdram, ctx, fourth_stone_placed);
    }

    void place_stone(uint8_t* rdram, recomp_context* ctx) {
        call(rdram, ctx, player_object);
        gpr player = ctx->r2;
        if (player == 0) {
            return;
        }
        ctx->r4 = 0x24;
        ctx->r5 = spellstone;
        call(rdram, ctx, new_placement);
        gpr p = ctx->r2;
        MEM_B(2, p) = 0x24 / 4;
        MEM_W(0x8, p) = MEM_W(0x18, player);
        MEM_W(0xC, p) = MEM_W(0x1C, player);
        MEM_W(0x10, p) = MEM_W(0x20, player);
        MEM_H(0x1E, p) = fourth_stone_carried;
        MEM_H(0x20, p) = fourth_stone_placed;
        ctx->r4 = p;
        ctx->r5 = 5;
        ctx->r6 = dragon_map;
        ctx->r7 = -1;
        MEM_W(0x10, ctx->r29) = 0;
        call(rdram, ctx, spawn_object);
        gpr stone = ctx->r2;
        if (stone != 0) {
            MEM_B(0, (gpr)MEM_W(0xB8, stone)) = 2; // dropped, as its export 0x2F0 makes it
        }
        if (logging) {
            fprintf(stderr, "dragon: beaten, SpellStone placed at the player (object 0x%08X)\n", (uint32_t)stone);
        }
    }

    void hide(uint8_t* rdram, recomp_context* ctx, gpr object) {
        if (MEM_H(0x6, object) & 0x4000) {
            return;
        }
        MEM_H(0x6, object) = MEM_H(0x6, object) | 0x4000;
        ctx->r4 = object;
        call(rdram, ctx, leave_hit_list);
    }

    // The way in and out. The layer the player is on is the signed byte at 0x80092A8C (func_80047374 raises it).
    constexpr int32_t player_layer = 0x80092A8C;
    constexpr int arena_won = 0x609;
    constexpr int arena_fight_won = 0x628;
    constexpr int into_room = 89;
    constexpr int out_of_room = 84;

    struct Box {
        float x0, x1, y0, y1, z0, z1;
        bool holds(float x, float y, float z) const {
            return x >= x0 && x <= x1 && y >= y0 && y <= y1 && z >= z0 && z <= z1;
        }
    };
    // In world units: the Skeetla arena at the bottom's east end (map 52, layer -1), and the dragon's room (53, layer
    // 0) over the same ground.
    const Box arena = { 1300.0f, 2500.0f, -1900.0f, -1700.0f, -6750.0f, -5900.0f };
    const Box room = { 1280.0f, 2560.0f, -2100.0f, -1600.0f, -7040.0f, -5700.0f };

    // The two warp entries, empty in ROM file 30: the room's setuppoint (map 53 @0000), and the arena where the
    // bottom's TriggerPlane 33F78 stands. +0xC and +0xE hold -1 for layer -1, as entry 88's do.
    struct DoorEntry {
        int index;
        float x, y, z;
        uint32_t layer;
    };
    const DoorEntry door_entries[] = {
        { into_room, 1925.0f, -1805.0f, -5965.0f, 0 },
        { out_of_room, 1659.0f, -1822.0f, -6121.0f, 0xFFFFFFFF },
    };
}

void dino::add_dragon_rock_options(recomp::config::Config& general) {
    general.add_enum_option(
        kamerian_dragon,
        "Kamerian Dragon",
        "The ice spell on Dragon Rock's top can be picked up and cast, and the Skeetla arena at its bottom fills "
        "Krystal's magic while its fight is on, the Earth Warrior there takes the blue mushrooms by the arena from "
        "Krystal's items, and the arena, once won, leads into the Kamerian Dragon's room. The dragon dies once its "
        "health runs out, and leaves the fourth SpellStone for the Desert Force Point, which leads back out. <b>Original is the prototype as it was left</b>: the ice spell can't be "
        "taken or cast, the arena can't be won, nothing leads to the room, the fight never ends, and the fourth "
        "SpellStone is nowhere.",
        kamerian_dragon_options,
        KamerianDragon::Restored
    );
}

// us.toml's hook in KamerianBoss's update (dll707 0x1BB8) once it has stored its health less one: $s4 its state, $s2
// the dragon.
extern "C" void dino_dragon_health(uint8_t* rdram, recomp_context* ctx) {
    int health = ((uint32_t)MEM_W(0, ctx->r20) >> 9) & 0xFF;
    if (logging) {
        fprintf(stderr, "dragon: health %d\n", health);
    }
    if (health != 0 || !restored()) {
        return;
    }
    Calls calls(ctx);
    if (!beaten(rdram, ctx)) {
        place_stone(rdram, ctx);
    }
}

// us.toml's hooks at the start of KamerianBoss's and the Nurse's updates (dll707 0x0FC4, dll708 0x0290): a0 the object.
// True skips the update.
extern "C" int dino_dragon_update(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return 0;
    }
    Calls calls(ctx);
    gpr object = ctx->r4;
    gpr placement = (gpr)MEM_W(0x4C, object);
    if (start_health >= 0 && placement != 0 && MEM_H(0, placement) == dragon_object) {
        static std::map<gpr, bool> started;
        if (!started[object]) {
            started[object] = true;
            gpr state = (gpr)MEM_W(0xB8, object);
            MEM_W(0, state) = (MEM_W(0, state) & ~(0xFF << 9)) | ((start_health & 0xFF) << 9);
        }
    }
    if (logging && placement != 0 && MEM_H(0, placement) == dragon_object) {
        static std::map<gpr, uint32_t> last;
        uint32_t state = MEM_W(0, (gpr)MEM_W(0xB8, object));
        if (last[object] != state) {
            fprintf(stderr, "dragon: 0x%08X state 0x%08X\n", (uint32_t)object, state);
            last[object] = state;
        }
    }
    if (!beaten(rdram, ctx)) {
        return 0;
    }
    hide(rdram, ctx, object);
    return 1;
}

// The ice spell's pickup on the top (ProjectileSpell, object 0x4A, dll272, uid 0x3397C), which sets 0x5CE once taken.
// A takes it within 16 of it: that is its def's reach, its target point's bytes 0 and 2 times 4 (within them the player
// highlights it, dll2_1F8C, and A marks it taken, dll2.19), and dll272's own, its state's +0x8 (0x02BC, from the same
// point). But the player stops about 18 from its middle, from whichever side she comes, so A never takes it.
constexpr int ice_spell_pickup = 0x4A;
constexpr int pickup_reach = 24;

// us.toml's hook at the start of dll272's update (0x0430): a0 the object. The ice spell's pickup reaches 24.
extern "C" void dino_ice_spell_pickup(uint8_t* rdram, recomp_context* ctx) {
    gpr object = ctx->r4;
    if (!restored() || MEM_H(0x46, object) != ice_spell_pickup) {
        return;
    }
    gpr points = (gpr)MEM_W(0x78, object);
    if (points != 0) {
        gpr point = points + 5 * MEM_BU(0xD4, object);
        MEM_B(0, point) = pickup_reach / 4;
        MEM_B(2, point) = pickup_reach / 4;
    }
    float reach = pickup_reach;
    uint32_t word;
    memcpy(&word, &reach, 4);
    MEM_W(0x8, (gpr)MEM_W(0xB8, object)) = word;
}

// The ice spell (0x5CE). The spell scroll sends the player the item it uses (dll210_1DDC, message 0x80002), and only
// the projectile (0x2D) is aimed: the player goes into the spell aim (state 0x3A, dll210_18EAC), and any other spell is
// only selected. With no target held, the aim then selects the projectile (dll210_6DD8 with 0x2D, at 0x190B8 and
// 0x19224) and holding A casts the selected spell; for 0x5CE that is the ice blast (dll210_1DC48, 0x19708), which
// nothing reaches. So the ice spell, which alone puts out a DRLavaControl, can't be cast.
static bool ice_aim = false;

// us.toml's hook in the player's messages (dll210_1DDC 0x1EF0), before it compares the item the scroll uses, in $t6,
// with the projectile: the ice spell is aimed as the projectile is, and remembered for the aim.
extern "C" void dino_ice_spell_use(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return;
    }
    ice_aim = (uint16_t)ctx->r14 == ice_spell;
    if (ice_aim) {
        ctx->r14 = 0x2D;
    }
}

// us.toml's hooks in the spell aim (dll210_18EAC 0x190B8 and 0x19224), before it selects the projectile ($a2): the
// ice spell instead, when the scroll used it.
extern "C" void dino_ice_spell_aim(uint8_t* rdram, recomp_context* ctx) {
    if (restored() && ice_aim) {
        ctx->r6 = ice_spell;
    }
}

// us.toml's hook in WARP_ENTRY (func_8004D1D4) once it has read entry 0x20(sp) of ROM file 30 into the buffer at
// 0x1C(sp), as krazoa.cpp's: the door's two entries.
extern "C" void dino_dragon_rock_warp_entry(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return;
    }
    int index = MEM_W(0x20, ctx->r29);
    gpr entry = (gpr)MEM_W(0x1C, ctx->r29);
    for (const DoorEntry& d : door_entries) {
        if (d.index == index) {
            memcpy(&MEM_W(0x0, entry), &d.x, 4);
            memcpy(&MEM_W(0x4, entry), &d.y, 4);
            memcpy(&MEM_W(0x8, entry), &d.z, 4);
            MEM_W(0xC, entry) = d.layer;
        }
    }
}

// In the arena while its fight is on, magic below a fifth of the most the player holds is filled: the ice spell, which
// alone puts out a DRLavaControl, costs a point a shot (dll277 0x0080), a DRLavaControl takes 15 hits, about 20 shots
// as it plays, and with three of them and the most 25, no magic to be had on Dragon Rock and none coming back on its
// own, the fight can't be won. The dragon's fight a layer up fills it each update (dll707 0x105C), with the player's
// export 14 (dll210_1CEFC), which plays a sound each time; this fills it with the same call, seldom enough that the
// sound marks it.
static void fill_magic(uint8_t* rdram, recomp_context* ctx, gpr player) {
    gpr stats = (gpr)MEM_W(0x34C, (gpr)MEM_W(0xB8, player));
    if (stats == 0) {
        return;
    }
    int magic = MEM_H(0x4, stats);
    int most = MEM_H(0x6, stats);
    if (magic * 5 >= most) {
        return;
    }
    gpr exports = (gpr)MEM_W(0x0, (gpr)MEM_W(0x68, player));
    ctx->r4 = player;
    ctx->r5 = most - magic;
    call(rdram, ctx, MEM_W(0x3C, exports));
    if (logging) {
        fprintf(stderr, "dragon: the arena fills magic from %d to %d\n", magic, most);
    }
}

// The arena's DR_EarthWarrior (dll713), shackled (its setup's +0x19 is 1), starts with 1 health (+0xA56 of its state,
// 0x03B8), loses one to each Skeetla's bite (0x30C0) and dies below -20 (0x5DE, the fight lost): about 45 s of bites
// from about 35 s after the arena loads. His food is the blue mushroom: SHbluemushroom (dll502) of model 0 (its
// setup's +0x20), which Dragon Rock - Bottom places four of by the arena (uids 348d8 to 348db, about 2260, -6200).
// Picked up (A on it) it raises item 0xC1 (0x0698, once the side food bag has said no), and 0xC1 used on him from the
// item menu adds 12 to his health and lowers 0xC1 by one (0x08CC-0x09A8). Two things of the ROM's stop that while the
// fight is on, and Restored lifts each:
// - Krystal's item menu doesn't list 0xC1. dll1's C-right lists the items of one of two tables, Krystal's (object
//   0x1F) or Sabre's (0x5C4-0x624), and 0xC1 is in Sabre's only (Tricky eats them, dll211), so the mushrooms she picks
//   up can't be chosen. Her table is given, where dll1 reads it (dll1_3718, whether the menu opens, and dll1_325C, its
//   list), as a copy with Sabre's 0xC1 entry after her own.
// - He can't be targeted while it is on: until it is won (0x628), his update sets +0xAF bit 3 each frame (0x0530),
//   which keeps the targeting from placing his focus point (func_80036438) and so from marking him used (dll2 0xE58).
//   While the player holds a mushroom it is left clear.
constexpr int32_t allocate = 0x80016AA8;      // (size, tag, name): a block of the game's heap
constexpr int krystal = 0x1F;
constexpr int blue_mushroom = 0xC1;
constexpr int item_entry = 12;                // dll1's table entry: +0 the item's bit, a negative one ends the table
constexpr int most_entries = 64;

// The entries of the table at table (to its end), or -1 for a table not ended within most_entries.
static int table_entries(uint8_t* rdram, gpr table) {
    for (int i = 0; i < most_entries; i++) {
        if ((int16_t)MEM_H(i * item_entry, table) < 0) {
            return i;
        }
    }
    return -1;
}

static int table_find(uint8_t* rdram, gpr table, int count, int item) {
    for (int i = 0; i < count; i++) {
        if ((int16_t)MEM_H(i * item_entry, table) == item) {
            return i;
        }
    }
    return -1;
}

// us.toml's hooks at the start of dll1_3718 and dll1_325C: a0 an item table. Sabre's follows Krystal's in dll1's data.
extern "C" void dino_item_table(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return;
    }
    gpr table = ctx->r4;
    int count = table_entries(rdram, table);
    if (count < 0 || table_find(rdram, table, count, blue_mushroom) >= 0) {
        return;
    }
    gpr next = table + (count + 1) * item_entry;
    int next_count = table_entries(rdram, next);
    int mushroom = next_count < 0 ? -1 : table_find(rdram, next, next_count, blue_mushroom);
    if (mushroom < 0) {
        return;
    }
    static gpr copy = 0;
    static int copy_count = 0;
    {
        Calls calls(ctx);
        call(rdram, ctx, player_object);
        if (ctx->r2 == 0 || (int16_t)MEM_H(0x46, ctx->r2) != krystal) {
            return;
        }
        if (copy == 0) {
            ctx->r4 = (count + 2) * item_entry;
            ctx->r5 = -1;
            ctx->r6 = 0;
            call(rdram, ctx, allocate);
            copy = ctx->r2;
            copy_count = count;
        }
    }
    if (copy == 0 || copy_count != count) {
        return;
    }
    for (int w = 0; w < item_entry / 4; w++) {
        for (int i = 0; i < count; i++) {
            MEM_W(i * item_entry + w * 4, copy) = MEM_W(i * item_entry + w * 4, table);
        }
        MEM_W(count * item_entry + w * 4, copy) = MEM_W(mushroom * item_entry + w * 4, next);
        MEM_W((count + 1) * item_entry + w * 4, copy) = MEM_W(count * item_entry + w * 4, table);
    }
    ctx->r4 = copy;
}

// us.toml's hook at dll713's store of +0xAF with bit 3 set while the fight is on (0x0540): $t2 the byte, $s1 him.
// DINO_LOG_DRAGON set (development): his health (+0xA56 of his state) is printed as it changes.
extern "C" void dino_earth_warrior_target(uint8_t* rdram, recomp_context* ctx) {
    static const bool logging = std::getenv("DINO_LOG_DRAGON") != nullptr;
    static int last_health = 0x7FFF;
    gpr state = (gpr)MEM_W(0xB8, ctx->r17);
    if (logging && state != 0 && (int16_t)MEM_H(0xA56, state) != last_health) {
        last_health = (int16_t)MEM_H(0xA56, state);
        fprintf(stderr, "dragon: the warrior's health %d\n", last_health);
    }
    if (!restored()) {
        return;
    }
    int mushrooms;
    {
        Calls calls(ctx);
        mushrooms = get(rdram, ctx, blue_mushroom);
    }
    if (mushrooms > 0) {
        ctx->r10 &= ~(gpr)0x8;
    }
}

// us.toml's hook at the start of func_800145AC, once a pass of the main thread's loop: the warp the door takes now, or
// -1. In the arena once it is won and the dragon isn't, into the room; in the room 3 s after the SpellStone is
// picked up, back to the arena.
extern "C" int dino_dragon_rock_door(uint8_t* rdram, recomp_context* ctx) {
    if (!restored()) {
        return -1;
    }
    using clock = std::chrono::steady_clock;
    static clock::time_point quiet_until; // a warp's fade takes a while: nothing more until it is done
    static clock::time_point carried_at;
    static bool carried = false;
    clock::time_point now = clock::now();
    if (now < quiet_until) {
        return -1;
    }
    Calls calls(ctx);
    call(rdram, ctx, player_object);
    gpr player = ctx->r2;
    if (player == 0) {
        return -1;
    }
    float x, y, z;
    memcpy(&x, &MEM_W(0x18, player), 4);
    memcpy(&y, &MEM_W(0x1C, player), 4);
    memcpy(&z, &MEM_W(0x20, player), 4);
    int layer = MEM_B(0, (gpr)player_layer);
    int warp = -1;
    if (layer == -1 && arena.holds(x, y, z)) {
        if (!get(rdram, ctx, arena_fight_won)) {
            fill_magic(rdram, ctx, player);
        }
        if (get(rdram, ctx, arena_won) && !beaten(rdram, ctx)) {
            warp = into_room;
        }
    }
    else if (layer == 0 && room.holds(x, y, z) && beaten(rdram, ctx)) {
        if (!carried) {
            carried = true;
            carried_at = now;
        }
        else if (now - carried_at >= std::chrono::seconds(3)) {
            warp = out_of_room;
        }
    }
    else {
        carried = false;
    }
    if (warp >= 0) {
        carried = false;
        quiet_until = now + std::chrono::seconds(5);
        if (logging) {
            fprintf(stderr, "dragon: the door warps to %d from %.0f %.0f %.0f, layer %d\n", warp, x, y, z, layer);
        }
    }
    return warp;
}
