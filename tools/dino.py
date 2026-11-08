"""Reads Dinosaur Planet's code from the ROM in names: what a person working out what the game does would otherwise page
through as raw MIPS. Run it from anywhere; it reads roms/dinoplanet.z64, syms/ and syms/names.toml.

    python3 tools/dino.py dis dll210_16220        one function: dll<n>_<offset>, func_<address>, a name, dll<n>.<export>
    python3 tools/dino.py dis dll272 0x18-0x200   a range of a DLL's text (offsets in its text, cut at its end)
    python3 tools/dino.py calls bit 0x1EB         every call to it: bit [<bit>], a function or dll<n>.<export>, args
    python3 tools/dino.py map 29 --name sandeel   a map's cells and group field; its placements summed, or filtered
    python3 tools/dino.py def SHswapstone         an object def: DLL, object ids, where it is placed, its record
    python3 tools/dino.py warp [26]               the warp table: each entry's place, the map holding it, who warps

`dis` prints one function densely: each line its offset in the DLL's text (or its address), then the instruction with
what its registers hold in place of them. Cut: nops, the $gp setups and reloads, the prologue's and epilogue's saves
and restores, and the loads and address arithmetic whose value a later line shows by name. A delay slot is printed
before its branch or call, since it runs first (a likely branch's runs only if taken: `[then ...]`). `>` marks a
branch target. A call is one line, `call <target>(a0=.., a1=..)`, with the arguments set since the last call; a call
to the bit setter or getter is `bit 0x5F4 set 1`, with the bit's name or its task (ROM file 55).

How a value is named, from a dataflow pass over the function (meets at branch targets, s-registers and stack slots
kept across calls):
- A DLL's GOT: an import is the main segment's function or data address it holds, a local word the DLL's own
  function (dll<n>_<offset>) or data (D_<offset in its text>). Main's lui/addiu pairs are addresses too.
- A DLL is called through its handle, which the game keeps in a word: **handle + 4 + 4 * export. The words that
  hold handles are found once in the whole ROM (each store of what the loader, func_8000BE04, returned for an
  immediate DLL number), so `lw t9, 0xC(t3)` on dll29's prints `call dll29.2`, with the export's name if it has one.
- In an object DLL (one an object def in ROM file 66 names at +0x58), each export's a0 is the object, `obj`, and
  export 0's a1 its placement, `setup`. obj+0x68 is its own DLL's handle and obj+0xB8 its `state`. A local function
  whose every caller passes the same kind in an argument gets it too.
- syms/names.toml names functions, addresses, exports, bits and the fields of obj, state and setup (its header says
  how); anything else is its address or offset.

`map` (ROM files 31, 32, 33 and 21) prints a map's cell, the cells it spans and its group field (dll29's data
+0xF0), then a count of each def it places, or with a filter (or --all) one line each: uid, def name, object id, DLL,
group (when flag 0x10 makes it one), flags, position, world position (local + the cell * 640), and the words of its
setup from +0x18, named from syms/names.toml where they are. `def` takes a name, an object id or dll<n>. `warp`
names the maps whose cells and layer hold an entry, and each call to func_8004D1D4 that warps there.

What it reads of the whole ROM (the handles, each function's argument kinds, every call site) is kept in
build/dino/<the ROM's md5>/, written the first time and read after; nothing from the ROM goes in git.
"""

import argparse
import concurrent.futures
import hashlib
import os
import pickle
import re
import struct
import sys
import tomllib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_syms as g  # noqa: E402

ROOT = g.ROOT
SYMS_PATH = ROOT / "syms" / "dinoplanet.syms.toml"
NAMES_PATH = ROOT / "syms" / "names.toml"
CACHE_ROOT = ROOT / "build" / "dino"

OBJECT_TAB_FILE, OBJECT_BIN_FILE, OBJECT_INDEX_FILE, BIT_TABLE_FILE = 65, 66, 67, 55
PLACEMENTS_FILE, MAP_TABLE_FILE, MAP_NAMES_FILE, MAP_GRID_FILE, WARPS_FILE = 31, 32, 33, 21, 30
CELL = 640
WARP_ENTRY = 0x8004D1D4
DLL_LOAD = 0x8000BE04
BIT_GET, BIT_SET = 0x800147EC, 0x80014670

GPR = ["zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1",
       "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"]
SAVED = {16, 17, 18, 19, 20, 21, 22, 23, 28, 29, 30}
ARGS = (4, 5, 6, 7)


def s16(x):
    return (x ^ 0x8000) - 0x8000


# ---- The ROM, the symbols and the names ----

class Game:
    def __init__(self):
        rom_path = g.ROM_PATH
        if not rom_path.exists():
            sys.exit(f"no ROM at {rom_path}")
        self.rom = rom_path.read_bytes()
        self.md5 = hashlib.md5(self.rom).hexdigest()
        self.imports = g.dll_imports(self.rom)
        self.dlls = {d.number: d for d in g.dlls(self.rom)}
        self.read_syms()
        self.read_names()
        self.read_defs()
        self.read_bit_table()
        self.read_maps()
        self.handle_words = {}
        for d in self.dlls.values():
            d.exports_set = {e for e in d.exports if e}

    def read_syms(self):
        """Each section's functions, as (start, end) in its own terms: an address in main, an offset in a DLL's text."""
        self.funcs, self.func_names = {}, {}
        for s in tomllib.loads(SYMS_PATH.read_text())["section"]:
            name, base = s["name"], s["vram"]
            key = "main" if name == "main" else int(name[3:])
            rel = 0 if key == "main" else base
            spans = sorted((f["vram"] - rel, f["vram"] - rel + f["size"], f["name"]) for f in s["functions"])
            self.funcs[key] = spans
            for a, _, n in spans:
                self.func_names[(key, a)] = n

    def read_names(self):
        t = tomllib.loads(NAMES_PATH.read_text()) if NAMES_PATH.exists() else {}
        for f in t.get("func", []):
            sec = "main" if f["section"] == "main" else int(f["section"][3:])
            a = f["vram"] - (0 if sec == "main" else next(s for s in self.dlls.values() if s.number == sec).vram)
            self.func_names[(sec, a)] = f["name"]
        self.addr_names = {a["vram"]: a["name"] for a in t.get("addr", [])}
        self.returns = {a["vram"]: a["returns"] for a in t.get("addr", []) if "returns" in a}
        self.export_names = {(e["dll"], e["index"]): e["name"] for e in t.get("export", [])}
        self.bit_names = {b["bit"]: b["name"] for b in t.get("bit", [])}
        self.fields = {(f["of"], f.get("dll"), f["offset"]): f["name"] for f in t.get("field", [])}
        self.by_name = {n: k for k, n in self.func_names.items()}
        self.by_name.update({n: ("main", a) for a, n in self.addr_names.items()})

    def read_defs(self):
        o, n = g.rom_file(self.rom, OBJECT_TAB_FILE)
        tab = struct.unpack_from(f">{n // 4}I", self.rom, o)
        b, _ = g.rom_file(self.rom, OBJECT_BIN_FILE)
        self.defs = []
        for i in range(len(tab) - 1):
            if tab[i + 1] == 0xFFFFFFFF:
                break
            r = self.rom[b + tab[i]:b + tab[i + 1]]
            v = struct.unpack_from(">H", r, 0x58)[0]
            self.defs.append((r[0x5F:0x6F].split(b"\0")[0].decode("latin-1"), (v & 0x7FFF) + 209 if v & 0x8000 else v))
        self.object_dlls = {d for _, d in self.defs if d}

    def read_maps(self):
        """Each map: its name (file 33), its header and placements (file 31, at the offsets file 32 gives), its cell
        and layer (file 21) and its group field (dll29's data +0xF0)."""
        rom = self.rom
        o, n = g.rom_file(rom, OBJECT_INDEX_FILE)
        self.object_def = struct.unpack_from(f">{n // 2}h", rom, o)
        names, nn = g.rom_file(rom, MAP_NAMES_FILE)
        tab, nt = g.rom_file(rom, MAP_TABLE_FILE)
        self.placements_rom, _ = g.rom_file(rom, PLACEMENTS_FILE)
        grid, ng = g.rom_file(rom, MAP_GRID_FILE)
        cells = {}
        for i in range(0, ng, 12):
            x, z, layer, m, a, b = struct.unpack_from(">6h", rom, grid + i)
            cells[m] = (x, z, layer, [k for k in (a, b) if k >= 0])
        d29 = self.dlls[29]
        self.maps = {}
        for m in range(min(nn // 32, nt // 28)):
            head, _, _, _, start, end, _ = struct.unpack_from(">7I", rom, tab + m * 28)
            name = rom[names + m * 32:names + m * 32 + 32].split(b"\0")[0].decode("latin-1")
            w, h, ox, oz = struct.unpack_from(">4h", rom, self.placements_rom + head)
            field = struct.unpack_from(">H", rom, d29.rom_addr + d29.data + 0xF0 + 2 * m)[0]
            self.maps[m] = {"name": name, "start": start, "end": end, "size": (w, h), "offset": (ox, oz),
                            "cell": cells.get(m), "field": field}

    def placements(self, m):
        """A map's placements: (offset in its list, the record)."""
        mp = self.maps[m]
        p, out = mp["start"], []
        while p < mp["end"]:
            size = self.rom[self.placements_rom + p + 2] * 4
            if size < 0x18:
                break
            out.append((p - mp["start"], self.rom[self.placements_rom + p:self.placements_rom + p + size]))
            p += size
        return out

    def def_of(self, obj):
        return self.object_def[obj] if obj < len(self.object_def) and self.object_def[obj] < len(self.defs) else -1

    def world(self, m, x, z):
        c = self.maps[m]["cell"]
        return (x + c[0] * CELL, z + c[1] * CELL) if c else None

    def maps_at(self, x, z, layer):
        """The maps whose cells hold a world position: a map's cells run from its file 21 cell less its header's
        +4 and +6, for +0 by +2 cells."""
        cx, cz = int(x // CELL), int(z // CELL)
        out = []
        for m, mp in self.maps.items():
            c = mp["cell"]
            if not c or c[2] != layer:
                continue
            x0, z0 = c[0] - mp["offset"][0], c[1] - mp["offset"][1]
            if x0 <= cx < x0 + mp["size"][0] and z0 <= cz < z0 + mp["size"][1]:
                out.append(m)
        return out

    def read_bit_table(self):
        o, n = g.rom_file(self.rom, BIT_TABLE_FILE)
        self.bit_task = {}
        for i in range(n // 4):
            _, flags, task = struct.unpack_from(">HBB", self.rom, o + i * 4)
            if flags & 0x20:
                self.bit_task[i] = task

    # Function lookups.
    def func_at(self, sec, a):
        spans = self.funcs.get(sec, [])
        lo, hi = 0, len(spans)
        while lo < hi:
            mid = (lo + hi) // 2
            if spans[mid][0] <= a:
                lo = mid + 1
            else:
                hi = mid
        if lo and spans[lo - 1][0] <= a < spans[lo - 1][1]:
            return spans[lo - 1]
        return None

    def text(self, sec):
        if sec == "main":
            _, r, v, t, _ = g.MAIN
            return self.rom[r:r + t], v
        return self.dlls[sec].text_bytes, 0

    # Names of values.
    def addr_name(self, sec, a):
        if sec == "main":
            if a in self.addr_names:
                return self.addr_names[a]
            if ("main", a) in self.func_names:
                return self.func_names[("main", a)]
            if (sec, a) in self.handle_words:
                return f"&dll{self.handle_words[(sec, a)]}"
            return f"D_{a:08X}"
        if (sec, a) in self.func_names:
            return self.func_names[(sec, a)]
        if (sec, a) in self.handle_words:
            return f"&dll{self.handle_words[(sec, a)]}"
        return f"D_{a:04X}"

    def func_name(self, key):
        if key[0] == "export":
            _, n, k = key
            e = self.export_names.get((n, k))
            return f"dll{n}.{k}" + (f" {e}" if e else "")
        sec, a = key
        return self.addr_name(sec, a)

    def field(self, of, dll, off):
        return self.fields.get((of, dll, off)) or self.fields.get((of, None, off))

    def bit_label(self, bit):
        if bit in self.bit_names:
            return self.bit_names[bit]
        if bit in self.bit_task:
            return f"task {self.bit_task[bit]}"
        return None


# ---- The dataflow pass over one function ----
#
# A value is None (unknown) or a tuple: ("imm", n), ("addr", sec, a), ("mem", sec, a) (the word at an address),
# ("func", key), ("handle", n), ("table", n), ("obj"|"state"|"setup", dll, label), ("ret", key).

def as_addr(v):
    """A constant that is a main-segment address (a lui's, or with its low half) as that address."""
    if v and v[0] == "imm" and 0x80000000 <= v[1] < 0x80800000:
        return ("addr", "main", v[1])
    return v


def decode(w):
    return w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31, w & 0x3F, w & 0xFFFF


def meet(a, b):
    if a is None:
        return dict(b)
    return {k: v for k, v in a.items() if b.get(k) == v}


def branch_target(o, w):
    """For a branch or jump at o (main: its address; a DLL: its offset), where it goes; None for none or a register."""
    op, rs, rt, _, fn, imm = decode(w)
    if op in (4, 5, 6, 7, 0x14, 0x15, 0x16, 0x17) or (op == 1 and rt in (0, 1, 2, 3, 0x10, 0x11, 0x12, 0x13)):
        return o + 4 + s16(imm) * 4
    if op == 0x11 and rs == 8:  # bc1f/bc1t and their likely forms
        return o + 4 + s16(imm) * 4
    if op in (2, 3):
        return (o & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
    return None


def is_likely(w):
    op, rs, rt, *_ = decode(w)
    return op in (0x14, 0x15, 0x16, 0x17) or (op == 1 and rt in (2, 3, 0x12, 0x13)) or (op == 0x11 and rs == 8 and rt & 2)


def has_slot(w):
    op, rs, rt, _, fn, _ = decode(w)
    return branch_target(0, w) is not None or op == 3 or (op == 0 and fn in (8, 9))


def ends_flow(w):
    op, rs, rt, _, fn, _ = decode(w)
    return (op == 0 and fn == 8) or op == 2 or (op == 4 and rs == 0 and rt == 0)


class Flow:
    """What each register holds before each instruction of a function, and its calls."""

    def __init__(self, game, sec, start, end, entry):
        self.game, self.sec, self.start, self.end = game, sec, start, end
        buf, base = game.text(sec)
        self.words = [struct.unpack_from(">I", buf, o - base)[0] for o in range(start, end, 4)]
        self.dll = None if sec == "main" else game.dlls[sec]
        self.entry = entry
        self.targets = set()
        for i, w in enumerate(self.words):
            t = branch_target(self.addr(i), w)
            if t is not None and start <= t < end and w >> 26 != 3:
                self.targets.add((t - start) // 4)
        self.run()

    def addr(self, i):
        return self.start + 4 * i

    def got_value(self, imm):
        d = self.dll
        k = imm // 4
        if imm & 3 or k >= len(d.got):
            return None
        w = d.got[k]
        if w & 0x80000000:
            a = self.game.imports[(w & 0x7FFFFFFF) - 1]
            return ("func", ("main", a)) if ("main", a) in self.game.func_names else ("addr", "main", a)
        if (self.sec, w) in self.game.func_names:
            return ("func", (self.sec, w))
        return ("addr", self.sec, w)

    def at(self, sec, a):
        return ("func", (sec, a)) if (sec, a) in self.game.func_names else ("addr", sec, a)

    def load(self, base, rs, imm, op):
        if rs == 28 and self.dll is not None:
            return self.got_value(imm) if op == 0x23 else None
        if op != 0x23:
            return None
        if rs == 29:
            return None
        base = as_addr(base)
        if base is None:
            return None
        kind = base[0]
        if kind == "addr":
            sec, a = base[1], base[2] + s16(imm)
            n = self.game.handle_words.get((sec, a))
            return ("handle", n) if n is not None else ("mem", sec, a)
        if kind == "handle" and imm == 0:
            return ("table", base[1])
        if kind == "table" and imm >= 4 and imm % 4 == 0:
            return ("func", ("export", base[1], imm // 4 - 1))
        if kind == "obj":
            if imm == 0x68:
                return ("handle", base[1])
            if imm == 0xB8:
                return ("state", base[1], base[2])
            if imm == 0x50:
                return ("def", base[1], base[2])
        return None

    def step(self, st, i, calls):
        """st after instruction i, in place."""
        w = self.words[i]
        op, rs, rt, rd, fn, imm = decode(w)
        get = lambda r: ("imm", 0) if r == 0 else st.get(r)
        out, val = None, None
        if op == 0x0F:
            out, val = rt, ("imm", imm << 16)
        elif op in (0x08, 0x09):
            b = get(rs)
            out = rt
            if b and b[0] == "imm":
                val = ("imm", (b[1] + s16(imm)) & 0xFFFFFFFF)
            elif b and b[0] == "addr":
                val = self.at(b[1], b[2] + s16(imm))
            elif b and b[0] == "func" and b[1][0] != "export" and imm:
                val = self.at(b[1][0], b[1][1] + s16(imm))
            elif b and b[0] == "func":
                val = b
        elif op == 0x0D:
            b = get(rs)
            out = rt
            if b and b[0] == "imm":
                val = ("imm", b[1] | imm)
        elif op == 0 and fn in (0x21, 0x25, 0x2D) and (rs == 0 or rt == 0):
            out, val = rd, get(rt if rs == 0 else rs)
        elif 0x20 <= op <= 0x27:
            if rs == 29 and op == 0x23:
                out, val = rt, st.get(("sp", imm))
            else:
                out, val = rt, self.load(get(rs), rs, imm, op)
        elif op in (0x28, 0x29, 0x2B):
            if rs == 29:
                if op == 0x2B and get(rt) is not None:
                    st[("sp", imm)] = get(rt)
                else:
                    st.pop(("sp", imm & ~3), None)
            elif op == 0x2B and calls is not None:
                v, b = get(rt), as_addr(get(rs))
                if v and v[0] == "handle" and v[1] is not None and b and b[0] == "addr":
                    calls.append(("store", b[1], b[2] + s16(imm), v[1]))
        elif op == 0 and fn in (8, 9) or op in (2, 3):
            pass
        elif op == 0x31 or op == 0x35 or op in (0x39, 0x3D):
            pass
        elif op == 0x11 or op == 0x10:
            if op == 0x11 and rs in (0, 1, 2) and rt:  # mfc1, dmfc1, cfc1
                out = rt
            elif op == 0x10 and rs == 0:
                out = rt
        elif op == 0 and fn in (0x10, 0x12):
            out = rd
        elif op == 0 and fn in (0x18, 0x19, 0x1A, 0x1B, 0x11, 0x13, 0x0C, 0x0D, 0x0F):
            pass
        elif op == 0:
            out = rd
        elif op in (0x0A, 0x0B, 0x0C, 0x0E):
            out = rt
        elif op == 1 or 4 <= op <= 7 or 0x14 <= op <= 0x17:
            pass
        else:
            out = rt if op < 0x28 else None
        if out:
            if val is None:
                st.pop(out, None)
            else:
                st[out] = val
        if (op == 0 and fn == 9) or op == 3:
            self.call(st, i, calls)

    def call(self, st, i, calls):
        w = self.words[i]
        op, rs, *_ = decode(w)
        if op == 3:
            tgt = ("func", ("main", branch_target(self.addr(i), w)))
        else:
            tgt = st.get(rs)
        key = tgt[1] if tgt and tgt[0] == "func" else None
        args = {r: st.get(r) for r in ARGS if r in st}
        if calls is not None:
            calls.append(("call", i, key, args))
        for r in list(st):
            if isinstance(r, int) and r not in SAVED:
                del st[r]
        if key == ("main", DLL_LOAD) and args.get(4, (None,))[0] == "imm":
            st[2] = ("handle", args[4][1])
        elif key and key[0] == "main" and key[1] in self.game.returns:
            r = self.game.returns[key[1]].split()
            st[2] = (r[0], int(r[1]) if len(r) > 1 else None, r[2] if len(r) > 2 else r[0])
        elif key:
            st[2] = ("ret", key)

    def run(self):
        n = len(self.words)
        self.before = [None] * n
        incoming = {}  # a branch target's index: {the branch's index: the state it brings}
        for _ in range(8):
            changed = False
            # Code no known branch reaches (a jump table's cases) starts with what the s-registers and the stack
            # held where the flow before it ended.
            st, rest, calls = dict(self.entry), dict(self.entry), []
            i = 0
            live = True
            while i < n:
                if i in self.targets:
                    m = st if live else None
                    for s in incoming.get(i, {}).values():
                        m = meet(m, s)
                    st = m if m is not None else dict(rest)
                elif not live:
                    st = dict(rest)
                live = True
                self.before[i] = dict(st)
                w = self.words[i]
                if has_slot(w) and i + 1 < n:
                    op, rs, _, _, fn, _ = decode(w)
                    t = branch_target(self.addr(i), w)
                    pre = dict(st)
                    self.before[i + 1] = dict(st)
                    is_call = op == 3 or (op == 0 and fn == 9)
                    slot = dict(st)
                    self.step(slot, i + 1, calls)
                    if is_call:
                        # The slot runs before the call: step the call on the state after the slot.
                        self.step(slot, i, calls)
                        st = slot
                    else:
                        if t is not None and self.start <= t < self.end:
                            into = incoming.setdefault((t - self.start) // 4, {})
                            if into.get(i) != slot:
                                into[i] = slot
                                changed = True
                        st = pre if is_likely(w) else slot
                        if ends_flow(w):
                            live = False
                            rest = {k: v for k, v in slot.items() if not isinstance(k, int) or k in SAVED}
                    i += 2
                    continue
                self.step(st, i, calls)
                i += 1
            self.calls = calls
            if not changed:
                break


# ---- The ROM-wide index: the words that hold handles, the arguments' kinds, every call ----

def entry_state(game, sec, a, argkinds):
    st = {29: ("sp",)}
    if sec != "main" and sec in game.object_dlls and a in game.dlls[sec].exports_set:
        st[4] = ("obj", sec, "obj")
        if game.dlls[sec].exports and a == game.dlls[sec].exports[0]:
            st[5] = ("setup", sec, "setup")
    for r, v in argkinds.get((sec, a), {}).items():
        st.setdefault(r, v)
    return st


def all_functions(game):
    for sec, spans in game.funcs.items():
        for a, b, _ in spans:
            yield sec, a, b


WORKER = None


def read_sections(job):
    """One worker's share of a round: its sections' handle stores, calls, and the kinds their own calls pass."""
    global WORKER
    secs, handles, argkinds = job
    if WORKER is None:
        WORKER = Game()
    game = WORKER
    game.handle_words = handles
    stores, calls, seen = {}, [], {}
    for sec in secs:
        for a, b, _ in game.funcs[sec]:
            f = Flow(game, sec, a, b, entry_state(game, sec, a, argkinds))
            for c in f.calls:
                if c[0] == "store":
                    stores[(c[1], c[2])] = c[3]
                    continue
                _, i, key, args = c
                calls.append((sec, a, a + 4 * i, key, args))
                if key and key[0] == sec and sec != "main":
                    kinds = {r: v for r, v in args.items() if v and v[0] in ("obj", "state", "setup", "def")}
                    s = seen.get(key)
                    seen[key] = kinds if s is None else {r: v for r, v in s.items() if kinds.get(r) == v}
    return stores, calls, seen


def build_index(game):
    """Rounds over every function until the handles and the arguments' kinds stop changing: a round finds the words
    that hold handles by what the last round knew, and a local function's kinds by what its callers passed."""
    secs = sorted(game.funcs, key=lambda s: -len(game.funcs[s]))
    jobs = [secs[i::os.cpu_count() * 2] for i in range(os.cpu_count() * 2)]
    handles, argkinds = {}, {}
    with concurrent.futures.ProcessPoolExecutor() as pool:
        for _ in range(4):
            stores, calls, seen = {}, [], {}
            for s, c, k in pool.map(read_sections, [(j, handles, argkinds) for j in jobs]):
                stores.update(s)
                calls += c
                seen.update(k)
            new = {k: v for k, v in seen.items() if v}
            if stores == handles and new == argkinds:
                break
            handles, argkinds = stores, new
    return {"handles": handles, "argkinds": argkinds, "calls": calls}


def load_index(game):
    """The index, kept by the ROM's md5 and by what it was read with: syms/, the names' `returns` and this file."""
    key = hashlib.md5(SYMS_PATH.read_bytes() + repr(sorted(game.returns.items())).encode()
                      + Path(__file__).read_bytes()).hexdigest()[:12]
    path = CACHE_ROOT / game.md5 / f"index-{key}.pickle"
    if path.exists():
        idx = pickle.loads(path.read_bytes())
    else:
        print("reading the whole ROM once, into build/dino/ (a minute)...", file=sys.stderr)
        idx = build_index(game)
        path.parent.mkdir(parents=True, exist_ok=True)
        for old in path.parent.glob("index-*.pickle"):
            old.unlink()
        path.write_bytes(pickle.dumps(idx))
    game.handle_words = idx["handles"]
    return idx


# ---- Printing ----

def fmt_imm(n):
    n &= 0xFFFFFFFF
    if n & 0x80000000 and n >= 0xFFFF0000:
        return str(n - 0x100000000)
    return str(n) if n < 10 else hex(n)


def value_name(game, v):
    if v is None:
        return None
    k = v[0]
    if k == "imm":
        x = v[1] & 0xFFFFFFFF
        if 0x80000000 <= x < 0x80800000 and (x & 0xFFFF):
            return game.addr_name("main", x)
        return fmt_imm(x)
    if k == "addr":
        return game.addr_name(v[1], v[2])
    if k == "mem":
        return f"[{game.addr_name(v[1], v[2])}]"
    if k == "func":
        return game.func_name(v[1])
    if k == "handle":
        return f"dll{v[1]}" if v[1] is not None else "dll?"
    if k == "table":
        return f"dll{v[1]}.table"
    if k in ("obj", "state", "setup", "def"):
        return v[2] if k == "obj" or v[2] == k else f"{v[2]}.{k}"
    if k == "ret":
        return f"ret({game.func_name(v[1])})"
    return None


def mem_operand(game, base, rs, imm):
    off = s16(imm)
    if rs == 29:
        return f"sp+{off:#x}"
    if base:
        k = base[0]
        if k in ("obj", "state", "setup", "def"):
            f = game.field(k, base[1], off)
            name = value_name(game, base)
            return f"{name}.{f}" if f else f"{name}+{off:#x}"
        if k == "addr":
            return game.addr_name(base[1], base[2] + off)
        if k == "imm" and 0x80000000 <= ((base[1] + off) & 0xFFFFFFFF) < 0x80800000:
            return game.addr_name("main", (base[1] + off) & 0xFFFFFFFF)
        n = value_name(game, base)
        if n:
            return f"{n}+{off:#x}" if off else n
    return f"{off:#x}({GPR[rs]})"


def is_quiet(f, i):
    """Instructions that print nothing: nops, $gp, the frame and the arguments' homes in the caller's."""
    w = f.words[i]
    op, rs, rt, rd, fn, imm = decode(w)
    if w == 0:
        return True
    if op == 0x2B and rs == 29 and rt in ARGS and imm >= f.frame:
        return True
    if rt == 28 and op in (0x0F, 0x0D, 0x23) or (op == 0x2B and rt == 28 and rs == 29):
        return True
    if op == 0 and rd == 28:
        return True
    if op == 0x09 and rs == 29 and rt == 29:
        return True
    if rs == 29 and op in (0x23, 0x2B) and rt in (16, 17, 18, 19, 20, 21, 22, 23, 30, 31):
        return True
    if rs == 29 and op in (0x35, 0x3D) and 20 <= rt <= 30:  # ldc1/sdc1 of the saved FPRs
        return True
    return False


def folds(f, i):
    """A load or computation of a known value (an address, a function, a handle, a constant) into a register that a
    later line reads by name before anything else touches it, with no branch target between: it prints nothing."""
    w = f.words[i]
    op, rs, rt, rd, fn, imm = decode(w)
    if op in (0x0F, 0x09, 0x0D, 0x23) and rt != 0:
        out = rt
    elif op == 0 and fn in (0x21, 0x25, 0x2D) and (rs == 0 or rt == 0):
        out = rd
    else:
        return False
    after = f.before[i + 1] if i + 1 < len(f.words) and f.before[i + 1] is not None else None
    if after is None or after.get(out) is None:
        return False
    v = after[out]
    for j in range(i + 1, len(f.words)):
        if j in f.targets or f.before[j] is None:
            return False
        reads, writes = regs(f.words[j])
        if out in reads:
            return f.before[j].get(out) == v and shows(f, j, out, v)
        if out in writes:
            return True
    return True


def shows(f, j, r, v):
    """Whether instruction j's line prints register r by its value v."""
    op, rs, rt, rd, fn, _ = decode(f.words[j])
    if op == 3 or (op == 0 and fn == 9):
        return r in ARGS or r == rs
    if 0x20 <= op <= 0x3F:
        return r == rs or (0x28 <= op <= 0x2E and r == rt and v[0] != "imm")
    if branch_target(0, f.words[j]) is not None:
        return True
    if op == 0 and fn in (0x21, 0x25, 0x2D) and (rs == 0 or rt == 0):
        return True
    if 0x08 <= op <= 0x0E:
        return r == rs
    return op == 0 and fn >= 0x20


def regs(w):
    """(read, written) general registers of an instruction."""
    op, rs, rt, rd, fn, _ = decode(w)
    if op == 0:
        if fn in (8,):
            return {rs}, set()
        if fn == 9:
            return {rs} | set(ARGS), {rd} | set(range(1, 16)) | {24, 25}
        if fn in (0x10, 0x12):
            return set(), {rd}
        if fn in (0x11, 0x13):
            return {rs}, set()
        if fn in (0x18, 0x19, 0x1A, 0x1B):
            return {rs, rt}, set()
        if fn in (0, 2, 3):
            return {rt}, {rd}
        return {rs, rt}, {rd}
    if op == 3:
        return set(ARGS), {31} | set(range(1, 16)) | {24, 25}
    if op == 2:
        return set(), set()
    if op == 1:
        return {rs}, set()
    if op in (4, 5, 0x14, 0x15):
        return {rs, rt}, set()
    if op in (6, 7, 0x16, 0x17):
        return {rs}, set()
    if op == 0x0F:
        return set(), {rt}
    if 0x08 <= op <= 0x0E or 0x20 <= op <= 0x27:
        return {rs}, {rt}
    if 0x28 <= op <= 0x2E:
        return {rs, rt}, set()
    if op in (0x31, 0x35, 0x39, 0x3D):
        return {rs}, set()
    if op == 0x11 and rs in (0, 1):
        return set(), {rt}
    if op == 0x11 and rs in (4, 5):
        return {rt}, set()
    return set(), set()


def render(game, f, i):
    """One instruction, its registers replaced by what they hold."""
    import rabbitizer
    w = f.words[i]
    op, rs, rt, rd, fn, imm = decode(w)
    st = f.before[i] or {}
    o = f.addr(i)
    ins = rabbitizer.Instruction(w, vram=o if f.sec == "main" else 0x1000000 + o)
    mn = ins.getOpcodeName()
    text = re.sub(r"\s+", " ", ins.disassemble().replace("$", "")).strip()
    reg = lambda r: value_name(game, ("imm", 0) if r == 0 else st.get(r)) or GPR[r]
    t = branch_target(o, w)
    if t is not None:
        two = op in (4, 5, 0x14, 0x15) and rt != 0
        names = [reg(r) if st.get(r) else GPR[r] for r in ((rs, rt) if two else (rs,))]
        if op == 2 or (op == 4 and rs == 0 and rt == 0):
            return f"b -> {t:X}"
        if op in (4, 5, 0x14, 0x15) and rt == 0:
            mn = ("beqz", "bnez", "beqzl", "bnezl")[(4, 5, 0x14, 0x15).index(op)]
        if op == 0x11:
            return f"{mn} -> {t:X}"
        return f"{mn} {', '.join(names)} -> {t:X}"
    if 0x20 <= op <= 0x3F and op not in (0x2F, 0x33, 0x37, 0x3B, 0x3E, 0x3F):
        m = mem_operand(game, st.get(rs), rs, imm)
        first = text.split(" ", 1)[1].split(",")[0]
        if 0x28 <= op <= 0x2E:
            v = st.get(rt) if rt else ("imm", 0)
            n = value_name(game, v)
            if n and v and v[0] != "imm":
                first = n
        return f"{mn} {first}, {m}"
    if op == 0 and fn in (0x21, 0x25, 0x2D) and (rs == 0 or rt == 0):
        src = rt if rs == 0 else rs
        return f"move {GPR[rd]}, {reg(src)}"
    if op in (0x09, 0x0D) and rs == 0:
        return f"li {GPR[rt]}, {fmt_imm(s16(imm) if op == 9 else imm)}"
    if op in (0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E) and st.get(rs):
        parts = text.split(", ")
        if len(parts) == 3:
            parts[1] = reg(rs)
            return ", ".join(parts)
    if op == 0 and fn not in (8, 9) and (st.get(rs) or st.get(rt)):
        parts = text.split(" ", 1)
        if len(parts) == 2:
            ops = parts[1].split(", ")
            if len(ops) == 3 and fn >= 0x20:
                ops[1], ops[2] = reg(rs), reg(rt)
                return f"{parts[0]} {', '.join(ops)}"
    return text


def render_call(game, f, i, args):
    w = f.words[i]
    op, rs, *_ = decode(w)
    st = f.before[i] or {}
    key = branch_target(f.addr(i), w) if op == 3 else None
    tgt = ("main", key) if key is not None else (st.get(rs)[1] if st.get(rs) and st.get(rs)[0] == "func" else None)
    if tgt is None:
        v = st.get(rs)
        name = value_name(game, v) if v else GPR[rs]
        return f"call {name}({fmt_args(game, args)})"
    if tgt == ("main", BIT_SET) or tgt == ("main", BIT_GET):
        b = args.get(4)
        bit = f"{b[1]:#x}" if b and b[0] == "imm" else (value_name(game, b) or "a0")
        label = game.bit_label(b[1]) if b and b[0] == "imm" else None
        if tgt == ("main", BIT_SET):
            v = value_name(game, args.get(5)) or "a1"
            s = f"bit {bit} set {v}"
        else:
            s = f"bit {bit} get"
        return s + (f"  ; {label}" if label else "")
    if tgt == ("main", DLL_LOAD) and args.get(4) and args[4][0] == "imm":
        return f"dll{args[4][1]} = load({fmt_args(game, args)})"
    return f"call {game.func_name(tgt)}({fmt_args(game, args)})"


def fmt_args(game, args):
    out = []
    for r in ARGS:
        if r in args:
            out.append(f"{GPR[r]}={value_name(game, args[r]) or '?'}")
    return ", ".join(out)


def call_args(f, i):
    """The arguments set since the last call or the function's start, as the call at i sees them (its slot run)."""
    st = dict(f.before[i] or {})
    if i + 1 < len(f.words):
        f.step(st, i + 1, None)
    first = 0
    for j in range(i - 1, -1, -1):
        op, _, _, _, fn, _ = decode(f.words[j])
        if op == 3 or (op == 0 and fn == 9):
            first = j + 2
            break
    written = set()
    for j in list(range(first, i)) + [i + 1]:
        written |= regs(f.words[j])[1] if j < len(f.words) else set()
    if first == 0:
        written |= {r for r in ARGS if r in f.entry}
    return {r: st.get(r) for r in ARGS if r in written}


def dis(game, idx, sec, start, end, title, kinds):
    entry = entry_state(game, sec, start, idx["argkinds"])
    entry.update(kinds)
    f = Flow(game, sec, start, end, entry)
    f.frame = next((-s16(w & 0xFFFF) for w in f.words[:8] if w >> 16 == 0x27BD and w & 0x8000), 0)
    lines = [title]
    kinds = [f"{GPR[r]}={value_name(game, v)}" for r, v in entry.items() if isinstance(r, int) and r in ARGS]
    if kinds:
        lines[0] += "  (" + ", ".join(kinds) + ")"
    n = len(f.words)
    i = 0
    width = 5 if sec != "main" else 8

    def emit(j, text):
        mark = ">" if j in f.targets else " "
        lines.append(f"{f.addr(j):0{width}X}{mark} {text}")

    def plain(j):
        if is_quiet(f, j) or folds(f, j):
            return None
        return render(game, f, j)

    while i < n:
        w = f.words[i]
        op, rs, rt, rd, fn, imm = decode(w)
        if has_slot(w) and i + 1 < n:
            is_call = op == 3 or (op == 0 and fn == 9)
            args = call_args(f, i) if is_call else None
            slot = None if is_call and slot_is_arg(f, i, args) else plain(i + 1)
            if is_call:
                if slot:
                    emit(i + 1, slot)
                emit(i, render_call(game, f, i, args))
            elif op == 0 and fn == 8 and rs == 31:
                if slot:
                    emit(i + 1, slot)
                st = dict(f.before[i] or {})
                f.step(st, i + 1, None)
                v = st.get(2)
                emit(i, "return" + (f" {value_name(game, v)}" if v else ""))
            elif is_likely(w):
                emit(i, render(game, f, i) + (f"  [then {slot}]" if slot else ""))
            else:
                if slot:
                    emit(i + 1, slot)
                emit(i, render(game, f, i) if not (op == 0 and fn == 8) else f"jr {GPR[rs]} (a jump table)")
            i += 2
            continue
        t = plain(i)
        if t:
            emit(i, t)
        i += 1
    return lines


def slot_is_arg(f, i, args):
    """A call's delay slot that only sets an argument to a value the call line shows."""
    w = f.words[i + 1]
    if w == 0 or is_quiet(f, i + 1):
        return True
    op, rs, rt, rd, fn, imm = decode(w)
    _, writes = regs(w)
    simple = op in (0x09, 0x0D, 0x0F) or (op == 0 and fn in (0x21, 0x25, 0x2D) and (rs == 0 or rt == 0))
    return simple and len(writes) == 1 and writes <= set(ARGS) and args.get(next(iter(writes))) is not None


# ---- Maps, defs and warps ----

def find_map(game, spec):
    if spec.isdigit():
        return int(spec)
    hits = [m for m, mp in game.maps.items() if spec.lower() in mp["name"].lower()]
    if len(hits) != 1:
        sys.exit(f"{spec}: " + (", ".join(f"{m} {game.maps[m]['name']}" for m in hits) or "no such map"))
    return hits[0]


def map_header(game, m):
    mp = game.maps[m]
    s = f"map {m} {mp['name']}"
    c = mp["cell"]
    if c:
        x0, z0 = c[0] - mp["offset"][0], c[1] - mp["offset"][1]
        s += (f"  cell ({c[0]}, {c[1]}) = world ({c[0] * CELL}, {c[1] * CELL}), layer {c[2]}, cells x {x0}.."
              f"{x0 + mp['size'][0] - 1} z {z0}..{z0 + mp['size'][1] - 1}")
        if c[3]:
            s += ", with maps " + ", ".join(f"{k} {game.maps[k]['name']}" for k in c[3] if k in game.maps)
    else:
        s += "  (no cell in file 21)"
    return s + f"  group field {mp['field']:#x}"


def placement_line(game, m, off, r, fields):
    obj, _, _, flags, _, group = struct.unpack_from(">HBBBBB", r, 0)
    x, y, z = struct.unpack_from(">3f", r, 8)
    uid = struct.unpack_from(">I", r, 0x14)[0]
    d = game.def_of(obj)
    name, dll = game.defs[d] if d >= 0 else ("?", 0)
    w = game.world(m, x, z)
    s = f"{uid:x} {name} #{obj:X} dll{dll}" + (f" g{group}" if flags & 0x10 else "") + f" fl{flags:02x}"
    s += f" ({x:.0f}, {y:.0f}, {z:.0f})" + (f" w({w[0]:.0f}, {w[1]:.0f})" if w else "")
    if fields:
        words = [struct.unpack_from(">H", r, o)[0] for o in range(0x18, len(r) - 1, 2)]
        while words and not words[-1]:
            words.pop()
        named = []
        for i, v in enumerate(words):
            f = game.field("setup", dll, 0x18 + 2 * i)
            named.append(f"{f}={v:x}" if f else f"{v:04x}")
        if named:
            s += "  +18: " + " ".join(named)
    return s


def loads_in(r, act):
    """Whether the object loader (func_8004B4A0) places this record in a map's act: byte +3's bit act - 1 set skips it
    in acts 1 to 8, byte +5's bit 16 - act in acts 9 to 16; act 0 places all."""
    if act == 0:
        return True
    return not ((r[3] >> (act - 1)) & 1 if act < 9 else (r[5] >> (16 - act)) & 1)


def show_map(game, a):
    m = find_map(game, a.map)
    print(map_header(game, m))
    rows = game.placements(m)
    near = [float(v) for v in a.near.split(",")] if a.near else None
    picked = []
    for off, r in rows:
        obj, flags, group = struct.unpack_from(">H", r, 0)[0], r[4], r[6]
        d = game.def_of(obj)
        name, dll = game.defs[d] if d >= 0 else ("?", 0)
        if a.name and a.name.lower() not in name.lower():
            continue
        if a.group is not None and not (flags & 0x10 and group == a.group):
            continue
        if a.dll is not None and dll != a.dll:
            continue
        if a.uid and struct.unpack_from(">I", r, 0x14)[0] != int(a.uid, 16):
            continue
        if a.act is not None and not loads_in(r, a.act):
            continue
        if near:
            x, _, z = struct.unpack_from(">3f", r, 8)
            w = game.world(m, x, z) or (x, z)
            if abs(w[0] - near[0]) > (near[2] if len(near) > 2 else 300) or \
                    abs(w[1] - near[1]) > (near[2] if len(near) > 2 else 300):
                continue
        picked.append((off, r, name, dll, flags, group))
    filtered = a.name or a.group is not None or a.dll is not None or a.uid or near or a.act is not None
    if not (filtered or a.all):
        # A summary: each def, how many, in which groups.
        seen = {}
        for _, r, name, dll, flags, group in picked:
            e = seen.setdefault((name, dll), [0, set()])
            e[0] += 1
            if flags & 0x10:
                e[1].add(group)
        print(f"{len(picked)} placements (one each: --all, or --name/--group/--dll/--uid/--near x,z[,r]):")
        print("  " + ", ".join(f"{n}/dll{d}" + (f" x{c}" if c > 1 else "") + (" g" + ",".join(map(str, sorted(gs))) if gs else "")
                           for (n, d), (c, gs) in sorted(seen.items(), key=lambda kv: -kv[1][0])))
        return
    for off, r, *_ in picked:
        print("  " + placement_line(game, m, off, r, True))
    print(f"{len(picked)} of {len(rows)} placements", file=sys.stderr)


def show_def(game, spec):
    m = re.fullmatch(r"dll(\d+)", spec)
    if m:
        found = [i for i, (_, d) in enumerate(game.defs) if d == int(m[1])]
    elif re.fullmatch(r"(?:0x|#)[0-9A-Fa-f]+", spec):
        found = [game.def_of(int(spec.lstrip("#"), 16))]
    else:
        found = [i for i, (n, _) in enumerate(game.defs) if n.lower() == spec.lower()] or \
                [i for i, (n, _) in enumerate(game.defs) if spec.lower() in n.lower()]
    found = [i for i in found if i >= 0]
    if not found:
        sys.exit(f"no object def {spec}")
    if len(found) > 4:
        print(", ".join(f"{game.defs[i][0]} (def {i}, dll{game.defs[i][1]})" for i in found))
        return
    o, _ = g.rom_file(game.rom, OBJECT_TAB_FILE)
    b, _ = g.rom_file(game.rom, OBJECT_BIN_FILE)
    for i in found:
        name, dll = game.defs[i]
        objs = [k for k, v in enumerate(game.object_def) if v == i]
        rec = game.rom[b + struct.unpack_from(">I", game.rom, o + 4 * i)[0]:]
        scale = struct.unpack_from(">f", rec, 4)[0]
        print(f"{name}  def {i}, dll{dll}, object id " + ", ".join(f"#{k:X}" for k in objs) + f", scale {scale:g}")
        where = {}
        for mn in game.maps:
            for _, r in game.placements(mn):
                if game.def_of(struct.unpack_from(">H", r, 0)[0]) == i:
                    where.setdefault(mn, []).append(struct.unpack_from(">I", r, 0x14)[0])
        if where:
            print("  placed: " + "; ".join(f"map {mn} {game.maps[mn]['name']} " + ",".join(f"{u:x}" for u in us[:6])
                                         + (f" (+{len(us) - 6})" if len(us) > 6 else "") for mn, us in where.items()))
        else:
            print("  placed nowhere: made by code (new_placement with its object id)")
        named = [f"+{off:x} {n}" for (of, d, off), n in sorted(game.fields.items(), key=lambda kv: kv[0][2])
                 if of == "def" and d in (None, dll)]
        if named:
            print("  fields: " + ", ".join(named))
        for row in range(0, 0x58, 0x20):
            print(f"  +{row:02x}: " + " ".join(rec[row + k:row + k + 4].hex() for k in range(0, 0x20, 4)))


def show_warps(game, idx, want):
    o, n = g.rom_file(game.rom, WARPS_FILE)
    callers = {}
    for sec, fa, site, key, args in idx["calls"]:
        if key == ("main", WARP_ENTRY) and args.get(4) and args[4][0] == "imm":
            callers.setdefault(args[4][1], []).append(game.func_names.get((sec, fa), f"{fa:X}") + f"+{site - fa:#x}")
    for i in range(n // 16):
        if want is not None and i != want:
            continue
        x, y, z = struct.unpack_from(">3f", game.rom, o + i * 16)
        layer = struct.unpack_from(">i", game.rom, o + i * 16 + 12)[0]
        if not (x or y or z or layer):
            continue
        s = f"{i:2} ({x:.0f}, {y:.0f}, {z:.0f}) layer {layer}"
        ms = game.maps_at(x, z, layer)
        for m in ms:
            c = game.maps[m]["cell"]
            s += f"  map {m} {game.maps[m]['name']} local ({x - c[0] * CELL:.0f}, {y:.0f}, {z - c[1] * CELL:.0f})"
        if not ms:
            s += "  in no map's cells"
        if i in callers:
            s += "  from " + ", ".join(callers[i])
        print(s)


# ---- Finding what to print ----

def parse_func(game, spec, rng=None):
    """(section, start, end, title) for a spec."""
    m = re.fullmatch(r"dll(\d+)\.(\d+)", spec)
    if m:
        n, k = int(m[1]), int(m[2])
        d = game.dlls[n]
        if k >= len(d.exports) or not d.exports[k]:
            sys.exit(f"dll{n} has no export {k}")
        return func_span(game, n, d.exports[k], f" = {game.func_name(('export', n, k))}")
    m = re.fullmatch(r"dll(\d+)(?:_([0-9A-Fa-f]+))?", spec)
    if m:
        n = int(m[1])
        if n not in game.dlls:
            sys.exit(f"no dll{n}")
        if rng:
            a, b = (int(x, 16) for x in rng.split("-"))
            b = min(b, len(game.dlls[n].text_bytes))
            if a >= b:
                sys.exit(f"dll{n}'s text ends at {b:#x}")
            return n, a, b, f"dll{n} {a:#x}-{b:#x}"
        if m[2] is None:
            sys.exit("dis dll<n> needs a function (dll<n>_<offset>, dll<n>.<export>) or a range of its text (0-ffff: all of it)")
        return func_span(game, n, int(m[2], 16), "")
    m = re.fullmatch(r"(?:func_|0x)?([0-9A-Fa-f]{8})", spec)
    if m:
        return func_span(game, "main", int(m[1], 16), "")
    if spec in game.by_name:
        sec, a = game.by_name[spec]
        return func_span(game, sec, a, "")
    for (n, k), e in game.export_names.items():
        if e == spec:
            return parse_func(game, f"dll{n}.{k}")
    sys.exit(f"no function {spec}")


def func_span(game, sec, a, extra):
    f = game.func_at(sec, a)
    if f is None:
        sys.exit(f"no function at {a:#x} in {sec}")
    named = game.addr_names.get(f[0]) if sec == "main" else None
    title = f[2] + (f" {named}" if named else "") + (f" (+{a - f[0]:#x})" if a != f[0] else "") + extra
    if sec != "main":
        d = game.dlls[sec]
        ex = [k for k, e in enumerate(d.exports) if e and e == f[0]]
        if ex and not extra:
            title += " = " + ", ".join(game.func_name(("export", sec, k)) for k in ex)
    return sec, f[0], f[1], title


def parse_target(game, spec):
    """A call target's key for `calls`."""
    m = re.fullmatch(r"dll(\d+)\.(\d+)", spec)
    if m:
        return ("export", int(m[1]), int(m[2]))
    m = re.fullmatch(r"dll(\d+)_([0-9A-Fa-f]+)", spec)
    if m:
        return (int(m[1]), int(m[2], 16))
    m = re.fullmatch(r"(?:func_|0x)?([0-9A-Fa-f]{8})", spec)
    if m:
        return ("main", int(m[1], 16))
    if spec in game.by_name:
        return game.by_name[spec]
    for (n, k), e in game.export_names.items():
        if e == spec:
            return ("export", n, k)
    sys.exit(f"no function {spec}")


def calls(game, idx, spec, want):
    if spec == "bit":
        keys = {("main", BIT_SET), ("main", BIT_GET)}
    else:
        keys = {parse_target(game, spec)}
        # A DLL's own function is also reached as its export.
        k = next(iter(keys))
        if k[0] == "export" and k[1] in game.dlls:
            d = game.dlls[k[1]]
            if k[2] < len(d.exports) and d.exports[k[2]]:
                keys.add((k[1], d.exports[k[2]]))
        elif isinstance(k[0], int):
            d = game.dlls[k[0]]
            keys |= {("export", k[0], i) for i, e in enumerate(d.exports) if e and e == k[1]}
    n = 0
    for sec, fa, site, key, args in idx["calls"]:
        if key not in keys:
            continue
        if want is not None:
            a0 = args.get(4)
            if not (a0 and a0[0] == "imm" and a0[1] == want):
                continue
        fname = game.func_names.get((sec, fa), f"{fa:X}")
        where = f"{fname}+{site - fa:#x}" if sec != "main" else f"{fname}+{site - fa:#x}"
        if key in ((("main", BIT_SET)), ("main", BIT_GET)):
            b = args.get(4)
            bit = f"{b[1]:#x}" if b and b[0] == "imm" else (value_name(game, b) or "?")
            label = game.bit_label(b[1]) if b and b[0] == "imm" else None
            what = f"bit {bit} " + (f"set {value_name(game, args.get(5)) or '?'}" if key == ("main", BIT_SET) else "get")
            print(f"{where:24} {what}" + (f"  ; {label}" if label else ""))
        else:
            print(f"{where:24} {game.func_name(key)}({fmt_args(game, args)})")
        n += 1
    print(f"{n} calls", file=sys.stderr)


def main():
    p = argparse.ArgumentParser(description=__doc__.split("\n\n")[0], formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    d = sub.add_parser("dis", help="one function, densely")
    d.add_argument("func")
    d.add_argument("range", nargs="?")
    d.add_argument("--args", help="what a0, a1, ... are, for a function reached only through a pointer: obj,state,setup,def or - (e.g. obj,-,state)")
    c = sub.add_parser("calls", help="every call to a function, an export, or the bit setter and getter")
    c.add_argument("target")
    c.add_argument("arg", nargs="?", help="only calls whose a0 is this (for bit: the bit)")
    m = sub.add_parser("map", help="a map: its cells, group field and placements")
    m.add_argument("map", help="its number or part of its name")
    m.add_argument("--all", action="store_true", help="every placement, one a line")
    m.add_argument("--name", help="placements whose def's name holds this")
    m.add_argument("--group", type=int)
    m.add_argument("--dll", type=int)
    m.add_argument("--act", type=int, help="placements the object loader places in the map's act n")
    m.add_argument("--uid")
    m.add_argument("--near", help="x,z[,r]: placements within r (300) of a world position")
    o = sub.add_parser("def", help="an object def: its DLL, object ids, where it is placed and its record")
    o.add_argument("spec", help="a name (or part), an object id (0x1A3 or #1A3), or dll<n>")
    w = sub.add_parser("warp", help="the warp table (ROM file 30): place, map and the calls that warp there")
    w.add_argument("index", nargs="?", type=int)
    a = p.parse_args()
    game = Game()
    if a.cmd == "map":
        return show_map(game, a)
    if a.cmd == "def":
        return show_def(game, a.spec)
    idx = load_index(game)
    if a.cmd == "warp":
        return show_warps(game, idx, a.index)
    if a.cmd == "dis":
        sec, s, e, title = parse_func(game, a.func, a.range)
        kinds = {}
        for r, k in zip(ARGS, (a.args or "").split(",")):
            if k in ("obj", "state", "setup", "def"):
                kinds[r] = (k, None if sec == "main" else sec, k)
        print("\n".join(dis(game, idx, sec, s, e, title, kinds)))
    elif a.cmd == "calls":
        calls(game, idx, a.target, int(a.arg, 16) if a.arg else None)


if __name__ == "__main__":
    main()
