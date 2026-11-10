// The natives the patches call (patches/syms.ld gives each a dummy address that N64Recomp turns into a call by name),
// of which there are none yet, and the functions the executable gives mods: a mod imports them from "*"
// (RECOMP_IMPORT("*", ...), mod_template/include/dino_api.h), beside the runtime's own (recomp_alloc, the mod config and
// UI calls) and the patches library's exports.

#include <bit>
#include <cstdio>
#include <cstring>
#include <string>

#include "recomp.h"
#include "librecomp/helpers.hpp"
#include "librecomp/overlays.hpp"

#include "dino.h"

namespace {

template <typename T>
void append_formatted(std::string& out, const std::string& spec, T value) {
    int len = snprintf(nullptr, 0, spec.c_str(), value);
    if (len <= 0) {
        return;
    }
    size_t start = out.size();
    out.resize(start + len + 1);
    snprintf(out.data() + start, len + 1, spec.c_str(), value);
    out.resize(start + len);
}

} // namespace

// int recomp_printf(const char* fmt, ...): printf to the console (stdout). A mod calls it as MIPS code calls any
// variadic function (o32): its arguments are 4-byte slots, the first four in $a0-$a3 and the rest at 16($sp) on, and
// an 8-byte one (a double, a long long) starts at an even slot, its high word first. Each conversion is formatted with
// the host's snprintf.
extern "C" void recomp_printf(uint8_t* rdram, recomp_context* ctx) {
    gpr fmt = ctx->r4;
    unsigned slot = 1;
    auto word = [&](unsigned n) -> uint32_t {
        return n < 4 ? (uint32_t)(&ctx->r4)[n] : (uint32_t)MEM_W(n * 4, ctx->r29);
    };
    auto next_word = [&]() -> uint32_t { return word(slot++); };
    auto next_dword = [&]() -> uint64_t {
        slot = (slot + 1) & ~1u;
        uint64_t hi = word(slot);
        uint64_t lo = word(slot + 1);
        slot += 2;
        return (hi << 32) | lo;
    };
    auto at = [&](size_t i) -> char { return (char)MEM_B(i, fmt); };

    std::string out;
    size_t i = 0;
    char c;
    while ((c = at(i++)) != '\0') {
        if (c != '%') {
            out += c;
            continue;
        }
        std::string spec = "%";
        while ((c = at(i)) != '\0' && strchr("-+ #0", c)) {
            spec += c;
            i++;
        }
        if (at(i) == '*') {
            spec += std::to_string((int32_t)next_word());
            i++;
        }
        while ((c = at(i)) >= '0' && c <= '9') {
            spec += c;
            i++;
        }
        if (at(i) == '.') {
            spec += '.';
            i++;
            if (at(i) == '*') {
                spec += std::to_string((int32_t)next_word());
                i++;
            }
            while ((c = at(i)) >= '0' && c <= '9') {
                spec += c;
                i++;
            }
        }
        // Length: on o32 only ll and j are 64-bit. h and hh go to the host as they are; the rest are 32-bit.
        std::string length;
        bool wide = false;
        while ((c = at(i)) != '\0' && strchr("hljztL", c)) {
            bool ll = c == 'l' && at(i + 1) == 'l';
            if (c == 'h') {
                length += c;
            }
            wide |= ll || c == 'j';
            i += ll ? 2 : 1;
        }
        c = at(i);
        if (c == '\0') {
            break;
        }
        i++;
        switch (c) {
            case 'd': case 'i':
                if (wide) {
                    append_formatted(out, spec + "ll" + c, (long long)next_dword());
                } else {
                    append_formatted(out, spec + length + c, (int)next_word());
                }
                break;
            case 'u': case 'o': case 'x': case 'X':
                if (wide) {
                    append_formatted(out, spec + "ll" + c, (unsigned long long)next_dword());
                } else {
                    append_formatted(out, spec + length + c, (unsigned int)next_word());
                }
                break;
            case 'c':
                append_formatted(out, spec + c, (int)next_word());
                break;
            case 'p':
                append_formatted(out, std::string("0x%08X"), (unsigned int)next_word());
                break;
            case 's': {
                gpr str = (int32_t)next_word();
                std::string s;
                if (str == 0) {
                    s = "(null)";
                } else {
                    for (size_t j = 0; (c = (char)MEM_B(j, str)) != '\0'; j++) {
                        s += c;
                    }
                }
                append_formatted(out, spec + 's', s.c_str());
                break;
            }
            case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A':
                append_formatted(out, spec + c, std::bit_cast<double>(next_dword()));
                break;
            case 'n':
                next_word();
                break;
            case '%':
                out += '%';
                break;
            default:
                out += spec;
                out += c;
                break;
        }
    }

    fwrite(out.data(), 1, out.size(), stdout);
    fflush(stdout);
    _return(ctx, (int)out.size());
}

void dino::register_mod_exports() {
    recomp::overlays::register_base_export("recomp_printf", recomp_printf);
}
