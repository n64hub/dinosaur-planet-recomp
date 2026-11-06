# CLAUDE.md

A static recompilation of the December 2000 prototype of *Dinosaur Planet* for the N64 with N64Recomp,
N64ModernRuntime, RecompFrontend and RT64. Its functions are found in the ROM's code, with no decompilation's names or
code, and its progression fixes are written for this repo. Almost all of it is written by AI agents, so this file is
what the next one needs: the rules, where things are, and the facts about the game the code can't say on its own.

## What lives where

This repo is the game: its recompilation, the hooks and patches over it, the desktop build and its web page.

- `syms/`: the symbols N64Recomp reads, written from the ROM by `tools/gen_syms.py` (its header says how), never
  edited by hand: the main segment and each DLL as a section, libultra's functions named by n64sym's signatures or by
  hand (`MANUAL_NAMES`, `DLL_MANUAL_NAMES`, each with its evidence) and the rest `func_<address>` or
  `dll<number>_<offset>`. A name worked out since goes into `syms/names.toml`, and keeps it.
- `us.toml`: N64Recomp's configuration, with the game's hooks and instruction patches; `tools/recompile.sh` writes
  `RecompiledFuncs/` with it, and the audio microcode with `aspMain.us.toml`.
- `patches/` and `patches.toml`: MIPS C recompiled into PatchesLib, where a patch to the game's code goes
  (`patches/Makefile`'s header says how they build). A patch declares what it touches by address.
- `src/`: RecompFrontend's main (`src/main/main.cpp`, with the GameEntry), the code sections it gives the runtime
  (`src/main/register_overlays.cpp`), the DLLs' loading (`src/game/dlls.cpp`), the natives the patches call
  (`src/game/recomp_api.cpp`), the libultra functions the runtime lacks (`src/game/libultra.cpp`), the HUD's and the
  clear's widescreen placement (`src/game/widescreen.cpp`), the free camera and mouse (`src/game/camera.cpp`), the
  Krazoa shrines' ways in and steps (`src/game/krazoa.cpp`), the settings, and on Windows a stack trace on a crash
  (`src/main/crash_trace.cpp`).
- `lib/`: upstream `rt64`, `N64ModernRuntime` and `RecompFrontend` at the commits N64Recomp-browser's web forks are
  built on, and `N64Recomp-browser` at a release tag. The web support every recomp's page shares is developed and
  documented there; `tools/build_n64recomp.sh` and `CMakeLists.txt` apply its `patches/` to the other three, and
  `CMakeLists.txt` then this game's own changes to them (`runtime-patches/`, its README lists them).
- `docs/web.md`: the web page.
- Comments beside the code, for how and why: each file's header. Read the header of the file you change and update
  it in the same commit.

## Rules

- No code from Dinosaur Planet: Recompiled (dino-recomp) or Dinomod: not its code, its asset diffs or its tables of
  DLL edits, and nothing looked up there, their docs, issues and Discord included. Nothing is offered to either or
  posted where they are. A fact this repo uses carries its own evidence from the ROM, in the comment beside it.
- Nothing from the Dinosaur Planet decompilation (zestydevy/dinosaur-planet) either: not its code, its symbols or
  names, its headers or its notes on the DLLs and objects, and nothing looked up there. Every function, name and fix
  is found in the ROM's own code.
- The project is "Dinosaur Planet: AI Slop Recomp" (`dino::program_name`, the window and the launcher), and the README
  opens with its banner saying it is AI slop and pointing at Dinosaur Planet: Recompiled. Keep both true.
- The README's Progression fixes lists the prototype's blockers being fixed and those fixed: a PR that lands a
  progression fix moves its line there from under way to fixed, in words a player knows the place by.
- No ROMs, ROM-derived assets or extracted game data in git, CI or PRs (`tools/ci/check_repo.py` checks). Nothing made
  from the ROM is committed but `syms/`'s names, addresses and sizes. The icon (`icons/icon.svg`, the launcher's
  thumbnail and the program's icon by `tools/icon.py`) is drawn for the project from plain shapes: no logo, character or
  picture from the game.
- The desktop releases are built where the ROM is, by `tools/release.sh` (its header says how), and published to
  GitHub Releases with their notes in `releases/`.
- The scope is the game as it plays, with the fixes a player needs to reach its end, plus what a web page needs (RT64
  interpolation to the display's rate, resolution Auto), plus modern enhancements: each an option with an Original
  value that plays as the console does.
- Write for a stranger, in the present tense: nothing about a contributor's machine, servers, paths or accounts, and
  nothing about where anyone keeps ROMs.
- GPL-3.0, as the recompilations it follows.

## How it fits together

- **The code.** The main segment (ROM 0x1000, VRAM 0x80000400, its code to 0x80089750), stored raw, and 796 DLLs in
  ROM file 70, which the game loads into memory it allocates and relocates itself. Each DLL is a section of its own at
  a made-up address (`tools/gen_syms.py`'s header says how one is read), and hooks on the game's loader and unloader
  (`us.toml`) map it where the game put it and take it out before it is freed (`src/game/dlls.cpp`).
- **Position-independent DLLs.** A DLL reaches the main segment through its GOT, filled from the import table (ROM
  file 72), and its own code through GOT pages plus a low half. It sets `$gp` with a lui and an ori, which N64Recomp
  adds signed (`runtime-patches/`).
- **The skinning routine.** Hand-written assembly at 0x8001B4F0 calls parts of itself as subroutines; it is one
  function to N64Recomp, which recompiles those calls (`runtime-patches/`), and its one jump through a table goes to
  that table's only entry (`us.toml`).
- **Development hardware.** The game's main thread first writes to a register at 0xBC000C02 of the hardware the
  prototype was made on, which a console doesn't have: `us.toml` stubs that function. It also reads that hardware's
  signature at 0xBC000000 in three places and the cartridge straight from its address once at boot, and the audio reads
  AI_LEN itself; `us.toml` gives each the value the hardware would.
- **Floating point.** The hand-written math (matrices, vectors, splines, skinning) uses the odd float registers as
  registers of their own, as with Status.FR set, where the compiled code expects FR clear; `us.toml` runs each of
  those functions in FR set. Without it every matrix of the world is 0 and only the 2D screens draw.
- **Fall-through entries.** A few functions start 4 bytes before another and run into it (the cosine into the sine):
  `tools/gen_syms.py` gives each the size of both.
- **libultra.** Some is named by hand (`MANUAL_NAMES`) where the signatures miss it and the runtime replaces it. The
  internals N64Recomp drops are reached only from code that never runs here (the debugger, the VI manager's thread,
  the Controller Pak), and stop the game naming themselves if one is (`src/game/libultra.cpp`).
- **Saves.** On Flash. dll29 is the save. dll31 drives the chip through the PI with its own copy of libultra's Flash
  functions; named (`DLL_MANUAL_NAMES`), they are the runtime's Flash. The main segment's copy is never called.
- **Microcodes.** Audio: `n_aspMain`, recompiled by RSPRecomp from `aspMain.us.toml`, with the second part it
  loads over its own text as an overlay slot. librecomp's vector unit uses
  SSE4.1, so `RecompiledFuncs` builds with `-march=nehalem` on x86-64.
- **Interpolation.** Four hooks in `us.toml` call `src/game/interpolation.cpp`, which writes RT64's extended commands
  into the list the game is building: its header says how. Each camera has a group that a cut changes, a frame cut in
  puts the world in a group of its own, and each object's model is a group by the object. The world's origin steps by
  640 units as the player walks, which isn't a cut: `RT64_INTERPOLATE_CAMERA_EYE` and `RT64_UNEVEN_FRAME_RATES`, on in
  `CMakeLists.txt`, cancel that step and keep blending through frames of 3 VIs, and a frame whose origin steps has the
  world and each model interpolate their translations, which RT64 by itself drops for a step that size.
- **Krazoa shrines.** Eight tests, each giving a spirit the player carries as a mask of its own to Warlock Mountain
  (map 11), whose act (bit 0xE4) each win moves on. The prototype reaches three from the world and has no step at
  Warlock Mountain for three spirits: the General tab's Krazoa Shrines setting, Finished by default, finishes them out
  of what the ROM holds, and Original leaves them as the console plays them (`src/game/krazoa.cpp`'s header has the
  order, the bits and the evidence).
- **Input.** The game is one player's: port 1 has a controller with a Rumble Pak (`main.cpp`).
- **Testing.** `--boot` starts the game from the launcher's first frame once the ROM is stored (`--boot-after
  <seconds>` that long after it), `--window <w>x<h>` sizes the window, `--input <file>` plays port 1 from a script of
  timed button states (`main.cpp` says the format), and on Windows `--stacks <seconds>` prints every thread's stack
  that long after start, for unattended runs and hangs, and `--watch <address>,...` stops on each write to those
  words (four at most) of the game's memory and prints the value and the stack that wrote it (`crash_trace.cpp` says
  the form: `**8008C994+4` follows two pointers), for what writes over something. `--warp <index>@<seconds>` warps
  the player by the game's own warp table, `DINO_LOG_DLLS=1` prints each DLL as it loads and unloads,
  `DINO_LOG_CUTS=1` each camera cut and object jump `src/game/interpolation.cpp` finds, with each origin step,
  `DINO_LOG_KRAZOA=1` each object `src/game/krazoa.cpp` places, each spirit it gives and has taken back and each
  event of Golden Plains' shrine, and `DINO_CHECK_HEAP=1` each block of the game's heaps written past its end, with
  the tag of what asked for it (`src/game/heap.cpp`).
- **Story points.** `--story <point>[@<seconds>][,<warp>]` starts a run from a named point of the story, with the game's
  bits, the character and the place set as the save would have them, in memory and with nothing written to Flash:
  `--boot --story before-wg-shrine` is in a Krazoa shrine with Krystal alive. `--place <x>,<y>,<z>[,<layer>[,<facing>]]`
  starts it somewhere the warp table has no entry, `--bits <hex>[=<value>][@<seconds>],...` sets more bits, at the start
  or later (a shrine's own bits are cleared as its map starts, so they are set a while after it), and `--report` prints
  a line a second with the map, the position and the health; `--watch-bits <hex>,...|all` prints each bit the game sets
  as it plays. `docs/story-points.md` has the points, each bit's evidence and the warp table's map: each index, its area
  and what it needs. None of it is in a player's menus.
