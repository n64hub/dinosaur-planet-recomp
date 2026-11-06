"""Writes syms/dinoplanet.syms.toml, N64Recomp's symbols for Dinosaur Planet (the December 2000 prototype), from the ROM
alone.

There is no symbol map, and nothing here comes from any decompilation: every function is found in the ROM's code.

- The main segment is stored raw at ROM 0x1000 and runs at 0x80000400, where the IPL3 copies the first MB of ROM: its
  CPU code ends at 0x80089750, where the RSP's microcodes start (the audio one there, aspMain.us.toml; rspboot at
  0x8008B090 and the graphics one at 0x8008B160 after it), and its data runs on to the bss at 0x800A3D70.
- The rest of the game's code is 796 DLLs, which the game loads and unloads as it plays (DLLS below). The game reads
  each one whole into memory it allocates (func_8000C3CC) and relocates it there itself (func_8000C4A8), so a DLL is a
  relocatable section of its own here, given a made-up address (DLL_VRAM) that the runtime moves to where the game put
  it (src/game/dlls.cpp).
- spimdisasm finds the functions in each text (jal targets, the code after a function's last jr ra, hi/lo pairs that
  make a text address), and a function it splits at a label inside one, which only that function's own branches reach,
  is joined again (merge_split_functions).
- A function reached only through a pointer must be a function to N64Recomp too, or the runtime can't find it when
  the game calls the pointer. In the main segment code makes some of those pointers (a lui then an addiu to a text
  address) and the rest are words in its data, of which a run that all point inside one function is that function's
  jump table and left alone (data_pointers). The DLLs' imports (the table in ROM file 72, DLL_IMPORTS) are main
  segment addresses the DLLs call, so each one in its text starts a function. In a DLL, the starts are its exports,
  its constructor and destructor, each $gp setup (below), and the code addresses its GOT gives: a word the code uses
  as it is, and a page plus a low half (Dll.got_uses).
- libultra and libc are named by n64sym's signatures (ref/n64sym, MIT: git clone https://github.com/shygoo/n64sym
  ref/n64sym): a function matches when the CRC32 of its first 8 bytes and of its whole body agree once relocated
  fields are masked out. N64Recomp has the runtime replace libultra functions by name, so those it reimplements must
  carry their real names; MANUAL_NAMES has the ones the signatures can't tell.

A DLL, as the game's loader reads it (func_8000C3CC and func_8000C4A8):

- The table in ROM file 71 gives each DLL, by its number, 8 bytes at (number + 1) * 8: its offset in file 70, which
  holds them all, and the size of its bss; the next one's offset ends it.
- Its header: the offset of its text (+0x0), of its data (+0x4) and of its relocations (+0x8), -1 for none; the number
  of exports (+0xC, a halfword); its constructor and destructor (+0x10, +0x14, 0 for none); and from +0x1C the exports.
  Every one of those is an offset in its text.
- Its text is position-independent code that reaches everything through its GOT: no jal, no absolute address. The
  relocations start with the GOT, a word each until -2: an offset in the text, or with the top bit set an index (from
  1) into the import table. Then the offsets in the text of the lui/ori pairs that set $gp to the GOT, one at the
  start of each function that uses it, until -3. Then offsets in its data of words that point into it, until -1.
- Its jump tables are words in its rodata, each the distance from the GOT to a case, which N64Recomp reads through
  the section's got_address.

The $gp setup is a lui and an ori: an ori takes the low half as it is, where the HI16 relocation N64Recomp emits for
the lui rounds the high half up for an addiu. runtime-patches/N64Recomp-1-ori-lo16-adds-signed.patch has N64Recomp add
an ori's LO16 signed, as an addiu, so the pair gives the GOT's address.

Everything is func_<address>, a DLL's functions dll<number>_<offset in its text>. Names worked out since go into
syms/names.toml ([[func]] section, vram, name), applied last, and keep their names for good (mods and hooks use them).

Usage: py -3 -I tools/gen_syms.py [--report]   (needs roms/dinoplanet.z64, ref/n64sym and spimdisasm)
"""
import bisect
import csv
import hashlib
import struct
import subprocess
import sys
import tempfile
import tomllib
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ROM_PATH = ROOT / "roms" / "dinoplanet.z64"
SIG_PATH = ROOT / "ref" / "n64sym" / "src" / "builtin_signatures.sig"
NAMES_PATH = ROOT / "syms" / "names.toml"
OUT_PATH = ROOT / "syms" / "dinoplanet.syms.toml"
ROM_MD5 = "49f7bb346ade39d1915c22e090ffd748"

# The main segment: ROM offset, VRAM, size of the text, size of the text and data.
MAIN = ("main", 0x1000, 0x80000400, 0x80089750 - 0x80000400, 0x800A3D70 - 0x80000400)

# The ROM's file table (func_80037090): a count, then the files' offsets from FILES_BASE, each ended by the next.
FILE_TABLE = 0xA4970
FILES_BASE = 0xA4AA0
DLL_FILE, DLL_TABLE_FILE, DLL_IMPORTS_FILE = 70, 71, 72

# Where the DLLs' sections are put: each at this plus its offset in file 70 and its text's offset, so no two overlap
# and none is anywhere the game's memory is.
DLL_VRAM = 0x90000000

# libultra functions the signatures can't name, by hand from what they do. N64Recomp drops or reimplements libultra
# functions by name.
MANUAL_NAMES = {
    # Controllers: the only caller of __osContGetInitData besides osContGetQuery, which also makes the SI access queue;
    # and the copy out of __osContPifRam right after it, which the game's pad code calls after each read.
    0x8007D530: "osContInit",
    0x8007D8F4: "osContGetReadData",
    # Audio: the one caller of __osAiDeviceBusy, which writes the AI's DMA registers; and the function before it that
    # divides osViClock by the rate and writes the AI's DAC rate and bit rate.
    0x8007DF00: "osAiSetNextBuffer",
    0x8007DC70: "osAiSetFrequency",
    # PI: the two that send a request to the PI manager, osJamMesg for a high priority one, osSendMesg otherwise; and the
    # read of a word through a PI handle (__osPiGetAccess, then __osEPiRawReadIo).
    0x8007DFE0: "osPiStartDma",
    0x80080F70: "osEPiStartDma",
    0x80080EA0: "osEPiReadIo",
    # VI: the one caller of __osViInit, which starts the VI manager's thread; and the read of the next framebuffer
    # between __osDisableInt and __osRestoreInt.
    0x8005E200: "osCreateViManager",
    0x8007F080: "osViGetNextFramebuffer",
    # The exception handler: it reads every RCP interrupt's register (MI, VI, AI, PI, SI), wakes threads with
    # __osEnqueueThread and calls kdebugserver. The runtime has none, and N64Recomp leaves it out by its name.
    0x80080090: "__osException",
    # Its message sending after it (and handle_CpU, which spimdisasm keeps in the same function), which branch back
    # into it: left out with it.
    0x80080A34: "send_mesg",
}

# Functions in a DLL named by hand, by DLL number and offset in its text. dll31 is the game's save, on Flash, and holds
# its own copy of libultra's Flash functions, which drive the chip through osEPiWriteIo, osEPiReadIo and osEPiStartDma:
# named, N64Recomp has the runtime's Flash in their place. Each is told by the commands it writes to the chip's command
# register (the PI handle at 0xA8000000, plus 0x10000): osFlashInit links that handle and checks the chip's ID against
# the Macronix ones (0x00C2001E, 0x00C20001), the status reads write 0xD2 and the ID read 0xE1, a sector erase 0x4B then
# 0x78, the page buffer's write 0xB4 before its DMA, the program 0xA5, and the read array 0xF0 before its DMA. The main
# segment has the same functions from 0x8005EAE8 on, which nothing calls or imports.
DLL_MANUAL_NAMES = {
    (31, 0x04DC): "osFlashInit",
    (31, 0x0640): "osFlashReadStatus",
    (31, 0x070C): "osFlashReadId",
    (31, 0x0820): "osFlashClearStatus",
    (31, 0x088C): "osFlashSectorErase",
    (31, 0x09A4): "osFlashWriteBuffer",
    (31, 0x0A50): "osFlashWriteArray",
    (31, 0x0B48): "osFlashReadArray",
}

# Function starts the analysis misses: where the entry point jumps once bss is clear (jr $t2).
EXTRA_FUNCTION_STARTS = {0x80000450}

# Hand-written assembly that calls parts of itself as subroutines (a jal to a label inside it, which returns by the $ra
# it saved in another register) and jumps from one part into another's: one function to N64Recomp, which recompiles
# those calls (runtime-patches/N64Recomp-2-local-calls.patch). The model code's skinning routine, from its entry to the
# next function.
ONE_FUNCTION = [(0x8001B4F0, 0x8001C8D4)]

# Signature names n64sym gives unrelated tiny functions.
UNTRUSTED_NAMES = {"n_alSynRemovePlayer", "osProfileFlush", "n_alSeqpDelete", "alSynFreeFX", "ptstart", "__dummy",
                   "osSyncPrintf", "__osGetId", "osScGetCmdQ"}


def word(buf, off):
    return struct.unpack_from(">I", buf, off)[0]


def rom_file(rom, index):
    """The ROM offset and size of file index."""
    start = word(rom, FILE_TABLE + 4 + index * 4)
    return FILES_BASE + start, word(rom, FILE_TABLE + 8 + index * 4) - start


class Dll:
    """One DLL as the game's loader reads it (the module's header says how)."""

    def __init__(self, rom, number, rom_addr, size):
        self.number, self.rom_addr, self.size = number, rom_addr, size
        h = lambda o: word(rom, rom_addr + o)
        self.text, self.data, self.rel = h(0), h(4), h(8)
        self.exports = [h(0x1C + 4 * i) for i in range(struct.unpack_from(">H", rom, rom_addr + 0xC)[0])]
        self.ctors = [x for x in (h(0x10), h(0x14)) if x]
        self.got, self.gp_sets = [], []
        if self.rel != 0xFFFFFFFF:
            p = rom_addr + self.rel
            while word(rom, p) != 0xFFFFFFFE:
                self.got.append(word(rom, p))
                p += 4
            p += 4
            while word(rom, p) != 0xFFFFFFFD:
                self.gp_sets.append(word(rom, p))
                p += 4
        # The text runs to the relocations, or to the data, or to the end.
        end = next(x for x in (self.rel, self.data, size) if x != 0xFFFFFFFF)
        self.text_size = end - self.text
        self.vram = DLL_VRAM + (rom_addr - rom_file(rom, DLL_FILE)[0]) + self.text
        self.text_rom = rom_addr + self.text
        self.text_bytes = rom[self.text_rom:self.text_rom + self.text_size]

    def got_vram(self):
        return self.vram + self.rel - self.text

    def got_uses(self):
        """The GOT's words its code uses as pages, and the addresses it makes from them. The code reaches its own
        functions and data as position-independent code does: a lw from $gp loads a GOT word, which is the address
        itself, or, for a local one, a page (its address rounded to 0x10000) that an addiu then adds the low half to.
        So a word an addiu adds to is no function of its own, where the page plus the low half may be."""
        pages, made = set(), set()
        n = len(self.text_bytes) // 4
        for i in range(n):
            w = word(self.text_bytes, i * 4)
            if w >> 26 != 0x23 or (w >> 21) & 31 != 28 or w & 0x8000 or (w & 0xFFFF) // 4 >= len(self.got):
                continue
            reg, index = (w >> 16) & 31, (w & 0xFFFF) // 4
            if self.got[index] & 0x80000000:
                continue
            for j in range(i + 1, min(i + 8, n)):
                x = word(self.text_bytes, j * 4)
                op, rs, rt, rd = x >> 26, (x >> 21) & 31, (x >> 16) & 31, (x >> 11) & 31
                if op == 0x09 and rs == reg:
                    pages.add(index)
                    made.add(self.got[index] + ((x & 0xFFFF) ^ 0x8000) - 0x8000)
                    break
                # Anything else that writes the register ends the search (stores and branches write none).
                if (op == 0 and rd == reg) or (op not in (0, 1, 2, 4, 5, 6, 7, 0x14, 0x15, 0x16, 0x17)
                                               and op < 0x28 and rt == reg):
                    break
        return pages, made

    def pointed(self):
        """The offsets in its text that something outside its code points at."""
        out = set(self.exports) | set(self.ctors) | set(self.gp_sets)
        pages, made = self.got_uses()
        out |= {w for i, w in enumerate(self.got) if i not in pages and not w & 0x80000000}
        out |= made
        return {o for o in out if o < self.text_size and o % 4 == 0}


def dlls(rom):
    bin_rom, _ = rom_file(rom, DLL_FILE)
    tab_rom, tab_size = rom_file(rom, DLL_TABLE_FILE)
    out = []
    number = 1
    while (number + 2) * 8 <= tab_size:
        start, next_start = word(rom, tab_rom + (number + 1) * 8), word(rom, tab_rom + (number + 2) * 8)
        if next_start == 0xFFFFFFFF:
            break
        if next_start > start:
            out.append(Dll(rom, number, bin_rom + start, next_start - start))
        number += 1
    return out


def dll_imports(rom):
    r, size = rom_file(rom, DLL_IMPORTS_FILE)
    return [word(rom, r + o) for o in range(0, size, 4)]


def load_sigs():
    sigs, cur = [], None
    for line in SIG_PATH.read_text().splitlines():
        p = line.split()
        if not p or p[0].startswith("#"):
            continue
        if p[0].startswith("."):
            cur["relocs"] += [(int(o, 16), p[0], p[1]) for o in p[2:]]
        else:
            cur = {"name": p[0], "size": int(p[1], 16), "crc8": int(p[2], 16), "crc": int(p[3], 16), "relocs": []}
            sigs.append(cur)
    return [s for s in sigs if s["size"] >= 12]


def masked(buf, off, sig):
    data = bytearray(buf[off:off + sig["size"]])
    if len(data) < sig["size"]:
        return None
    for o, kind, _ in sig["relocs"]:
        w = struct.unpack_from(">I", data, o)[0]
        w &= 0xFFFF0000 if kind in (".hi16", ".lo16") else 0xFC000000
        struct.pack_into(">I", data, o, w)
    return bytes(data)


def match_library(buf, vram, func_addrs, sigs):
    """{vram: name} for the functions whose code matches a signature (the largest match wins)."""
    by_crc8 = {}
    for s in sigs:
        by_crc8.setdefault(s["crc8"], []).append(s)
    names = {}
    for addr in func_addrs:
        off = addr - vram
        hits = []
        for s in by_crc8.get(zlib.crc32(masked(buf, off, {"size": 8, "relocs": []}) or b""), []):
            d = masked(buf, off, s)
            if d is not None and zlib.crc32(d[:8]) == s["crc8"] and zlib.crc32(d) == s["crc"]:
                hits.append(s)
        if not hits:
            continue
        size = max(s["size"] for s in hits)
        best = [s for s in hits if s["size"] == size]
        if len({s["name"] for s in best}) > 1:
            print(f"  ambiguous at {addr:08X}: {sorted({s['name'] for s in best})}")
            continue
        names[addr] = best[0]["name"]
        # The jal targets in a matched function are named by its relocations too.
        for o, kind, target in best[0]["relocs"]:
            if kind == ".targ26" and "_text_" not in target:
                names.setdefault(((word(buf, off + o) & 0x3FFFFFF) << 2) | 0x80000000, target)
    return names


def spimdisasm_functions(buf, vram, tmp):
    binpath = Path(tmp) / f"{vram:08X}.bin"
    binpath.write_bytes(buf)
    info = Path(tmp) / f"{vram:08X}.csv"
    subprocess.run([sys.executable, "-m", "spimdisasm", "singleFileDisasm", str(binpath), str(Path(tmp) / "asm"),
                    "--start", "0", "--end", hex(len(buf)), "--vram", hex(vram), "--function-info", str(info)],
                   check=True, capture_output=True)
    with info.open() as f:
        return sorted(int(r["address"], 16) for r in csv.DictReader(f))


def branch_targets(buf, vram, start, end):
    """The targets of the PC-relative branches and j instructions in [start, end)."""
    out = set()
    for a in range(start, end, 4):
        w = word(buf, a - vram)
        op = w >> 26
        if op in (4, 5, 6, 7, 20, 21, 22, 23) or (op == 1 and ((w >> 16) & 0x1F) in (0, 1, 2, 3)) or \
                (op == 0x11 and ((w >> 21) & 0x1F) == 8):  # beq/bne/blez/bgtz (and likely), regimm, bc1x
            out.add(a + 4 + (((w & 0xFFFF) ^ 0x8000) - 0x8000) * 4)
        elif op == 2:  # j
            out.add(((a + 4) & 0xF0000000) | ((w & 0x3FFFFFF) << 2))
    return out


def in_main_text(addr):
    _, _, v, t, _ = MAIN
    return v <= addr < v + t


def code_pointers(rom):
    """{target: [where]} for each jal target and each text address the main segment makes with a lui and an addiu."""
    out = {}
    _, r, v, t, _ = MAIN
    his = {}
    for o in range(0, t, 4):
        w = word(rom, r + o)
        op, rs, rt, imm = w >> 26, (w >> 21) & 31, (w >> 16) & 31, w & 0xFFFF
        if op == 3:
            out.setdefault(((w & 0x3FFFFFF) << 2) | 0x80000000, []).append(v + o)
        elif op == 15:
            his[rt] = imm << 16
        elif op == 9 and rs in his:
            target = (his[rs] + imm - (imm & 0x8000) * 2) & 0xFFFFFFFF
            if in_main_text(target) and target % 4 == 0:
                out.setdefault(target, []).append(v + o)
        if op == 2 or (op == 0 and (w & 0x3F) == 8):
            his.clear()  # a jump ends what a lui set up, as a function's end does
        elif op != 15:
            his.pop((w >> 11) & 31 if op == 0 else rt, None)  # anything else written to the register
    return out


def jump_table_words(rom, starts):
    """The addresses of the words in every jump table: where a jr through a register other than $ra jumps to a word
    loaded from a lui's %hi and the lw's %lo, the table runs on while its words point inside the jr's function."""
    starts = sorted(starts)
    out = set()
    _, r, v, t, size = MAIN
    for o in range(0, t, 4):
        w = word(rom, r + o)
        if w >> 26 != 0 or w & 0x3F != 8 or (w >> 21) & 31 == 31:
            continue
        reg = (w >> 21) & 31
        lo = base = None
        for back in range(1, 12):
            x = word(rom, r + o - back * 4)
            if lo is None and x >> 26 == 35 and (x >> 16) & 31 == reg:  # lw reg, %lo(table)(base)
                lo, base = (x & 0xFFFF) - (x & 0x8000) * 2, (x >> 21) & 31
            elif lo is not None and x >> 26 == 15:  # the lui of %hi(table), into base or a register added to it
                table = ((x & 0xFFFF) << 16) + lo & 0xFFFFFFFF
                break
        else:
            continue
        i = bisect.bisect_right(starts, v + o) - 1
        first, end = starts[i], starts[i + 1] if i + 1 < len(starts) else v + t
        if v <= table < v + size:
            a = table
            while a < v + size and first <= word(rom, r + a - v) < end:
                out.add(a)
                a += 4
    return out


def data_pointers(rom, starts, jump_tables):
    """{target: [where]} for the words in the main segment's data that point into its text, but those in jump tables.
    GCC often keeps a table's address in a saved register from the function's start, out of jump_table_words' sight,
    so a run of two or more words that point inside one function, not all at its start, is its jump table too: a
    table of function pointers points at functions' starts."""
    starts = sorted(starts)
    owner = lambda a: starts[max(0, bisect.bisect_right(starts, a) - 1)]
    out = {}
    _, r, v, t, size = MAIN
    run = []
    for o in list(range(t, size, 4)) + [None]:
        w = word(rom, r + o) if o is not None else 0
        pointer = o is not None and in_main_text(w) and w % 4 == 0 and v + o not in jump_tables
        if pointer and run and owner(w) == owner(run[-1][1]):
            run.append((v + o, w))
            continue
        if not (len(run) >= 2 and any(p != owner(p) for _, p in run)):
            for at, p in run:
                out.setdefault(p, []).append(at)
        run = [(v + o, w)] if pointer else []
    return out


def looks_like_start(rom, addr):
    """The code before addr ends a function: a jr $ra or a j and its delay slot, and nothing but nops after them."""
    _, r, v, t, _ = MAIN
    if not v <= addr < v + t:
        return False
    a = addr - 4
    while a > v and word(rom, r + a - v) == 0 and addr - a < 32:
        a -= 4
    for b in (a, a - 4):
        w = word(rom, r + b - v) if b >= v else 0
        if w == 0x03E00008 or w >> 26 == 2:
            return True
    return addr == v


def merge_split_functions(buf, vram, size, starts, called):
    """Joins a function spimdisasm split at a label inside it: when a function branches into the next and nothing calls
    or points at that one, it is the same function."""
    merged = True
    while merged:
        merged = False
        for i in range(len(starts) - 1):
            end = starts[i + 1]
            beyond = [t for t in branch_targets(buf, vram, starts[i], end) if end <= t < vram + size]
            if not beyond:
                continue
            j = i + 1
            while j < len(starts) and starts[j] <= max(beyond):
                j += 1
            if all(s not in called for s in starts[i + 1:j]):
                starts = starts[:i + 1] + starts[j:]
                merged = True
                break
    return starts


def load_registry():
    if not NAMES_PATH.exists():
        return {}
    return {(f["section"], f["vram"]): f["name"] for f in tomllib.loads(NAMES_PATH.read_text()).get("func", [])}


def function_lines(starts, end, names, used):
    out, named = [], 0
    for i, addr in enumerate(starts):
        stop = starts[i + 1] if i + 1 < len(starts) else end
        name = names(addr)
        if name in used:
            raise SystemExit(f"{name} named twice")
        used.add(name)
        out.append(f'    {{ name = "{name}", vram = 0x{addr:08X}, size = 0x{stop - addr:X} }},')
    return out


def main():
    rom = ROM_PATH.read_bytes()
    if hashlib.md5(rom).hexdigest() != ROM_MD5:
        sys.exit(f"{ROM_PATH}: not Dinosaur Planet's December 2000 prototype, MD5 {ROM_MD5}")
    report = "--report" in sys.argv
    registry = load_registry()
    sec, r, v, t, _ = MAIN
    all_dlls = dlls(rom)
    imports = dll_imports(rom)
    with tempfile.TemporaryDirectory() as tmp:
        found = spimdisasm_functions(rom[r:r + t], v, tmp)
        dll_found = {d.number: spimdisasm_functions(rom[d.text_rom:d.text_rom + d.text_size], d.vram, tmp)
                     for d in all_dlls}
    by_code = code_pointers(rom)
    by_data = data_pointers(rom, found, jump_table_words(rom, found))
    by_import = {a: ["the DLLs' imports"] for a in imports if in_main_text(a)}
    candidates = set(by_code) | set(by_data) | set(by_import)
    pointed = {a for a in candidates if a in found or a in by_import or looks_like_start(rom, a)}
    rejected = sorted(a for a in candidates if a not in pointed and in_main_text(a))
    pointed |= EXTRA_FUNCTION_STARTS

    out = ["# Written by tools/gen_syms.py from the ROM; not edited by hand.", ""]
    used = set()
    buf = rom[r:r + t]
    starts = sorted(set(found) | {a for a in pointed if v <= a < v + t})
    new = len(starts) - len(found)
    starts = merge_split_functions(buf, v, t, starts, pointed)
    starts = [s for s in starts if not any(a < s < b for a, b in ONE_FUNCTION)]
    lib = {a: n for a, n in match_library(buf, v, starts, load_sigs()).items() if n not in UNTRUSTED_NAMES}
    manual = set(MANUAL_NAMES.values())
    lib = {a: n for a, n in lib.items() if n not in manual}
    lib.update(MANUAL_NAMES)
    out += ["[[section]]", f'name = "{sec}"', f"rom = 0x{r:08X}", f"vram = 0x{v:08X}", f"size = 0x{t:X}", "",
            "functions = ["]
    seen_lib = set()

    def main_name(a):
        n = registry.get((sec, a)) or lib.get(a)
        if n is None or n in seen_lib:
            return f"func_{a:08X}"
        seen_lib.add(n)
        return n

    out += function_lines(starts, v + t, main_name, used)
    out += ["]", ""]
    print(f"main: {len(starts)} functions ({len(found)} from spimdisasm, {new} more from pointers), "
          f"{len(seen_lib)} named, {len(rejected)} pointers into the middle of a function left alone")
    if report:
        for a in sorted(set(starts) - set(found)):
            print(f"  {a:08X} from " + ", ".join(f"{x}" if isinstance(x, str) else f"{x:08X}"
                                                 for x in (by_code.get(a) or by_data.get(a) or by_import.get(a) or [])[:4]))
        for a in rejected:
            print(f"  left alone: {a:08X}")

    total = 0
    for d in all_dlls:
        name = f"dll{d.number}"
        dv, dt = d.vram, d.text_size
        dbuf = rom[d.text_rom:d.text_rom + dt]
        pointed = {dv + o for o in d.pointed()}
        starts = sorted(set(dll_found[d.number]) | pointed)
        starts = merge_split_functions(dbuf, dv, dt, starts, pointed)
        total += len(starts)
        out += ["[[section]]", f'name = "{name}"', f"rom = 0x{d.text_rom:08X}", f"vram = 0x{dv:08X}",
                f"size = 0x{dt:X}"]
        if d.rel != 0xFFFFFFFF:
            out.append(f"got_address = 0x{d.got_vram():08X}")
        out += ["", "functions = ["]
        out += function_lines(starts, dv + dt,
                              lambda a: registry.get((name, a)) or DLL_MANUAL_NAMES.get((d.number, a - dv))
                              or f"dll{d.number}_{a - dv:04X}", used)
        out += ["]", ""]
        if d.gp_sets:
            out.append("relocs = [")
            for g in d.gp_sets:
                out.append(f'    {{ type = "R_MIPS_HI16", vram = 0x{dv + g:08X}, target_vram = 0x{d.got_vram():08X} }},')
                out.append(f'    {{ type = "R_MIPS_LO16", vram = 0x{dv + g + 4:08X}, target_vram = 0x{d.got_vram():08X} }},')
            out += ["]", ""]
    print(f"{len(all_dlls)} DLLs: {total} functions")
    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUT_PATH.write_text("\n".join(out), newline="\n")
    print(f"wrote {OUT_PATH.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
