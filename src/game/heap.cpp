// DINO_CHECK_HEAP set (development): each pass of the main thread's loop, every block in use of the game's heaps is
// checked for a write past its end, and the first time one is found it is printed with what holds it. For memory that
// something writes over, before --watch (src/main/crash_trace.cpp) can say who.
//
// The game's allocator, func_80016AA8 (size, tag, name), takes a block from one of three heaps, each 0x14 bytes from
// 0x800B09D0: +0 how many blocks its table holds, +8 the table. A block's entry is 0x14 bytes: +0 its address, +4 its
// size, +8 not 0 while in use, +0xC the next entry (-1 ends the heap), +0x10 the tag its caller passed. func_80016EE0
// adds 8 bytes to each block it gives and writes -1 to both words of them (the block's last 8 bytes), which nothing of
// the game's touches after: a write past the block's end changes them.
//
// The heap's late frees, always on: with D_800B179C not 0 (2, set by func_80017254 while a frame may still draw from
// memory) func_800172E8 hands what it frees to func_800175D4, which adds it, with that 2, to the 8-byte entries from
// 0x800B0A78 that D_800B1798 counts. func_80017344 counts each entry's byte down once a frame and frees it at 0, and
// func_80017254(0) frees them all. The entries end at their own count, so they hold 420, and func_800175D4 tests
// nothing: a frame that frees more wrote its 421st and 422nd over the loaded models' table at 0x800B17A4 and its count
// at 0x800B17AC, and the next model freed searched a table of garbage. Dragon Rock - Bottom's arena unloading after
// the Earth Warrior is freed frees more. What doesn't fit waits here, counted down and freed as the game's own.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <vector>

#include "recomp.h"

#include "dino.h"

namespace {
    constexpr int32_t heaps = (int32_t)0x800B09D0;
    constexpr int heap_count = 3;

    constexpr int32_t late_frees_count = (int32_t)0x800B1798;
    constexpr int32_t late_frees_mode = (int32_t)0x800B179C;
    constexpr int late_frees_room = 420;
    constexpr int32_t free_now = (int32_t)0x8001753C;

    struct LateFree {
        uint32_t address;
        int frames;
    };
    std::mutex late_frees_lock;
    std::vector<LateFree> late_frees;

    void free_all(uint8_t* rdram, recomp_context* ctx, const std::vector<uint32_t>& addresses) {
        for (uint32_t address : addresses) {
            recomp_context c = *ctx;
            c.r29 -= 0x40;
            c.r4 = (gpr)(int32_t)address;
            LOOKUP_FUNC(free_now)(rdram, &c);
        }
    }
}

// func_800175D4, about to add $a0 to the late frees: with no room left, it waits here, and the game's is skipped.
extern "C" int dino_late_free(uint8_t* rdram, recomp_context* ctx) {
    if (MEM_H(0, (gpr)late_frees_count) < late_frees_room) {
        return 0;
    }
    std::lock_guard<std::mutex> guard(late_frees_lock);
    late_frees.push_back({ (uint32_t)ctx->r4, (int)(uint8_t)MEM_W(0, (gpr)late_frees_mode) });
    return 1;
}

// func_80017344, once a frame: each waiting here counted down as the game's are, and freed at 0.
extern "C" void dino_late_frees_counted(uint8_t* rdram, recomp_context* ctx) {
    std::vector<uint32_t> due;
    {
        std::lock_guard<std::mutex> guard(late_frees_lock);
        for (auto it = late_frees.begin(); it != late_frees.end();) {
            if (--it->frames <= 0) {
                due.push_back(it->address);
                it = late_frees.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    free_all(rdram, ctx, due);
}

// func_80017254($a0): with 0, the game frees all of its late frees, and these with them.
extern "C" void dino_late_frees_mode(uint8_t* rdram, recomp_context* ctx) {
    if (ctx->r4 != 0) {
        return;
    }
    std::vector<uint32_t> due;
    {
        std::lock_guard<std::mutex> guard(late_frees_lock);
        for (const LateFree& late : late_frees) {
            due.push_back(late.address);
        }
        late_frees.clear();
    }
    free_all(rdram, ctx, due);
}

void dino::heap_check(uint8_t* rdram, double seconds) {
    static std::vector<uint32_t> reported;
    for (int heap = 0; heap < heap_count; heap++) {
        gpr desc = (gpr)(heaps + heap * 0x14);
        gpr table = (gpr)(int32_t)MEM_W(8, desc);
        int32_t max = MEM_W(0, desc);
        if (table == 0 || max <= 0 || max > 0x2000) {
            continue;
        }
        int32_t index = 0;
        int32_t prev = -1;
        for (int32_t steps = 0; index >= 0 && index < max && steps < max; steps++) {
            gpr entry = table + index * 0x14;
            uint32_t addr = MEM_W(0, entry);
            uint32_t size = MEM_W(4, entry);
            if (MEM_H(8, entry) != 0 && size >= 8 && size < 0x800000 && (addr >> 24) == 0x80) {
                gpr guard = (gpr)(int32_t)(addr + size - 8);
                bool broken = MEM_W(0, guard) != 0xFFFFFFFF || MEM_W(4, guard) != 0xFFFFFFFF;
                if (broken && std::find(reported.begin(), reported.end(), addr) == reported.end()) {
                    reported.push_back(addr);
                    fprintf(stderr, "heap: %.2f s, heap %d's block at 0x%08X (size 0x%X, tag 0x%08X) is written past its end:", seconds, heap, addr, size - 8, MEM_W(0x10, entry));
                    for (int offset = -0x20; offset < 0x20; offset += 4) {
                        fprintf(stderr, "%s%08X", offset == 0 ? " | " : " ", MEM_W(offset, guard));
                    }
                    if (prev >= 0) {
                        gpr before = table + prev * 0x14;
                        fprintf(stderr, " (the block before: 0x%08X, size 0x%X, tag 0x%08X)", MEM_W(0, before), MEM_W(4, before), MEM_W(0x10, before));
                    }
                    fprintf(stderr, "\n");
                }
            }
            prev = index;
            index = MEM_H(0xC, entry);
        }
    }
}
