// The audio frame's command list, kept inside its buffer. us.toml's hooks call these.
//
// func_80011720 allocates two lists of 0x4000 bytes, 2048 commands, from the game's heap at boot (the size at
// 0x80011940), and func_80011C70 has func_800642F4 build each audio frame's commands into one of them in turn, with no
// test of the end anywhere. Most of a frame is voice pulls: func_80075F20, once for each bus of each chunk of the
// frame, pulls each of the bus's sources with func_8006C840, which writes up to 96 bytes and returns the new end of the
// list. The rest (each bus's mixing, the effects, the interleave and the save) came to 7664 bytes a frame at most, with
// all 32 of the sound player's states in use.
//
// With 32 voices of one sound (Cape Claw's SandEels can start their sound 0xB20 every frame and fill every state
// with it) a frame grows past 16384 bytes, and what follows the second list in the heap is DLL 1:
// the frame writes over its export table, and the next call through it (dll19's) finds no function there. So a
// source is pulled only while the list still has room for its pull and for the rest of the frame that isn't pulls;
// past that a frame leaves its last voices out, which go quiet for that part of the frame.

#include <cstdio>

#include "recomp.h"

namespace {
    constexpr uint32_t list_size = 0x4000;
    // The most one pull and the rest of a frame were seen to write, with room to spare.
    constexpr uint32_t pull_room = 0x100;
    constexpr uint32_t rest_room = 0x2000;

    uint32_t list_start = 0;
    // The bytes this frame's pulls have written so far, and the end of the list as the pull being made found it.
    uint32_t pulled = 0;
    uint32_t pull_start = 0;
    bool frame_short = false;
    uint32_t frames_short = 0;
}

// func_800642F4, starting a frame: $a0 is the list.
extern "C" void dino_audio_frame(uint32_t list) {
    list_start = list;
    pulled = 0;
    frame_short = false;
}

// func_8006C840, about to pull a source: whether to leave it out, with $a2 the list's end. Left out, the pull returns
// the end as it was, as for a source with nothing to play.
extern "C" int dino_audio_pull_skipped(uint32_t end) {
    uint32_t used = end - list_start;
    // The rest of the frame not written yet: what isn't pulls so far, out of rest_room.
    uint32_t rest = used - pulled < rest_room ? rest_room - (used - pulled) : 0;
    // A source left out still reaches dino_audio_pulled, with the end as it was: it adds nothing.
    pull_start = end;
    if (used + pull_room + rest <= list_size) {
        return false;
    }
    if (!frame_short && frames_short++ % 300 == 0) {
        fprintf(stderr, "Dinosaur Planet: an audio frame's command list is full, so it leaves voices out (%u frames)\n",
            frames_short);
    }
    frame_short = true;
    return true;
}

// func_80075F20, after func_8006C840 returned $v0, the list's new end.
extern "C" void dino_audio_pulled(uint32_t end) {
    pulled += end - pull_start;
}
