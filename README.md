# Dinosaur Planet: AI Slop Recomp

> ⚠️ **This is AI slop.**
> AI agents wrote every line of this project: the code, the fixes, the docs. No human has played or tested it, so
> expect it to break.
>
> **You almost certainly want [Dinosaur Planet: Recompiled](https://github.com/DinosaurPlanetRecomp/dino-recomp)
> instead.** People made it carefully, by hand. This project uses none of its code and isn't part of it. It also uses
> no code, assets or data from [Dinomod Enhanced](https://github.com/EoinODoodles/dinomod-enhanced-recompiled) or the
> [Dinosaur Planet decompilation](https://github.com/zestydevy/dinosaur-planet).
>
> Its developers ask that AI not be used on their work. So use this one only if you want to use AI on a Dinosaur
> Planet recomp yourself, or you're simply thirsty for slop.

Report this one's bugs here, not to Dinosaur Planet: Recompiled. None of it is offered to the projects it builds on,
which don't take AI-written code.

A native PC port of the December 2000 prototype of Rare's *Dinosaur Planet* for the N64, made by static recompilation
([N64Recomp](https://github.com/N64Recomp/N64Recomp) + [N64ModernRuntime](https://github.com/N64Recomp/N64ModernRuntime) +
[RT64](https://github.com/rt64/rt64), with [RecompFrontend](https://github.com/N64Recomp/RecompFrontend)'s launcher
and menus): the game's own MIPS code translated to C and run natively, with RT64 drawing it. Its functions, the main
segment's and those of the 796 DLLs the game loads as it plays, are found in the ROM's code by `tools/gen_syms.py`,
and the DLLs are loaded and unloaded by the game's own loader (`src/game/dlls.cpp`). It is not an emulator.

The repo holds no ROM, no asset from the game and no data extracted from it: you supply your own ROM, and nothing in
git or CI is made from one except the function names, addresses and sizes in `syms/`.

**Status:** not released. The desktop build plays from the title through a new game's name entry and the opening
cutscene into the first level, blended to your display's frame rate from the game's 30 frames a second (Graphics:
Framerate Display by default, Original for the console's). The prototype's progression fixes are to come, written for
this repo.

## The ROM

The prototype's ROM as a big-endian `.z64` of 64 MiB with MD5 `49f7bb346ade39d1915c22e090ffd748` (SHA-1
`cb9760b49198f7b754593a01c8ed642211caacde`). No other build of the game is supported.

## Building

You need that ROM at `roms/dinoplanet.z64`. On Windows: LLVM's clang-cl, CMake 3.31 and Ninja, and a clang
with a MIPS backend plus ld.lld for `patches/`. On Linux: `clang lld libsdl2-dev libgtk-3-dev libfreetype-dev`, CMake
3.31 and Ninja.

From the repo's root in a bash shell:

```sh
git -c core.longpaths=true submodule update --init --recursive   # paths past Windows' 260 characters
bash tools/recompile.sh -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl   # the game's code as C
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl \
      -DPATCHES_C_COMPILER=<MIPS clang> -DPATCHES_LD=<ld.lld>
ninja -C build DinosaurPlanet
```

With [ccache](https://ccache.dev) on `PATH` every compile goes through it, so a new build dir or another checkout
of the same code compiles only what changed since the last build on that computer (CMakeLists.txt says how).

On Linux the same lines take `clang` and `clang++` in place of `clang-cl`, and the system's `clang` and `ld.lld` for
`patches/`. `tools/recompile.sh` builds N64Recomp from the submodule the first time, then translates the game. Run
`build/DinosaurPlanet` (`.exe` on Windows) from the repo's root, pick the ROM with Load ROM, then Start Game.
`tools/release.sh`'s header says how a release is built.

`syms/` is written from the ROM by `tools/gen_syms.py` (its header says how; it needs spimdisasm and n64sym's
signatures). Only a change to how it finds functions needs it again.

## License

GPL-3.0 (`LICENSE`), as the recompilations it follows.
