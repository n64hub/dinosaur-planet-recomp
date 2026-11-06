"""Writes roms/dinoplanet.recomp.z64, the copy of the ROM N64Recomp reads the game's code from (us.toml), from
roms/dinoplanet.z64.

The two differ only in the DLLs' $gp setups (tools/gen_syms.py's header says what a DLL is): each is a lui and an ori
the game's loader fills in with the halves of the address of the DLL's GOT, and N64Recomp fills them in with its
HI16/LO16 relocations (syms/) when the runtime has put the DLL where the game loaded it. The ori takes the low half as
it is, where the high half N64Recomp's HI16 gives is rounded up for an addiu, so the address would be 0x10000 too high
whenever its low half is 0x8000 or more. Here each of those ori is an addiu, which adds the low half signed, and the
pair then gives the GOT's address exactly. The game itself still runs the ROM's code only through the recompilation, so
nothing else changes; the program reads the player's own ROM.

Usage: py -3 -I tools/recomp_rom.py   (tools/recompile.sh runs it)
"""
import hashlib
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_syms  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
OUT_PATH = ROOT / "roms" / "dinoplanet.recomp.z64"


def main():
    rom = bytearray(gen_syms.ROM_PATH.read_bytes())
    if hashlib.md5(rom).hexdigest() != gen_syms.ROM_MD5:
        sys.exit(f"{gen_syms.ROM_PATH}: not Dinosaur Planet's December 2000 prototype, MD5 {gen_syms.ROM_MD5}")
    count = 0
    for d in gen_syms.dlls(bytes(rom)):
        for g in d.gp_sets:
            at = d.text_rom + g + 4
            w = struct.unpack_from(">I", rom, at)[0]
            if w >> 26 != 0x0D:  # ori
                sys.exit(f"dll{d.number} +0x{g:X}: no ori after the lui")
            struct.pack_into(">I", rom, at, (0x09 << 26) | (w & 0x03FFFFFF))  # addiu, same registers and immediate
            count += 1
    if not OUT_PATH.exists() or OUT_PATH.read_bytes() != rom:
        OUT_PATH.write_bytes(rom)
    print(f"{OUT_PATH.relative_to(ROOT)}: {count} $gp setups made lui/addiu")


if __name__ == "__main__":
    main()
