// DINO_CHECK_HEAP set (development): each pass of the main thread's loop, every block in use of the game's heaps is
// checked for a write past its end, and the first time one is found it is printed with what holds it. For memory that
// something writes over, before --watch (src/main/crash_trace.cpp) can say who.
//
// The game's allocator, func_80016AA8 (size, tag, name), takes a block from one of three heaps, each 0x14 bytes from
// 0x800B09D0: +0 how many blocks its table holds, +8 the table. A block's entry is 0x14 bytes: +0 its address, +4 its
// size, +8 not 0 while in use, +0xC the next entry (-1 ends the heap), +0x10 the tag its caller passed. func_80016EE0
// adds 8 bytes to each block it gives and writes -1 to both words of them (the block's last 8 bytes), which nothing of
// the game's touches after: a write past the block's end changes them.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "recomp.h"

#include "dino.h"

namespace {
    constexpr int32_t heaps = (int32_t)0x800B09D0;
    constexpr int heap_count = 3;
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
