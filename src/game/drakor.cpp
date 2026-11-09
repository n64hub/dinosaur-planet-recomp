// BOSS Drakor (map 44), the prototype's last fight, and the game's end. The fight is won in the ROM as it is: a hit
// lets the Kamerian Diamond go into the room under the fight, and the player who reaches it there plays Drakor's fall
// (sequence 1). Nothing follows it: the fight starts again, and the game never ends. With Ending on Restored (the
// default) the end of his fall plays the ROM's own credits, which nothing opens, then goes back to the title; on
// Original it plays as the console does.
//
// The fight, read from the ROM (offsets are a DLL's text, `@` a placement in map 44's object list):
// - BossDrakorLevel (dll692, @0080) holds the fight's objects: the player, Drakor (0x2AB, dll697), the Diamond (0x376,
//   dll695), the flat rock the player stands on (0x4DC, dll696).
// - Drakor's hit handler (dll697 0x0838) asks func_80025F40 for a hit, leaves out the asteroids' (0x4DB) and the flat
//   rock's (0x4DC), and sends him to his second state, telling the level (0x08E0 with 1, its export 0x2F8): 0x7C6,
//   which plays sequence 2, and the Diamond let go (+0x8 0x20). The player's staff is such a hit (played: object 0x1F).
//   His health (+0x348, 24) is never lowered, and none is needed: one hit lets go.
// - Sequence 2 takes the player, Drakor and the Diamond down into the room under the fight (y -960 against the rock's
//   -18), where the Diamond comes to rest. The level keeps the Diamond with Drakor only while it isn't let go (0x0E34);
//   let go, it sets 0x7AE once the player is within 50 units of it on the ground plane. Played: the player lands about
//   790 units from it and walks to it in some 20 s.
// - Drakor's second state runs 13 steps (dll697 0x0C54, 0x0CB0-0x0CC4), then gives the Diamond back (0x08E0 with 0):
//   0x7C5, sequence 3, and the fight goes on in the sky. Played: 43 s after the hit, which is the player's time to reach
//   the Diamond. His steps wait while sequence 1 plays (played: 0x7C5 only after it).
// - Four BossDrakor_sequ (dll309, @0128 to @01A0) each play a sequence once their trigger bit is set (placement +0x1A,
//   the sequence +0x1E): 0x7AC 0 (the intro), 0x7AE 1 (@0150), 0x7C6 2, 0x7C5 3. Sequence 1 is Drakor's fall: Krystal
//   falls into the room, Drakor lands, roars and collapses on the platform, about 33 s. Its event callback (0x047C)
//   warps only on an event 2, to placement +0x24, which is 0 in all four, and sequence 1 sends no event (played). It
//   ends with the player dropped to the floor and 4 health lost, and once Drakor's second state runs out, 0x7C5 and
//   the fight again.
// - Nothing in the ROM follows it: no map, warp, text or credits. Drakor's only lines are before the fight (ROM file
//   19). The ROM's way back to the title is dll67's, the menu that 0x8008C840's menu table gives menu 9 (0x02F8-0x0324):
//   func_80013FB4, which resets the video mode and the play flags and loads Front End2 (func_800141A4 with map 1), then
//   func_8000F404 with menu 2, the title (dll60). func_80013FB4 alone, as BOSS TRex's defeat in the prototype calls it
//   (dll702 0x2DF4), leaves the play menu up over Front End2, and the screen black.
// - The credits are dll78, the menu table's menu 15, and nothing calls set_menu(15) (EndObject, dll299, placed in map
//   4, is empty). Its constructor loads texture 0xC5 (func_8003CD7C) and text 0x1FD (dll21.4); its update (export 0,
//   0x00D4) adds each frame's time to a clock (D_0F20) and moves its page (D_0F18) on as the clock passes the page's
//   end (a page is 0x98 bytes of a table at D_0924, its end at +0x92), each line fading in and out by four times of
//   its own; its draw (export 1, 0x0570) shows page 0 as the texture and pages 1 to 9 as text 0x1FD's lines, white
//   on a black shadow, over whatever is drawn. Past page 9 it draws nothing and stays, so it ends nowhere: nothing in
//   it opens another menu.
//
// What Restored changes:
// - The end of sequence 1: a sequence object's callback runs each frame its sequence plays and marks the object's state
//   (bit 4 of the byte at +0xB8, which its update clears), so updates with no mark after marked ones are the sequence
//   ended. Then the game goes to the front end as dll67 does, but with menu 15, the credits, over it in place of the
//   title: over Front End2 with no menu of its own they show on black, and the fight doesn't go on behind them. The
//   save is left as it was: Game Select plays from the last save, before the fight.
// - The end of the credits: dll78's update, as it reads its page, sees the page past the last, and opens the title.
//   Only Restored opens the credits, so it needs no check of its own.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
#include "dino.h"

namespace {
    const std::string ending = "ending";

    enum class Ending {
        Original,
        Restored,
        OptionCount
    };
    const std::vector<recomp::config::ConfigOptionEnumOption> ending_options = {
        { Ending::Original, "Original" },
        { Ending::Restored, "Restored" },
    };

    bool restored() {
        return std::get<uint32_t>(recompui::config::get_general_config().get_option_value(ending)) ==
            (uint32_t)Ending::Restored;
    }

    constexpr int32_t to_front_end = 0x80013FB4;
    constexpr int32_t set_menu = 0x8000F404;
    constexpr int title_menu = 2;
    constexpr int credits_menu = 15;
    constexpr int credits_pages = 10;
    constexpr int diamond_reached = 0x7AE;
    constexpr int quiet_updates = 15; // updates with no mark before sequence 1 counts as ended

    // DINO_LOG_DRAKOR set (development): the fall's end is printed.
    const bool logging = std::getenv("DINO_LOG_DRAKOR") != nullptr;

    gpr fall_object = 0; // sequence 1's object, once its sequence has played a frame
    int quiet = 0;

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

    void open_menu(uint8_t* rdram, recomp_context* ctx, int menu) {
        Calls calls(ctx);
        ctx->r4 = menu;
        call(rdram, ctx, set_menu);
    }

    // The ROM's way back to the title, as dll67 0x02F8-0x0324, with the credits in place of the title.
    void go_to_credits(uint8_t* rdram, recomp_context* ctx) {
        {
            Calls calls(ctx);
            call(rdram, ctx, to_front_end);
        }
        open_menu(rdram, ctx, credits_menu);
    }
}

void dino::add_drakor_options(recomp::config::Config& general) {
    general.add_enum_option(
        ending,
        "Ending",
        "Once Drakor is beaten and his fall has played, the game's own credits play, then the title. <b>Original is "
        "the prototype as it was left</b>: after his fall the fight starts again, and the game never ends.",
        ending_options,
        Ending::Restored
    );
}

// us.toml's hook at the start of a BossDrakor_sequ's update (dll309 0x0124): a0 the object. Only sequence 1's object,
// the one 0x7AE triggers, is watched.
extern "C" void dino_drakor_sequence(uint8_t* rdram, recomp_context* ctx) {
    gpr object = ctx->r4;
    gpr placement = (gpr)MEM_W(0x4C, object);
    if (placement == 0 || MEM_HU(0x1A, placement) != diamond_reached || !restored()) {
        return;
    }
    if (MEM_BU(0, (gpr)MEM_W(0xB8, object)) & 4) {
        fall_object = object;
        quiet = 0;
        return;
    }
    if (fall_object != object || ++quiet < quiet_updates) {
        return;
    }
    fall_object = 0;
    if (logging) {
        fprintf(stderr, "drakor: his fall has played, the credits\n");
    }
    go_to_credits(rdram, ctx);
}

// us.toml's hook in the credits' update (dll78 0x0104), once it has read its page into v0: past the last, the title.
extern "C" void dino_credits_page(uint8_t* rdram, recomp_context* ctx) {
    if ((int32_t)ctx->r2 < credits_pages) {
        return;
    }
    if (logging) {
        fprintf(stderr, "drakor: the credits have played, the title\n");
    }
    open_menu(rdram, ctx, title_menu);
}
