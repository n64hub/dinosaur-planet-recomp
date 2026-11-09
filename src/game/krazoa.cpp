// The Krazoa shrines: the eight tests that each give Krystal or Sabre a spirit, carried to Warlock Mountain (map 11) and
// released there. The prototype has all eight shrines' maps and tests, but only Discovery Falls', Diamond Bay's and Moon
// Mountain Pass's are reached from the world, and the spirits of three (Willow Grove's, Golden Plains' and Northern
// Wastes') have no step at Warlock Mountain. With Krazoa Shrines on Restored (the default) this file finishes them out
// of what the ROM already holds, and on Original it leaves them as the console plays them. Two fixes are always on,
// since each is in a shrine the prototype reaches: Moon Mountain Pass's shrine Transporter, the pad the player arrives
// on, sent the player to an empty warp entry, which has no map (here), and Golden Plains' fly baddie shot second
// players (us.toml, dll475).
//
// How a spirit goes, read from the ROM (offsets are a DLL's text, `@` a placement in its map's object list):
// - A shrine's win sets Warlock Mountain's act with dll29's export 16 (map 0xB): DF 2 (dll433 0xBC4), DB 3 (dll477
//   0xA04), MM 4 (dll466 0xA30), EC 5 (dll469 0x11A4), CC 6 (dll486 0xC44), GP 8 (dll472 0xD44), NW 9 (dll481 0xA84).
//   Willow Grove's (dll488) sets none: 7 is missing.
// - The spirit carried is a mask of the player's: its export 0x9C(mask) asks, 0xA0(mask, on) gives or takes. DF 1, DB 2,
//   MM 4, EC 8, CC 0x20, WG 0x40. 0x10 and 0x80 are given by nothing: here they are GP's and NW's.
// - WL_LevelControl (dll607) gives the act's mask on arrival unless its release is done (its update dispatches acts 1
//   to 7 at 0x298, `sltiu 8`): acts 6 and 7 read 0x366 for that (0x1544, 0x17D4), a bit nothing sets, so they gave CC's
//   and WG's spirits again on every visit. Acts 8 and 9 have no handler.
// - 0x366 is the trigger of a mode 1 seqpoint (@1D34, sequence 0x12, done bit 0) placed in act 7 only, beside WG's
//   release (@1C0C). Sequence 0x12's event (dll611 0x630) is the only code that sets Cape Claw's act 9 (map 0x1D), and
//   it sets 0x367 and 0x368 and clears 0x366. 0x367 shows a WM_WarpPoint (dll275, @1D58: +0x1A where it sends, +0x1D
//   its mode, +0x1E its radius in 4s) to entry 77, empty in ROM file 30: in mode 2 a radius of 0 skips the distance
//   (0x3E0), so it warps the moment 0x367 is set, wherever the player is. Cape Claw's act 9 loads act 8's placements
//   but 28 of its curves (dll324), none of its own and no trigger that sets an act, so Cape Claw stays in it. By its
//   west entrance it loads setuppoint 7 (map 29 @08F0, +0x18 0x0107); the one beside it (@0000, uid E7B) loads only in
//   acts 12, 13, 15 and 16, which nothing sets.
// - A WL_seqpoint (dll611, object 0x25F: +0x18 yaw, +0x19 mode, +0x1A radius, +0x1C its sequence, +0x1E trigger bit,
//   +0x20 done bit) at each spirit's site plays the release when the player walks in with its trigger bit set (mode 2),
//   or clear, setting it (mode 3). The sequence's last event takes the mask back and sets the released bits (dll611
//   0x4B4): DF 0x21B/0x277, DB 0x21F/0x316, MM 0x21C/0x317, EC 0x21D/0x318, CC 0x221 (case 15, mask at 0x8D4), WG 0x222
//   (case 17, mask at 0x778). The sequence plays where its seqpoint stands (dll3 0x8AC4).
// - The seqpoint's done bit is set the frame its sequence starts, and once set the seqpoint never plays again (dll611
//   0xD8-0x144). MM's, EC's, CC's and WG's all name bit 0, so the first of them to play stopped the other three.
//
// What Restored changes:
// - Ways in. ROM file 30's warp table has empty entries where the shrines' own Transporters send the player back: 65
//   (NW), 67 (WG), 70 (GP) and 71 (CC); WARP_ENTRY (func_8004D1D4) gets them filled. Golden Plains has its pad (map 9
//   @03F0, own entry 31) pointed at the empty entry 32: it goes to GP's shrine (42). The others get a Transporter
//   (object 0xEC, dll334: +0x19 its own entry, +0x1A where it sends, +0x20 the bit that shows it) beside the place in
//   their area the ROM ties to the shrine: EC's between Walled City's Sun and Moon stones; CC's by Cape Claw's three
//   Krazoa tablet receptacles; WG's by Willow Grove's sundial; NW's where Northern Wastes is entered. Each shows once
//   the spirit before it in the act order is released, EC's once both stones are in the TRex statues' sockets (once
//   the Moon Stone is picked up with Third SpellStone on Original, where the sockets never load).
// - EC's is the ROM's own pad, moved. Map 36, the EarthWalker Temple, an older layout of Walled City's temples (its
//   Sun and Moon stones, dial, bridge and aperture, in older placement formats), places a Transporter (@00C7) at
//   (319, -371, -959), own entry 68, to 40, ECShrine's, whose pad sends back to 68: entry 68 holds that spot. The
//   world grid (ROM file 21) gives map 36 no cell, so in world units 68 falls in the Swap Store's cells (map 51) with
//   no floor there. Entry 0 is the same: map 20, an older Krazoa Shrine with no cell, has its Transporter at 0's
//   (486, 56, 1125). Walled City (map 13), which replaced map 36, has no Transporter: Restored stands map 36's there
//   and fills 68 where it stands.
// - Golden Plains' shrine exit (map 34 @06A0) went to DB's pad (66), a copy of DB's map, and in its mode 0 it sent
//   the player out on the way to the shrine: it goes to GP's pad (70), and only once the shrine is won.
// - Willow Grove: its state 3 (dll488 0x8FC) gave the spirit only while 0x1D1, the torch puzzle's bit, was clear, and
//   the torches light themselves as a demonstration about 15 s after the map loads (dll492 0x3F4), which sets it; it
//   now asks whether WG's spirit is already won (act 7 or later). Its win sets act 7.
// - Walled City's shell game (dll469, states 3 to 5 its rounds): a round's answer wait (substep 5) counts 1000 ticks
//   down (+0x14, 0xF98) while the pick (+0x18) is -1, and on running out wrote what the last round's right pick writes
//   (state 6, 0x1B, with sounds 0x344 and 0x33E: 0xFC4-0x1038 against 0xF0C-0xF90), in any round, so a player who stood
//   still won. Running out takes a wrong pick's way (0xCD8: sequence 2, then state 9 starts the test again). Its state 6
//   asked whether the player already carries the spirit with mask 2, Diamond Bay's (0x1050), where its win gives 8
//   (0x1184), so a player still carrying Diamond Bay's spirit was given none: it asks 8.
// - Golden Plains' "already passed" check (dll472 0xBB0) read 0x12A, Moon Mountain Pass's bit, which MM's earlier win
//   always set, so GP never paid out: it reads act 8 or later instead.
// - Warlock Mountain, each visit: act 5 sets 0x142, the trigger of EC's release (@0FC0), which nothing set; act 7 sets
//   0x1D1, WG's (@1C0C), which the torches also clear; acts 8 and 9 give GP's and NW's spirits as acts 6 and 7 do
//   theirs, raise the column to an upper site (WL_colrise @1358, @1AB0, shown in acts 8 and 9 too) and place a release
//   seqpoint there: GP's at CC's site, playing CC's sequence, NW's at WG's, playing WG's, with the mask the sequence
//   takes back made 0x10 or 0x80. Their trigger bits, which a mode 3 seqpoint sets the frame its sequence starts, are
//   0x21E and 0x220, which ROM file 55 has saved between the other released bits and nothing uses, and acts 8 and 9
//   read them as released. Acts 6 and 7 read CC's and WG's released bits: 0x366 is set for one release and sequence
//   0x12 clears it, so it could not keep a second visit from giving the spirit again. The done bits of MM's, EC's, CC's
//   and WG's seqpoints become their released bits.
// - After the shrines: WG's release, taking its mask back in act 7, sets 0x366, so sequence 0x12 plays, sets Cape
//   Claw's act 9 and warps the player to 77. Its seqpoint's done bit becomes 0x367, and entry 77 is filled at the
//   setuppoint by Cape Claw's west entrance that act 9 loads.
// - Cape Claw's test (dll486) waited for 0x5B2, a bit nothing sets, where its one SabreBaddie's kill bit is 0x1E7: it
//   reads and resets 0x1E7 (us.toml).
//
// Cape Claw's Krazoa tablets: three are carried into the tablet room (map 29) and set in its receptacles, and its
// puzzle then opens the way on. Krazoa Tablets is its own setting: on Restored (the default) the third tablet is found
// and the puzzle's end leads on, and on Original both are as the console plays them. Read from the ROM:
// - CCkrazoaBits (dll458, uid 34B04) counts a tablet into 0x23B and sets 0x23C for each of 0x234, 0x236 and 0x237,
//   the bits of the three pickups. 0x234 is a Discovery Falls TriggerPlane, 0x236 a CCKrazoaTabletP (object 0x2ED,
//   dll308: +0x1B 1 hides it once its bit +0x1C is set) in the settlement room. Nothing in the ROM places one with
//   0x237, and nothing else sets it. The third is a CCKrazoaTabletP with 0x237, placed once the sand worm is dead
//   (0x3FB, dll461 0xFD0) where the worm's arena has a sequence object that nothing triggers (CCnewseqobject uid
//   42BDF, in the worm's object group, trigger bit 0x96). CCkrazoaBits keeps the three bits as it is created (0x18)
//   and counts each that has turned set since (0xB4), but it is placed by distance in the tablet room, so a tablet
//   picked up by the worm went uncounted (played): the pickup counts the third itself (dll308 0x288, once it set its
//   bit), and CCkrazoaBits keeps 0x237 as set.
// - CCkrazoabright (dll457, uid 34872) runs the room in states, its first update choosing one from bits (0xB4):
//   0x3AC 4, 0x4D6 3, 0x393 2, else 1. State 1 (0x420): the four perch switches' six lights, solved by each switch
//   once, plays sequence 0, whose data sets 0x393. State 2 (0x728): the two levers (0x41E, 0x604) set 0x4D6 and play
//   sequence 1. State 3 (0x808): the three beacons and the three tablets set in (0x3DD-0x3DF, 0x238-0x23A) play
//   sequence 3, whose data sets 0x3AC, task 136. State 4 (0xAAC): the lights again (0x4D7, 0x3F4 and 0x4D8 once
//   each) play sequence 5 and go to state 5 (0xD78), which has no handler, and sequence 5 sets nothing: the puzzle's
//   end led nowhere. Restored sets 0x52B there, a saved bit (ROM file 55, bank 1) that nothing in the code or the maps
//   uses, and the first update goes to state 5 with it set (0x108), so the solved lights are not asked for again.
// - Krazoa Palace (map 3) is placed in the world grid beside Cape Claw only, its east end at this room's local
//   coordinates. Its two KP_Transporters (dll334) send to each other, east (@0644, own entry 94) to west (@0668, 93),
//   both empty entries. A Transporter in the tablet room, shown by 0x52B, sends to 94 from its own entry 72 (empty),
//   and 93 and 94 are filled where the palace's Transporters stand.
// - The tablet room's model is in the ROM (map 29's blocks 932 and 933), and eight VisAnimators (dll354) hide it
//   and the rooms around it, each the shapes of its groups, while 0x2C0 is clear. Two TriggerPlanes toggle 0x2C0 as
//   the player crosses them (action 0x12 with 0x8000 on the bit): uid 41FC0 in the hall west of the room and 352FB in
//   the tunnel east of it, so a player who walks in has it set (played, both) and it is saved. Warp entries 71 and 72
//   land inside with no plane crossed, so they set it, which changes nothing for a player who walked in.
//
// Krazoa Palace (map 3, world = local + (6400, 0, 22400)): Krazoa Palace is its own setting. On Restored (the default)
// the palace's four wings are played in turn and the red Krazoa sends the player on to Drakor; on Original its rooms
// stand empty. Read from the ROM:
// - Its objects have defs and are placed nowhere: KP_console_fire, _eart, _wate and _magi (0x58F-0x592, dll339),
//   KP_textblock_* in the same order (0x593-0x596, dll654, DFP_Textblock's DLL), KP_RedKrazoa (0x598, dll214),
//   KP_SectionDoor (0x59C) and KP_FinDoor (0x59E, both dll307). Only its two KP_Transporters (0x59A, dll334) are
//   placed: +0x19 their own entry, which nothing reads, and +0x1A the entry they send to (0xD610), east (@0644, 94) to
//   west (@0668, 93) and back.
// - Its floors (map 3's triangles): the west Transporter stands in a high hall at y 960-973, which joins the rest by
//   no floor: below it a shaft drops to a pit at y -18 by the hub. The hub's floor is at y 0 round a solid block, and
//   a ramp from the block's east side climbs to 64, where a walkway leads south-east to the earth wing and a slope west
//   to a bridge (128-160) between the fire wing (north-west) and the water wing (south-west); from the bridge a ramp
//   climbs to a platform at 192 and a walkway north-east to the magic wing. The east Transporter's room, with the
//   room the red Krazoa stands in west of it, joins the hub by no floor either.
// - Restored places a Transporter at the shaft's top and one at its bottom, sending to warp entries 45 and 46 (empty
//   in ROM file 30). 45 is filled beside the bottom pad: an entry on a pad's own spot over the shaft drops the player
//   down it to the pit (played). 46 is filled at the west Transporter, as 93 is: it stands 13 above the high hall's
//   floor, and a player who walks at it from the hall stops 46 short and isn't taken (played), but one put on it is,
//   and walks off it east to the shaft's pad. Each wing gets a section door at its mouth, and at its end a console and a text block.
//   The blocks go off in turn, fire, earth, water, magic, each needing the last one's done bit (+0x20) and setting
//   its own (+0x1E): 0x6A7-0x6AA, saved bits in ROM file 55 that nothing in the code or the maps uses. The fire wing's
//   door opens with 0x52B, the tablet room's, and each other wing's with the block before it. The last door, between
//   the east Transporter and the red Krazoa, is KP_FinDoor's four leaves in an iris, each opened by one wing's bit.
// - The leaves have no collision, so while the door isn't open the player is held east of it. A section door's
//   collision stays where it was placed whatever its animation, so an open one is put under the floor.
// - The red Krazoa, once 0x6AA is set, sends a player who walks up to it to warp entry 44 (empty), filled at BOSS
//   Drakor's setuppoint (map 44): nothing in the ROM leads to the fight (docs/story-points.md).
//
// Every object this file places (extras, below) is made once, as its map loads, and is in no map's object list, so
// nothing makes it again until that map loads again. The object loop (func_8004A800) frees an object with no flag in
// its placement (+4) once the camera is out of its range (func_8004B190 0x8004B40C-0x8004B478), the placement's +6
// times 8 (func_80021450 0x800215B8) plus 40, and an extra's +6 is 0. Played with no flag: Warlock Mountain's release
// seqpoints in acts 8 and 9 and Cape Claw's third tablet were freed within 2 s of being placed, the player 1500 away,
// and a palace text block while the player was in another wing; the Transporters stayed. Each is placed with flag 1,
// as the palace's own KP_Transporters are (@0644, @0668): the loop keeps it however far the player goes (0x8004B1D8)
// and frees it with its map (func_8004AEFC), which is when this file places it again (played: none freed, in each of
// their maps). Flag 2 takes it out of the loop altogether (0x8004A950), its map's unloading included.

#include <algorithm>
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
    const std::string krazoa_shrines = "krazoa_shrines";

    enum class KrazoaShrines {
        Original,
        Restored,
        OptionCount
    };
    // A value saved under a key not here, as the Finished that Restored was called, loads as the default, Restored.
    const std::vector<recomp::config::ConfigOptionEnumOption> krazoa_shrines_options = {
        { KrazoaShrines::Original, "Original" },
        { KrazoaShrines::Restored, "Restored" },
    };

    const std::string krazoa_tablets = "krazoa_tablets";

    enum class KrazoaTablets {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> krazoa_tablets_options = {
        { KrazoaTablets::Original, "Original" },
        { KrazoaTablets::Restored, "Restored" },
    };

    const std::string krazoa_palace = "krazoa_palace";

    enum class KrazoaPalace {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> krazoa_palace_options = {
        { KrazoaPalace::Original, "Original" },
        { KrazoaPalace::Restored, "Restored" },
    };

    bool shrines_restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(krazoa_shrines)) ==
            (uint32_t)KrazoaShrines::Restored;
    }

    bool tablets_restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(krazoa_tablets)) ==
            (uint32_t)KrazoaTablets::Restored;
    }

    bool palace_restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(krazoa_palace)) ==
            (uint32_t)KrazoaPalace::Restored;
    }

    // Which setting an entry of the tables below belongs to.
    enum class Setting {
        Shrines,
        Tablets,
        Palace
    };

    bool on(Setting setting) {
        switch (setting) {
            case Setting::Shrines:
                return shrines_restored();
            case Setting::Tablets:
                return tablets_restored();
            default:
                return palace_restored();
        }
    }

    constexpr int32_t set_bit = 0x80014670;
    constexpr int32_t get_bit = 0x800147EC;
    constexpr int32_t player_object = 0x80023914;
    constexpr int32_t dll29_handle = 0x8008C9F4;
    constexpr int32_t new_placement = 0x800233F4; // (size, object id): a zeroed placement record
    constexpr int32_t spawn_object = 0x800213EC;  // (placement, 5, map, -1, [sp+0x10] parent), as dll434 0x1F4-0x33C
    constexpr int32_t map_structs = 0x800B5328;   // each loaded map's header, by map number (0 while not loaded)
    constexpr int32_t loading_map = 0x800B96AC;   // the header func_80045BF4 has just loaded
    constexpr int warlock_mountain = 0xB;

    constexpr int transporter = 0xEC;
    constexpr int seqpoint = 0x25F;
    constexpr int after_shrines = 0x366;      // the trigger of Warlock Mountain's sequence 0x12 (seqpoint @1D34)
    constexpr int tablet_pickup = 0x2ED;

    constexpr int increment_bit = 0x80014930; // (bit): adds one to its value
    constexpr int third_tablet = 0x237;
    constexpr int tablets_counted = 0x23B;    // the tablets counted, which the receptacles read (+0x1E)
    constexpr int tablet_new = 0x23C;         // opens the settlement room (dll307)
    constexpr int tablets_done = 0x52B;       // Cape Claw's tablet room solved, Restored
    constexpr int tablet_area = 0x2C0;        // the VisAnimators' bit that shows the tablet room's part of map 29

    constexpr int kp_transporter = 0x59A;
    constexpr int kp_red_krazoa = 0x598;
    constexpr int kp_section_door = 0x59C;
    constexpr int kp_last_door = 0x59E;
    constexpr int kp_console_fire = 0x58F;    // then earth, water and magic
    constexpr int kp_textblock_fire = 0x593;  // then earth, water and magic
    constexpr int32_t warp_entry = 0x8004D1D4; // (index, 0), as --warp and WM_WarpPoint (dll275 0x584) call it
    constexpr int palace = 3;
    constexpr int drakor_entry = 44;
    constexpr float red_krazoa_x = 9140.0f, red_krazoa_z = 23040.0f, red_krazoa_reach = 100.0f;
    bool sent_to_drakor = false;
    constexpr float textblock_reach = 60.0f;

    int warlock_act = 0; // as WL_LevelControl last read it, for dll611's release

    // DINO_LOG_KRAZOA set (development): each object placed, spirit given and mask taken back is printed.
    const bool logging = std::getenv("DINO_LOG_KRAZOA") != nullptr;

    int32_t word(uint8_t* rdram, int32_t address) {
        return MEM_W(0, (gpr)address);
    }

    void call(uint8_t* rdram, recomp_context* ctx, int32_t function) {
        LOOKUP_FUNC(function)(rdram, ctx);
    }

    int get(uint8_t* rdram, recomp_context* ctx, int bit) {
        ctx->r4 = bit;
        call(rdram, ctx, get_bit);
        return (int32_t)ctx->r2;
    }

    void set(uint8_t* rdram, recomp_context* ctx, int bit, int value) {
        ctx->r4 = bit;
        ctx->r5 = value;
        call(rdram, ctx, set_bit);
    }

    void set_if_clear(uint8_t* rdram, recomp_context* ctx, int bit) {
        if (!get(rdram, ctx, bit)) {
            set(rdram, ctx, bit, 1);
        }
    }

    // dll29's export n is word n + 1 of the table its handle points at: 15 reads a map's act, 16 sets it.
    int warlock_act_now(uint8_t* rdram, recomp_context* ctx) {
        ctx->r4 = warlock_mountain;
        call(rdram, ctx, word(rdram, word(rdram, word(rdram, dll29_handle)) + 4 + 4 * 15));
        return (int32_t)ctx->r2;
    }

    void set_warlock_act(uint8_t* rdram, recomp_context* ctx, int act) {
        ctx->r4 = warlock_mountain;
        ctx->r5 = act;
        call(rdram, ctx, word(rdram, word(rdram, word(rdram, dll29_handle)) + 4 + 4 * 16));
    }

    // The player's own functions are reached through the table at **(player + 0x68), as dll611 0x760 does.
    int player_call(uint8_t* rdram, recomp_context* ctx, int offset, int mask, int on) {
        call(rdram, ctx, player_object);
        int32_t player = (int32_t)ctx->r2;
        ctx->r4 = player;
        ctx->r5 = mask;
        ctx->r6 = on;
        call(rdram, ctx, word(rdram, word(rdram, word(rdram, player + 0x68)) + offset));
        return (int32_t)ctx->r2;
    }

    bool has_spirit(uint8_t* rdram, recomp_context* ctx, int mask) {
        return player_call(rdram, ctx, 0x9C, mask, 0) != 0;
    }

    void give_spirit(uint8_t* rdram, recomp_context* ctx, int mask) {
        player_call(rdram, ctx, 0xA0, mask, 1);
    }

    // A hook's calls run below the hooked function's frame, and put back every register it had.
    struct Calls {
        uint8_t* rdram;
        recomp_context* ctx;
        recomp_context saved;
        Calls(uint8_t* rdram, recomp_context* ctx) : rdram(rdram), ctx(ctx), saved(*ctx) {
            ctx->r29 -= 0x40;
        }
        ~Calls() {
            *ctx = saved;
        }
    };

    struct WarpEntry {
        int index;
        float x, y, z;
        Setting setting;
        int area; // a bit that shows the part of its map the entry is in, set on the way there, or 0
    };

    // Each entry is a place the map it is in holds, in world units (a map's placement + 640 times its world grid cell,
    // ROM file 21). 70 is entry 31, GP's pad itself. 71 is the setuppoint by Cape Claw's receptacles (map 29 @22C8), 77
    // setuppoint 7 by its west entrance (map 29 @08F0).
    // 68, the ROM's EC pad's own entry (in map 36's units, header), moves with the pad to the setuppoint between Walled
    // City's two stones (map 13 @0800). 67 is setuppoint 1 by Willow Grove's sundial (map 37 @003C). 65 is the
    // setuppoint in the snow where Northern Wastes is entered (map 10 @00B0). 72 is the tablet room's Transporter, east
    // of the receptacles. 93 and 94 are Krazoa Palace's west and east KP_Transporters (map 3 @0668, @0644).
    const WarpEntry warp_entries[] = {
        { 65, -3932.0f, -110.7f, -361.2f, Setting::Shrines },
        { 67, -4523.2f, -1316.0f, -4004.5f, Setting::Shrines },
        { 68, -7939.2f, -1123.0f, -7441.1f, Setting::Shrines },
        { 70, -833.3f, 208.0f, 13285.5f, Setting::Shrines },
        { 71, 4200.3f, -7.1f, 17613.1f, Setting::Shrines, tablet_area },
        { 72, 4776.0f, -8.0f, 17606.0f, Setting::Tablets, tablet_area },
        { 77, 1456.6f, 156.1f, 16939.9f, Setting::Shrines },
        { 93, 4481.0f, 973.0f, 23358.0f, Setting::Tablets },
        { 94, 10414.0f, 23.0f, 23040.0f, Setting::Tablets },
        { 44, 4810.0f, 6.0f, 7109.0f, Setting::Palace },
        { 45, 4860.0f, 0.0f, 23358.0f, Setting::Palace },
        { 46, 4481.0f, 973.0f, 23358.0f, Setting::Palace },
    };

    // A change to one placement in a map's object list, found by its map, object id and unique id (+0x14).
    struct PlacementEdit {
        int map;
        int object;
        uint32_t uid;
        int offset;
        int size; // 1 or 2 bytes
        int value;
        bool always;
    };

    const PlacementEdit placement_edits[] = {
        // MM's shrine Transporter (map 32 @0B24) sends to entry 71, CC's: MM's pad is entry 73.
        { 32, transporter, 0x0C5D, 0x1A, 1, 73, true },
        // GP's pad (map 9 @03F0) to GP's shrine, shown once WG's spirit is released.
        { 9, transporter, 0x350D8, 0x1A, 1, 42, false },
        { 9, transporter, 0x350D8, 0x20, 2, 0x222, false },
        // GP's shrine exit (map 34 @06A0, a WM_WarpPoint, +0x1A where it sends) to GP's pad, and +0x1D's mode 0, which
        // warps within 508 units (+0x1E's 0x7F x 4) of it, 21 from the shrine, before the test, to the other shrines'
        // mode 2: only once +0x20's bit 0xFD, the win, is set (dll275 0x45C).
        { 34, 0x265, 0x2C8D, 0x1A, 1, 70, false },
        { 34, 0x265, 0x2C8D, 0x1D, 1, 2, false },
        // Warlock Mountain: the seqpoints' done bits, and the columns to the upper sites in acts 8 and 9 (a placement
        // is hidden in act n of 1-8 by bit n-1 of +3, and in act 9 by bit 7 of +5, func_8004B4A0).
        { 11, seqpoint, 0x27AB, 0x20, 2, 0x317, false }, // MM's, @2BD4
        { 11, seqpoint, 0x2983, 0x20, 2, 0x318, false }, // EC's, @0FC0
        { 11, seqpoint, 0x2AC7, 0x20, 2, 0x221, false }, // CC's, @1570
        { 11, seqpoint, 0x003C, 0x20, 2, 0x222, false }, // WG's, @1C0C
        { 11, seqpoint, 0x30055, 0x20, 2, 0x367, false }, // sequence 0x12's, @1D34
        { 11, 0x15D, 0x2AB7, 0x3, 1, 0xDF & ~0x80, false }, // CC's column, @1358: also act 8
        { 11, 0x15D, 0x002C, 0x5, 1, 0xE4 & ~0x80, false }, // WG's column, @1AB0: also act 9
        // The fallen column (WMfallencol, @1A44) lies across the way north from the pad in acts 5-13, and nothing
        // moves it (dll331 only turns it). In act 5 it shuts off all EC's course (the lasers, checkpoint 57, the
        // guards, seqpoints 296b, 2973 and 297b, the release 2983), and in acts 7 and 9 WG's column (@1AB0), which
        // stands where 2973 does. It is left out of acts 5, 7 and 9: act 6's and 8's way, CC's column, is west of it.
        { 11, 0x2BB, 0x0011, 0x3, 1, 0x0F | 0x10 | 0x40, false },
        { 11, 0x2BB, 0x0011, 0x5, 1, 0x04 | 0x80, false },
    };

    // An object placed by this file while its map is loaded: the map frees it when it unloads (+0xAC, func_8004A800).
    struct Extra {
        int map;
        int object;
        float x, y, z;
        uint8_t params[12]; // from +0x18
        int act;            // the Warlock Mountain act it is placed in, or 0 for any
        Setting setting;
        int needs;          // a bit it waits for while its map is loaded, or 0
        bool placed;
        uint32_t setup;     // its placement while it stands, for DINO_LOG_KRAZOA
    };

    // Transporters: yaw 0, own entry, destination, then dll334's 0x1E radius and its show bit, as GP's pad.
    // Seqpoints: yaw, mode 3 (walked into with its trigger clear, which it then sets), radius, sequence, trigger, no
    // done bit, at the upper sites of CC (@1570) and WG (@1C0C) with their yaws and radii.
    // Cape Claw's third tablet: the settlement room's CCKrazoaTabletP (map 29 @435C) with 0x237, at the worm arena's
    // untriggered sequence object (uid 42BDF), once the worm is dead. The tablet room's Transporter: own entry 72, to
    // the palace's east end (94), shown once the room is solved.
    Extra extras[] = {
        { 13, transporter, -7939.2f, -1123.0f, -7441.1f, { 0, 68, 40, 0, 0, 0, 0x1E, 0xFF, 0x02, 0x64, 0, 0 }, 0,
            Setting::Shrines, 0, false },
        { 29, transporter, 4200.3f, -7.1f, 17613.1f, { 0, 71, 41, 0, 0, 0, 0x1E, 0xFF, 0x03, 0x18, 0, 0 }, 0,
            Setting::Shrines, 0, false },
        { 37, transporter, -4523.2f, -1316.0f, -4004.5f, { 0, 67, 39, 0, 0, 0, 0x1E, 0xFF, 0x02, 0x21, 0, 0 }, 0,
            Setting::Shrines, 0, false },
        { 10, transporter, -3932.0f, -110.7f, -361.2f, { 0, 65, 37, 0, 0, 0, 0x1E, 0xFF, 0x02, 0x1E, 0, 0 }, 0,
            Setting::Shrines, 0, false },
        { 11, seqpoint, 13959.0f, 470.0f, 2123.0f, { 0x00, 3, 0, 0x46, 0, 0x0F, 0x02, 0x1E, 0xFF, 0xFF, 0, 0 }, 8,
            Setting::Shrines, 0, false },
        { 11, seqpoint, 12932.0f, 456.0f, 1091.0f, { 0x7F, 3, 0, 0x48, 0, 0x11, 0x02, 0x20, 0xFF, 0xFF, 0, 0 }, 9,
            Setting::Shrines, 0, false },
        { 29, tablet_pickup, 3574.0f, -257.0f, 16514.0f, { 0, 0, 0, 1, 0x02, 0x37, 0xFF, 0xFF, 0, 0, 0xFF, 0xFF }, 0,
            Setting::Tablets, 0x3FB, false },
        { 29, transporter, 4776.0f, -8.0f, 17606.0f, { 0, 72, 94, 0, 0, 0, 0x1E, 0xFF, 0x05, 0x2B, 0, 0 }, 0,
            Setting::Tablets, 0, false },
        { 3, kp_transporter, 4730.0f, 973.0f, 23358.0f, { 0, 46, 45, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_transporter, 4730.0f, -5.0f, 23358.0f, { 0, 45, 46, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_console_fire, 5300.0f, -31.0f, 21442.0f, { 0, 0, 0, 0x40, 0, 0, 0, 0, 0, 0, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_console_fire + 1, 8640.0f, -132.0f, 25432.0f, { 0, 0, 0, 0x40, 0, 0, 0, 0, 0, 0, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_console_fire + 2, 5300.0f, -63.0f, 25281.0f, { 0, 0, 0, 0x40, 0, 0, 0, 0, 0, 0, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_console_fire + 3, 8643.0f, -3.0f, 21291.0f, { 0, 0, 0, 0x40, 0, 0, 0, 0, 0, 0, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_textblock_fire, 5360.0f, -2.0f, 21442.0f, { 0x40, 0, 0x01, 0xF9, 0, 0, 0x06, 0xA7, 0xFF, 0xFF, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_textblock_fire + 1, 8640.0f, -103.0f, 25372.0f, { 0, 1, 0x01, 0xFA, 0, 0, 0x06, 0xA8, 0x06, 0xA7, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_textblock_fire + 2, 5360.0f, -34.0f, 25281.0f, { 0x40, 0, 0x01, 0xFB, 0, 0, 0x06, 0xA9, 0x06, 0xA8, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_textblock_fire + 3, 8643.0f, 26.0f, 21351.0f, { 0, 1, 0x01, 0xFC, 0, 0, 0x06, 0xAA, 0x06, 0xA9, 0, 0 }, 0,
            Setting::Palace, 0, false },
        { 3, kp_red_krazoa, 9140.0f, 0.0f, 23040.0f, { 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0 }, 0, Setting::Palace, 0, false },
        // The palace's doors: no bits and no sequence (+0x1C), so dll307 leaves them be; their yaw at +0x1F. The last
        // door's four leaves are at its centre, each with its roll at +0x1A, which dll307 doesn't read.
        { 3, kp_section_door, 5757.0f, 95.0f, 21440.0f, { 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0xFF, 0x40, 0, 0x40, 0xFF, 0xFF },
            0, Setting::Palace, 0, false },
        { 3, kp_section_door, 8638.0f, 0.0f, 24957.0f, { 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0xFF, 0, 0, 0x40, 0xFF, 0xFF },
            0, Setting::Palace, 0, false },
        { 3, kp_section_door, 5754.0f, 63.0f, 25280.0f, { 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0xFF, 0x40, 0, 0x40, 0xFF, 0xFF },
            0, Setting::Palace, 0, false },
        { 3, kp_section_door, 8641.0f, 127.0f, 21750.0f, { 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0xFF, 0, 0, 0x40, 0xFF, 0xFF },
            0, Setting::Palace, 0, false },
        { 3, kp_last_door, 10190.0f, 85.0f, 23040.0f, { 0xFF, 0xFF, 0x00, 0, 0, 0, 0xFF, 0x40, 0, 0x40, 0xFF, 0xFF },
            0, Setting::Palace, 0, false },
        { 3, kp_last_door, 10190.0f, 85.0f, 23040.0f, { 0xFF, 0xFF, 0x40, 0, 0, 0, 0xFF, 0x40, 0, 0x40, 0xFF, 0xFF },
            0, Setting::Palace, 0, false },
        { 3, kp_last_door, 10190.0f, 85.0f, 23040.0f, { 0xFF, 0xFF, 0x80, 0, 0, 0, 0xFF, 0x40, 0, 0x40, 0xFF, 0xFF },
            0, Setting::Palace, 0, false },
        { 3, kp_last_door, 10190.0f, 85.0f, 23040.0f, { 0xFF, 0xFF, 0xC0, 0, 0, 0, 0xFF, 0x40, 0, 0x40, 0xFF, 0xFF },
            0, Setting::Palace, 0, false },
    };

    // Returns its placement.
    gpr place(uint8_t* rdram, recomp_context* ctx, int map, int object, float x, float y, float z, const uint8_t* params) {
        // dll214 reads +0x24 too (its create, 0x70), so its actors get the 0x28 bytes the ROM's have.
        int size = object == kp_red_krazoa ? 0x28 : 0x24;
        ctx->r4 = size;
        ctx->r5 = object;
        call(rdram, ctx, new_placement);
        gpr p = ctx->r2;
        MEM_B(2, p) = size / 4;
        // Flag 1 (+4): kept while its map is loaded, however far the player goes (the header says why).
        MEM_B(4, p) = 1;
        memcpy(&MEM_W(0x8, p), &x, 4);
        memcpy(&MEM_W(0xC, p), &y, 4);
        memcpy(&MEM_W(0x10, p), &z, 4);
        for (int i = 0; i < 12; i++) {
            MEM_B(0x18 + i, p) = (int8_t)params[i];
        }
        ctx->r4 = p;
        ctx->r5 = 5;
        ctx->r6 = map;
        ctx->r7 = -1;
        MEM_W(0x10, ctx->r29) = 0;
        call(rdram, ctx, spawn_object);
        if (logging) {
            fprintf(stderr, "krazoa: placed object 0x%X in map %d at %.0f %.0f %.0f\n", object, map, x, y, z);
        }
        return p;
    }

    constexpr int32_t objects = 0x800B1920;      // the objects that stand, an array
    constexpr int32_t object_count = 0x800B1924;

    // Whether an object of that id stands with that placement (+0x4C).
    bool standing(uint8_t* rdram, int object, uint32_t setup) {
        int32_t all = word(rdram, objects);
        for (int32_t i = 0; all != 0 && i < word(rdram, object_count); i++) {
            int32_t o = word(rdram, all + 4 * i);
            if (o != 0 && (uint32_t)word(rdram, o + 0x4C) == setup && MEM_H(0x46, (gpr)o) == object) {
                return true;
            }
        }
        return false;
    }
}

void dino::add_krazoa_options(recomp::config::Config& general) {
    general.add_enum_option(
        krazoa_shrines,
        "Krazoa Shrines",
        "<b>Restored</b>: all eight Krazoa shrines can be reached and played, Cape Claw's test ends once its baddie is "
        "beaten, and each spirit is released at Warlock Mountain. <b>Original is the prototype as it was left</b>: three "
        "shrines are reached from the world, Cape Claw's test never ends, and Willow Grove's, Golden Plains' and "
        "Northern Wastes' spirits have nowhere to go.",
        krazoa_shrines_options,
        KrazoaShrines::Restored
    );
    general.add_enum_option(
        krazoa_tablets,
        "Krazoa Tablets",
        "Cape Claw's third Krazoa tablet is found where the sand worm fell, and the tablet room's last puzzle opens a "
        "Transporter to Krazoa Palace. <b>Original is the prototype as it was left</b>: the third tablet is nowhere, "
        "and the last puzzle leads nowhere.",
        krazoa_tablets_options,
        KrazoaTablets::Restored
    );
    general.add_enum_option(
        krazoa_palace,
        "Krazoa Palace",
        "Krazoa Palace's four wings each have a section door, a console and a text block, and the four open the last "
        "door to the red Krazoa, which sends the player on to Drakor. <b>Original is the prototype as it was left</b>: "
        "the palace's rooms stand empty and lead nowhere.",
        krazoa_palace_options,
        KrazoaPalace::Restored
    );
}

// us.toml's hook in WARP_ENTRY (func_8004D1D4) once it has read entry 0x20(sp) of ROM file 30 into the buffer at
// 0x1C(sp): 16 bytes, x, y and z as floats and an s16 layer, all zero in an empty entry.
extern "C" void dino_krazoa_warp_entry(uint8_t* rdram, recomp_context* ctx) {
    int index = MEM_W(0x20, ctx->r29);
    gpr entry = (gpr)MEM_W(0x1C, ctx->r29);
    for (const WarpEntry& w : warp_entries) {
        if (w.index == index && on(w.setting)) {
            memcpy(&MEM_W(0x0, entry), &w.x, 4);
            memcpy(&MEM_W(0x4, entry), &w.y, 4);
            memcpy(&MEM_W(0x8, entry), &w.z, 4);
            MEM_W(0xC, entry) = 0;
            if (w.area != 0) {
                Calls calls(rdram, ctx);
                set(rdram, ctx, w.area, 1);
            }
        }
    }
}

// us.toml's hook at the end of func_80045BF4, which has loaded map $s0: its header (+0x8 the object list's bytes,
// +0x20 the list) is at 0x800B96AC, and none of its objects is placed yet.
extern "C" void dino_krazoa_map_loaded(uint8_t* rdram, recomp_context* ctx) {
    int map = (int32_t)ctx->r16;
    bool on = shrines_restored();
    for (Extra& e : extras) {
        if (e.map == map) {
            e.placed = false;
        }
    }
    gpr header = (gpr)word(rdram, loading_map);
    gpr list = (gpr)MEM_W(0x20, header);
    int bytes = MEM_HU(0x8, header);
    for (int at = 0; at < bytes;) {
        gpr p = list + at;
        int size = MEM_BU(2, p) * 4;
        if (size == 0) {
            break;
        }
        for (const PlacementEdit& edit : placement_edits) {
            if (edit.map == map && (edit.always || on) && MEM_H(0, p) == edit.object &&
                (uint32_t)MEM_W(0x14, p) == edit.uid) {
                if (edit.size == 1) {
                    MEM_B(edit.offset, p) = (int8_t)edit.value;
                }
                else {
                    MEM_H(edit.offset, p) = (int16_t)edit.value;
                }
            }
        }
        at += size;
    }
}

// us.toml's hook after func_80046BC8 places the loaded maps' objects (func_8004A800), each frame from the main loop
// (func_80013D80): an Extra that needs a bit is placed the frame it is set.
extern "C" void dino_krazoa_place_objects(uint8_t* rdram, recomp_context* ctx) {
    if (!shrines_restored() && !tablets_restored() && !palace_restored()) {
        return;
    }
    Calls calls(rdram, ctx);
    if (word(rdram, map_structs + 4 * palace) == 0) {
        sent_to_drakor = false;
    }
    else if (!sent_to_drakor && palace_restored() && get(rdram, ctx, 0x6AA)) {
        call(rdram, ctx, player_object);
        int32_t player = (int32_t)ctx->r2;
        float px, pz;
        int32_t wx = word(rdram, player + 0xC), wz = word(rdram, player + 0x14);
        memcpy(&px, &wx, 4);
        memcpy(&pz, &wz, 4);
        float dx = px - red_krazoa_x, dz = pz - red_krazoa_z;
        if (player != 0 && dx * dx + dz * dz < red_krazoa_reach * red_krazoa_reach) {
            sent_to_drakor = true;
            ctx->r4 = drakor_entry;
            ctx->r5 = 0;
            call(rdram, ctx, warp_entry);
            if (logging) {
                fprintf(stderr, "krazoa: the red Krazoa sends the player to Drakor (entry %d)\n", drakor_entry);
            }
        }
    }
    // EC's Transporter (extras[0]) stands once both stones are in the TRex statues' sockets, the task list's "used Sun
    // and Moon Stones to open shrine": Walled City's act 2 loads the sockets (dll785), and each sets its done bit at
    // its sequence's event 1 (0x202 the sun's, 0x243 the moon's, dll785_0634). It is placed once the sun's is set and
    // shows at the moon's, which the moon's socket takes either first or second. Without that act (Third SpellStone on
    // Original) it stands once the Moon Stone is picked up (0x264), from the moon temple.
    Extra& ec = extras[0];
    if (!ec.placed) {
        bool sockets = dino::third_stone_restored();
        int show = sockets ? 0x243 : 0x264;
        ec.needs = !sockets ? 0x264 : get(rdram, ctx, 0x202) ? 0x243 : 0x202;
        ec.params[8] = (uint8_t)(show >> 8);
        ec.params[9] = (uint8_t)show;
    }
    for (Extra& e : extras) {
        if (word(rdram, map_structs + 4 * e.map) == 0) {
            e.placed = false;
            e.setup = 0;
        }
        else if (!e.placed && on(e.setting) && (e.needs == 0 || get(rdram, ctx, e.needs))) {
            e.placed = true;
            if (e.act == 0 || warlock_act_now(rdram, ctx) == e.act) {
                e.setup = (uint32_t)place(rdram, ctx, e.map, e.object, e.x, e.y, e.z, e.params);
            }
        }
        else if (logging && e.setup != 0 && !standing(rdram, e.object, e.setup)) {
            fprintf(stderr, "krazoa: object 0x%X in map %d at %.0f %.0f %.0f was freed with its map loaded\n", e.object,
                e.map, e.x, e.y, e.z);
            e.setup = 0;
        }
    }
}

// us.toml's hooks in Cape Claw's test (dll486) and Warlock Mountain's acts 6 and 7 (dll607), which read the bit they ask
// for from this.
extern "C" int dino_krazoa_shrines_restored(void) {
    return shrines_restored();
}

// us.toml's hook in WL_LevelControl's update (dll607 0x298), which has read Warlock Mountain's act into $v0.
extern "C" void dino_krazoa_warlock(uint8_t* rdram, recomp_context* ctx) {
    int act = (int32_t)ctx->r2;
    warlock_act = act;
    if (!shrines_restored()) {
        return;
    }
    Calls calls(rdram, ctx);
    switch (act) {
        case 5:
            if (!get(rdram, ctx, 0x318)) {
                set_if_clear(rdram, ctx, 0x142);
            }
            break;
        case 7:
            if (!get(rdram, ctx, 0x222)) {
                set_if_clear(rdram, ctx, 0x1D1);
            }
            break;
        case 8:
        case 9: {
            int released = act == 8 ? 0x21E : 0x220;
            int mask = act == 8 ? 0x10 : 0x80;
            if (!get(rdram, ctx, released)) {
                // What acts 6 and 7 set as they give their spirits (dll607 0x14FC, 0x1788).
                set_if_clear(rdram, ctx, 0x2D);
                set_if_clear(rdram, ctx, 0x1D7);
                set_if_clear(rdram, ctx, act == 8 ? 0x1CE : 0x1D1);
                if (!has_spirit(rdram, ctx, mask)) {
                    give_spirit(rdram, ctx, mask);
                    if (logging) {
                        fprintf(stderr, "krazoa: act %d gives spirit mask 0x%X\n", act, mask);
                    }
                }
            }
            break;
        }
    }
}

// us.toml's hooks in WL_seqpoint's release events (dll611), before the player's export 0xA0 takes CC's mask (0x8D4)
// and WG's (0x778) back: GP's and NW's releases play those sequences, and WG's in act 7 sets 0x366, which plays
// sequence 0x12 (seqpoint @1D34).
extern "C" void dino_krazoa_release_mask(uint8_t* rdram, recomp_context* ctx) {
    if (warlock_act == 8) {
        ctx->r5 = 0x10;
    }
    else if (warlock_act == 9) {
        ctx->r5 = 0x80;
    }
    if (logging) {
        fprintf(stderr, "krazoa: act %d release takes mask 0x%X\n", warlock_act, (int)ctx->r5);
    }
    if (warlock_act == 7 && shrines_restored()) {
        Calls calls(rdram, ctx);
        set(rdram, ctx, after_shrines, 1);
    }
}

// us.toml's hook at Willow Grove's state 3 (dll488 0x8FC), on GET_BIT(0x1D1)'s answer: with it set the shrine gives
// nothing.
extern "C" void dino_krazoa_wg_won(uint8_t* rdram, recomp_context* ctx) {
    if (!shrines_restored()) {
        return;
    }
    int won;
    {
        Calls calls(rdram, ctx);
        won = warlock_act_now(rdram, ctx) >= 7;
    }
    ctx->r2 = won;
}

// us.toml's hook at Willow Grove's state 4 (dll488 0xA18), its win, which sets no act.
extern "C" void dino_krazoa_wg_win(uint8_t* rdram, recomp_context* ctx) {
    if (!shrines_restored()) {
        return;
    }
    Calls calls(rdram, ctx);
    if (warlock_act_now(rdram, ctx) < 7) {
        set_warlock_act(rdram, ctx, 7);
    }
}

// us.toml's hook in Walled City's shell game (dll469 0xCCC), each frame of a round's answer wait, with the pick in v1
// (1 right, 0 wrong, -1 none yet): with none and the answer time running out this frame (+0x14 less the frame's ticks,
// as 0xF98 counts it), the pick is wrong.
extern "C" void dino_krazoa_ec_answer(uint8_t* rdram, recomp_context* ctx) {
    if (!shrines_restored() || (int32_t)ctx->r3 != -1) {
        return;
    }
    int32_t ticks = MEM_BU(0, (gpr)word(rdram, (int32_t)ctx->r28 + 0x2C));
    if (MEM_H(0x14, ctx->r16) - ticks > 0) {
        return;
    }
    if (logging) {
        fprintf(stderr, "krazoa: Walled City's answer time ran out in state %d\n", (int)MEM_BU(0x1B, ctx->r16));
    }
    ctx->r3 = 0;
}

// us.toml's hook at Walled City's state 6 (dll469 0x1054), which asks the player's export 0x9C whether it already
// carries the spirit, with Diamond Bay's mask in a1.
extern "C" void dino_krazoa_ec_carried(uint8_t*, recomp_context* ctx) {
    if (shrines_restored()) {
        ctx->r5 = 8;
    }
}

// us.toml's hook at Golden Plains' state 4 (dll472 0xBB0), on GET_BIT(0x12A)'s answer, which MM's win set.
extern "C" void dino_krazoa_gp_won(uint8_t* rdram, recomp_context* ctx) {
    if (!shrines_restored()) {
        return;
    }
    int won;
    {
        Calls calls(rdram, ctx);
        won = warlock_act_now(rdram, ctx) >= 8;
    }
    if (logging) {
        fprintf(stderr, "krazoa: Golden Plains' test passed, %s\n", won ? "already won" : "won now");
    }
    ctx->r2 = won;
}

// us.toml's hook at Golden Plains' timeout (dll472 0xB28), after func_800311E4 lists every carryable (group 0x12) and
// before the loop frees each one. The player's render writes the position of the object in its hands (its state's
// +0x868) every frame and calls that object's DLL, so freeing the one the player carries crashed the game. The barrel
// (dll301 0x158) lets go first through the player's export 11 (its table's +0x30) with 0; this does the same for the
// one carried, whatever the setting.
extern "C" void dino_krazoa_gp_timeout(uint8_t* rdram, recomp_context* ctx) {
    int32_t list = (int32_t)ctx->r2;
    int32_t count = word(rdram, (int32_t)ctx->r29 + 0x34);
    int32_t player = word(rdram, (int32_t)ctx->r29 + 0x50);
    if (logging) {
        fprintf(stderr, "krazoa: Golden Plains' test timed out, %d carryables\n", count);
    }
    if (!player) {
        return;
    }
    int32_t held = word(rdram, word(rdram, player + 0xB8) + 0x868);
    if (!held) {
        return;
    }
    for (int32_t i = 0; i < count; i++) {
        if (word(rdram, list + 4 * i) == held) {
            Calls calls(rdram, ctx);
            ctx->r4 = player;
            ctx->r5 = 0;
            call(rdram, ctx, word(rdram, word(rdram, word(rdram, player + 0x68)) + 0x30));
            if (logging) {
                fprintf(stderr, "krazoa: Golden Plains' timeout drops the carried object 0x%X\n", (unsigned)held);
            }
            return;
        }
    }
}

// us.toml's hook in Golden Plains' sequence callback (dll472 0x1134), with the event less one in t4 (event 4 is
// 0x11DC: state 4). The shrine's update doesn't run while its sequence plays: the win's sequence 1 sends event 6
// (state 5, the reward) and then event 4, so the shrine came back to state 4, played the win again and never paid out.
// While state 5 is pending, event 4 is dropped (t4 past the table, as an unknown event is).
extern "C" void dino_krazoa_gp_event(uint8_t* rdram, recomp_context* ctx) {
    if (logging) {
        fprintf(stderr, "krazoa: Golden Plains' event %d in state %d\n", (int)ctx->r12 + 1, (int)MEM_BU(0x15, ctx->r16));
    }
    if (!shrines_restored()) {
        return;
    }
    if ((int32_t)ctx->r12 == 4 - 1 && MEM_BU(0x15, ctx->r16) == 5) {
        ctx->r12 = 0xFF;
        if (logging) {
            fprintf(stderr, "krazoa: Golden Plains' win keeps its reward\n");
        }
    }
}

// us.toml's hook in CCkrazoabright's state 4 (dll457 0xD78), the last lights solved, as it stores state 5.
extern "C" void dino_krazoa_tablets_solved(uint8_t* rdram, recomp_context* ctx) {
    if (!tablets_restored()) {
        return;
    }
    Calls calls(rdram, ctx);
    set_if_clear(rdram, ctx, tablets_done);
    if (logging) {
        fprintf(stderr, "krazoa: Cape Claw's tablet room solved\n");
    }
}

// us.toml's hook in CCkrazoabright's first update (dll457 0x108), which stores state 4 from $t7 with 0x3AC set.
extern "C" void dino_krazoa_tablets_state(uint8_t* rdram, recomp_context* ctx) {
    if (!tablets_restored()) {
        return;
    }
    bool solved;
    {
        Calls calls(rdram, ctx);
        solved = get(rdram, ctx, tablets_done) != 0;
    }
    if (solved) {
        ctx->r15 = 5;
    }
}

// us.toml's hook in the tablet pickup's update (dll308 0x288), just after it set its bit (+0x1C), which it does only
// while that bit was clear. CCkrazoaBits counts a pickup only while it is loaded, by the tablet room, so the third
// tablet, picked up by the worm, is counted here as dll458 0x1C0-0x1E8 would.
extern "C" void dino_krazoa_tablet_picked(uint8_t* rdram, recomp_context* ctx) {
    if (!tablets_restored() || MEM_H(0x1C, (gpr)word(rdram, (int32_t)ctx->r29 + 0x20)) != third_tablet) {
        return;
    }
    Calls calls(rdram, ctx);
    ctx->r4 = tablets_counted;
    call(rdram, ctx, increment_bit);
    set(rdram, ctx, tablet_new, 1);
    if (logging) {
        fprintf(stderr, "krazoa: the third tablet is counted\n");
    }
}

// us.toml's hook in CCkrazoaBits' creation (dll458 0x98), as it keeps 0x237's value in $v0 to count it once set: the
// third tablet is counted at its pickup instead.
extern "C" void dino_krazoa_tablet_bits(uint8_t*, recomp_context* ctx) {
    if (tablets_restored()) {
        ctx->r2 = 1;
    }
}

// us.toml's hook in the text block's update (dll654 0x314), on the answer of the call at 0x30C (dll1 export 7, with
// 0x7BF). Once its needed bit (+0x20) is set and its done bit (+0x1E) isn't, a text block asks that every frame; on
// yes it plays its sequence (+0x19) and marks it played (state+0xC), and on the first no after that it sets its done
// bit (0x350). 0x7BF is Desert Force Point's own: its controller sets it (dll638 0xF58) and DFP's first block needs
// it (map 21 @uid 405DE +0x20), so a palace block goes off instead with the player standing at it, once: only while
// its sequence hasn't played ($s0 is its state here).
extern "C" void dino_krazoa_palace_textblock(uint8_t* rdram, recomp_context* ctx) {
    int32_t block = word(rdram, (int32_t)ctx->r29 + 0x28);
    int object = MEM_H(0, (gpr)word(rdram, block + 0x4C));
    if (object < kp_textblock_fire || object > kp_textblock_fire + 3 || !palace_restored() ||
        MEM_BU(0xC, (gpr)ctx->r16) != 0) {
        return;
    }
    int32_t player;
    {
        Calls calls(rdram, ctx);
        call(rdram, ctx, player_object);
        player = (int32_t)ctx->r2;
    }
    float dx, dz;
    int32_t px = word(rdram, player + 0xC), pz = word(rdram, player + 0x14);
    int32_t bx = word(rdram, block + 0xC), bz = word(rdram, block + 0x14);
    float pxf, pzf, bxf, bzf;
    memcpy(&pxf, &px, 4);
    memcpy(&pzf, &pz, 4);
    memcpy(&bxf, &bx, 4);
    memcpy(&bzf, &bz, 4);
    dx = pxf - bxf;
    dz = pzf - bzf;
    if (dx * dx + dz * dz < textblock_reach * textblock_reach) {
        ctx->r2 = 1;
        if (logging) {
            fprintf(stderr, "krazoa: palace text block 0x%X goes off\n", object);
        }
    }
}

namespace {
    // A palace door, found by where it was placed, and the bit that opens it. A section door shuts and opens by its
    // model's own animation, 1.0 shut and 0 open (func_80023D30(object, 0, progress, 0)). The last door is four
    // leaves of one model in an iris, each turned by its roll (+4) and moved out along `out` as it opens.
    struct PalaceDoor {
        float x, y, z;
        int bit;
        int16_t roll;     // a leaf's, from +0x1A
        float out_x, out_y, out_z;
        int32_t object; // as last seen, so a door made again starts where its bit says
        float shut;     // 1 shut, 0 open
    };

    constexpr int32_t set_animation = 0x80023D30;
    constexpr int32_t ticks = 0x8008C958; // the frame's length in ticks, a float (dll654 0x284)
    constexpr float door_ticks = 90.0f;   // a door opens in 1.5 s
    constexpr float leaf_travel = 100.0f;
    constexpr float door_sunk = 1000.0f;
    constexpr float last_door_stop = 10215.0f;

    PalaceDoor palace_doors[] = {
        { 5757.0f, 95.0f, 21440.0f, tablets_done },
        { 8638.0f, 0.0f, 24957.0f, 0x6A7 },
        { 5754.0f, 63.0f, 25280.0f, 0x6A8 },
        { 8641.0f, 127.0f, 21750.0f, 0x6A9 },
        { 10190.0f, 85.0f, 23040.0f, 0x6A7, 0, 0.0f, 1.0f, 0.0f },
        { 10190.0f, 85.0f, 23040.0f, 0x6A8, 0x4000, 0.0f, 0.0f, 1.0f },
        { 10190.0f, 85.0f, 23040.0f, 0x6A9, (int16_t)0x8000, 0.0f, -1.0f, 0.0f },
        { 10190.0f, 85.0f, 23040.0f, 0x6AA, (int16_t)0xC000, 0.0f, 0.0f, -1.0f },
    };

    float word_float(uint8_t* rdram, int32_t address) {
        int32_t w = word(rdram, address);
        float f;
        memcpy(&f, &w, 4);
        return f;
    }
}

// us.toml's hook in dll307's update (0x1E4, $a0 the door): a palace door is posed from its bit each frame.
extern "C" void dino_krazoa_palace_door(uint8_t* rdram, recomp_context* ctx) {
    int32_t door = (int32_t)ctx->r4;
    int32_t setup = word(rdram, door + 0x4C);
    int object = MEM_H(0, (gpr)setup);
    if ((object != kp_section_door && object != kp_last_door) || !palace_restored()) {
        return;
    }
    float x = word_float(rdram, setup + 0x8), y = word_float(rdram, setup + 0xC), z = word_float(rdram, setup + 0x10);
    for (PalaceDoor& d : palace_doors) {
        if (d.x != x || d.y != y || d.z != z || (object == kp_last_door && MEM_H(0x1A, (gpr)setup) != d.roll)) {
            continue;
        }
        Calls calls(rdram, ctx);
        float target = get(rdram, ctx, d.bit) ? 0.0f : 1.0f;
        if (d.object != door) {
            d.object = door;
            d.shut = target;
        }
        float step = word_float(rdram, ticks) / door_ticks;
        d.shut = target < d.shut ? std::max(target, d.shut - step) : std::min(target, d.shut + step);
        if (object == kp_section_door) {
            float shut = d.shut;
            int32_t bits;
            memcpy(&bits, &shut, 4);
            ctx->r4 = door;
            ctx->r5 = 0;
            ctx->r6 = bits;
            ctx->r7 = 0;
            call(rdram, ctx, set_animation);
            // Its collision stays where it stands whatever its pose, so an open one is put under the floor.
            float dy = d.y - (d.shut == 0.0f ? door_sunk : 0.0f);
            memcpy(&MEM_W(0x10, (gpr)door), &dy, 4);
        }
        else {
            float open = (1.0f - d.shut) * leaf_travel;
            float lx = d.x + d.out_x * open, ly = d.y + d.out_y * open, lz = d.z + d.out_z * open;
            MEM_H(4, (gpr)door) = d.roll;
            memcpy(&MEM_W(0xC, (gpr)door), &lx, 4);
            memcpy(&MEM_W(0x10, (gpr)door), &ly, 4);
            memcpy(&MEM_W(0x14, (gpr)door), &lz, 4);
            // The leaves have no collision, so a door not yet open holds the player east of it.
            if (d.shut > 0.0f) {
                call(rdram, ctx, player_object);
                int32_t player = (int32_t)ctx->r2;
                if (player != 0 && word_float(rdram, player + 0xC) < last_door_stop &&
                    word_float(rdram, player + 0xC) > d.x - leaf_travel) {
                    memcpy(&MEM_W(0xC, (gpr)player), &last_door_stop, 4);
                }
            }
        }
        return;
    }
}
