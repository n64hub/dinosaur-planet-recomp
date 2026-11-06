# Dinosaur Planet (prototype), recompiled

> **Made with AI.** Almost all of the code, documentation and tests in this repo were written by AI agents
> (Claude), directed and play-tested by a person. None of it is offered to the upstream projects it builds on, which
> don't take AI-written code.

This is not Dinosaur Planet: Recompiled and has none of its code or Dinomod's: report this one's bugs here, not there.

A native PC port of the December 2000 prototype of Rare's *Dinosaur Planet* for the N64, made by static recompilation
([N64Recomp](https://github.com/N64Recomp/N64Recomp) + [N64ModernRuntime](https://github.com/N64Recomp/N64ModernRuntime) +
[RT64](https://github.com/rt64/rt64), with [RecompFrontend](https://github.com/N64Recomp/RecompFrontend)'s launcher
and menus): the game's own MIPS code translated to C and run natively, with RT64 drawing it. Its functions, the main
segment's and those of the 796 DLLs the game loads as it plays, are found in the ROM's code by `tools/gen_syms.py`,
and the DLLs are loaded and unloaded by the game's own loader (`src/game/dlls.cpp`). It is not an emulator.

The repo holds no ROM, no asset from the game and no data extracted from it: you supply your own ROM, and nothing in
git or CI is made from one except the function names, addresses and sizes in `syms/`.

**Status:** not released. The desktop build plays from the title through a new game's name entry and the opening
cutscene into the first level. The prototype's progression fixes are to come, written for this repo.

## The ROM

The prototype's ROM as a big-endian `.z64` of 64 MiB with MD5 `49f7bb346ade39d1915c22e090ffd748` (SHA-1
`cb9760b49198f7b754593a01c8ed642211caacde`). No other build of the game is supported.

## Options

Each option a modern port adds has a value that plays as the console does. The defaults are the modern ones.

- **Aspect Ratio** (Graphics): Expand widens the world to the window, and a cutscene's bars and the screen's clear
  reach its sides. Original is the console's 4:3.
- **HUD Placement** (Graphics): the minimap and the panels at the picture's sides move out to 16:9's edges with
  Clamp16x9 and the window's with Full. Original keeps them where the console puts them. Dialogue and subtitles stay
  centred.
- **Free Camera** (General): the right stick and the mouse turn the camera around the player, who walks by the turned
  view. A turned view keeps to the player's side of the walls, and the D-pad plays the C buttons in the stick's place.
  Original is the console's camera, which follows by itself, with the right stick on the C buttons.
- **Mouse Sensitivity** (General): how far the mouse turns the camera, with left click B and right click Z. 0% is no
  mouse, as on a console, and leaves the pointer free.
- **Invert Camera Y** (General): moving the mouse or the right stick up looks down.

Controls are rebound in Controls. A cutscene's camera and the first person view are the game's own in either mode.

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
