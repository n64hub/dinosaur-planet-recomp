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
  `dll<number>_<offset>`. A name worked out since goes into `syms/names.toml`, and keeps it: a function's
  (`[[func]]`, which renames it in `syms/`), and the names only the tools print, of addresses, exports, bits and the
  fields of an object, its state and its placement (the file's header says which).
- `tools/dino.py`: the game's code read in those names, in place of raw MIPS (its header says what each command
  prints). Read a function with `dis`, find who calls something or sets a bit with `calls`, and a level's objects,
  an object's def and the warp table with `map`, `def` and `warp`, and the scenes that show a text with `text`.
- `us.toml`: N64Recomp's configuration, with the game's hooks and instruction patches; `tools/recompile.sh` writes
  `RecompiledFuncs/` with it, and the audio microcode with `aspMain.us.toml`.
- `patches/` and `patches.toml`: MIPS C recompiled into PatchesLib, where a patch to the game's code goes
  (`patches/Makefile`'s header says how they build). A patch declares what it touches by address.
- `src/`: RecompFrontend's main (`src/main/main.cpp`, with the GameEntry), the code sections it gives the runtime
  (`src/main/register_overlays.cpp`), the DLLs' loading (`src/game/dlls.cpp`), the natives the patches call
  (`src/game/recomp_api.cpp`), the libultra functions the runtime lacks (`src/game/libultra.cpp`), the HUD's and the
  clear's widescreen placement (`src/game/widescreen.cpp`), the 3D picture's top and bottom rows
  (`src/game/view_rows.cpp`), the player's cell kept loaded beside the camera's (`src/game/streaming.cpp`), the
  player kept out of the ground where a wall has no line (`src/game/ground.cpp`), the
  free camera and mouse (`src/game/camera.cpp`), the Krazoa shrines' ways in and steps (`src/game/krazoa.cpp`), the
  Kamerian Dragon's end
  (`src/game/dragon_rock.cpp`), Drakor's end (`src/game/drakor.cpp`), CloudRunner Fortress's treasure lift
  and baby CloudRunners (`src/game/cloudrunner.cpp`), wind bridge (`src/game/cloudrunner_barrels.cpp`), BOSS
  TRex's setting (`src/game/boss_trex.cpp`), Walled City's way in and its King (`src/game/walled_city.cpp`), the
  settings, and on Windows a stack trace on a crash (`src/main/crash_trace.cpp`).
- `lib/`: upstream `rt64`, `N64ModernRuntime` and `RecompFrontend` at the commits N64Recomp-browser's web forks are
  built on, and `N64Recomp-browser` at a release tag. The web support every recomp's page shares is developed and
  documented there; `tools/build_n64recomp.sh` and `CMakeLists.txt` apply its `patches/` to the other three, and
  `CMakeLists.txt` then this game's own changes to them (`runtime-patches/`, its README lists them).
- `docs/web.md`: the web page.
- `docs/restored-content.md`: every fix sorted into crashes, what the ROM shows and the choices made where it says
  nothing, each choice with what it touches and what else the ROM offers. A restoration's PR adds its lines there,
  and a choice is checked against it first: one the ROM answers is a clash, fixed the ROM's way.
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
- **Interpolation.** Eight hooks in `us.toml` call `src/game/interpolation.cpp`, which writes RT64's extended commands
  into the list the game is building: its header says how. Each camera has a group that a cut changes, a frame cut in
  puts the world in a group of its own, each object's model is a group by the object, and each map block a group by
  the block and its cell, so a step never pairs a block with its neighbour. The world's origin steps by
  640 units as the player walks, which isn't a cut: `RT64_INTERPOLATE_CAMERA_EYE` and `RT64_UNEVEN_FRAME_RATES`, on in
  `CMakeLists.txt`, cancel that step and keep blending through frames of 3 VIs, and a frame whose origin steps has the
  world and each model interpolate their translations, which RT64 by itself drops for a step that size.
- **Krazoa shrines.** Eight tests, each giving a spirit the player carries as a mask of its own to Warlock Mountain
  (map 11), whose act (bit 0xE4) each win moves on. The prototype reaches three from the world and has no step at
  Warlock Mountain for three spirits: the General tab's Krazoa Shrines setting, Restored by default, finishes them out
  of what the ROM holds, and Original leaves them as the console plays them (`src/game/krazoa.cpp`'s header has the
  order, the bits and the evidence). Its Krazoa Tablets setting, Restored by default, does the same for Cape Claw's
  tablets: HighTop handing over the first in Discovery Falls, the third tablet and the way on to Krazoa Palace.
- **The Kamerian Dragon.** Under Dragon Rock (map 53) the fight is built and can't be won: its health runs out and
  nothing reads it, and nothing gives the fourth SpellStone (0x7A8) that the Desert Force Point's other place takes.
  The General tab's Kamerian Dragon setting, Restored by default, has it die there and leave that stone, and Original
  leaves the fight endless (`src/game/dragon_rock.cpp`'s header has the evidence).
- **The Desert Force Point.** Golden Plains' level control sets the second SpellStone's place (0x5F3) on every load,
  and no surface joins the force point's floor (map 21) to the duct that reaches the place. The General tab's Desert
  Force Point setting, Restored by default, leaves the place to the player and stands a second DFPLift1 in the
  corridor behind the round door, under one of the duct's stubs, and Original leaves both as they were
  (`src/game/force_point.cpp`'s header has the evidence).
- **Drakor's end.** The last fight (map 44) is won in the ROM as it is: a hit lets the Kamerian Diamond go, and the
  player who reaches it in the room below plays Drakor's fall, sequence 1 (0x7AE). Nothing followed it; the General
  tab's Ending setting, Restored by default, goes back to the title at its end as the game's own menus do, and Original
  starts the fight again (`src/game/drakor.cpp`'s header has the evidence).
- **CloudRunner Fortress's treasure lift.** Past the blasted wall (map 12) a wind lift goes down to the treasure caves
  and the way to the race, but its direction bit (0x8C8) is flipped only by two levers at the bottom, so from the top
  it only blows up. The General tab's Treasure Lift setting, Restored by default, lowers a player who steps in from
  the top, and Original leaves it as it was (`src/game/cloudrunner.cpp`'s header has the evidence).
- **CloudRunner Fortress's baby CloudRunners.** A baby waiting to be found (CFCloudBaby, dll373) comes out when Kyte's
  export 24 answers yes: she is on a find node, the curves beside each baby that nothing sends her to. The General
  tab's Baby CloudRunners setting, Restored by default, sends her there (her curve group, 0x46E) as Krystal comes near
  a waiting baby, which comes out once she steps back from its call, and Original leaves them hidden (the same
  header). Kyte flies only along curves joined to the one her path started from, and the courtyard's deck, where she
  is shut in, has six that join no others: once she is freed (0x43) her path starts again off the deck, always on, so
  she can fly to the babies and the gold perch (0x8D1).
- **BOSS TRex.** The TRex's defeat loads the front end with no SpellStone. The General tab's BOSS TRex setting, Restored
  by default, gives the third SpellStone as won (0x82E) and the way back to Walled City in its place, and Original goes
  to the title as the prototype did. The fixes that let the fight be won at all (the floor switches' pads, the TRex's
  corners and facing, the door behind the player) are always on (`patches/boss_trex.c` and us.toml beside them have the
  evidence, docs/story-points.md how the fight plays).
- **Walled City.** The task list has the Queen EarthWalker, a RedEye chase for ten teeth that open the city's gate and
  the King activating the third SpellStone, and the ROM built none of it: the Queen and the chase have defs and stand
  nowhere, the gate is open and the stone is active once won. The General tab's Walled City setting, Restored by
  default, places them and has the King activate the stone (0x82E cleared, the Volcano Force Point to act 2, whose level
  control hands it over as 0x83B), and Original activates it as it is won and leaves the city as the prototype did
  (`src/game/walled_city.cpp`'s header has the evidence). Tricky's flame, which angers the RedEye, is the one DarkIce
  Mines' SnowHorn teaches him (docs/story-points.md).
- **CloudRunner Fortress's courtyard roof.** The roof (CFBlastedRoof) blows at the third barrel blown under it, and
  the barrels stand west of the treasure shaft, whose wind drops a player carrying one: the walking states skip the
  check that puts her on the wind while she carries. The General tab's Wind Bridge setting, Restored by default, puts
  her on it, and Original leaves it as it was (`src/game/cloudrunner_barrels.cpp`'s header has the evidence).
- **Krazoa Palace.** Map 3's rooms are built, but of its objects only two Transporters are placed: the wings' doors,
  consoles and text blocks, the last door and the red Krazoa have defs and stand nowhere, and nothing leads on to
  Drakor (map 44). The General tab's Krazoa Palace setting, Restored by default, places them, chains the four wings
  by saved bits nothing else uses (0x6A7-0x6AA) and has the red Krazoa send the player to Drakor, and Original
  leaves the rooms empty (`src/game/krazoa.cpp`'s header has the layout and the evidence).
- **The third SpellStone.** A map's act, a story value, picks which of its objects load, and the Volcano Force Point
  and Walled City never leave act 1 in the ROM, so the third SpellStone has nowhere to go and Walled City's Sun and
  Moon Stones never show; nothing gives the moonseeds Krystal plants in Moon Mountain Pass either. The General tab's
  Third SpellStone setting, Restored by default, moves both maps' acts with the story and, once the stone is placed,
  lays four moonseeds in the cave under SwapStone Hollow for Sabre to find, and Original leaves them as they were (`src/game/acts.cpp` and `src/game/moonseeds.cpp` have the
  evidence, docs/story-points.md, The maps' acts, how they move).
- **Input.** The game is one player's: port 1 has a controller with a Rumble Pak (`main.cpp`).
- **Testing.** `--boot` starts the game from the launcher's first frame once the ROM is stored (`--boot-after
  <seconds>` that long after it), `--window <w>x<h>` sizes the window, `--input <file>` plays port 1 from a script of
  timed button states, which can wait for a bit the game sets, an enemy's place or act, and take whichever of two
  ways an enemy comes, and walk the player to a place, an object or one of its hit spheres (`main.cpp` says the
  format), and on Windows `--stacks <seconds>` prints every thread's stack that long after start, for unattended runs
  and hangs, and `--watch <address>,...` stops on each write to those words (four at most) of the
  game's memory and prints the value and the stack that wrote it (`crash_trace.cpp` says the form: `**8008C994+4`
  follows two pointers), for what writes over something. `--warp <index>@<seconds>` warps the player by the game's own
  warp table, `DINO_LOG_DLLS=1` prints each DLL as it loads and unloads, `DINO_LOG_CUTS=1` each camera cut and object
  jump `src/game/interpolation.cpp` finds, with each origin step, `DINO_LOG_KRAZOA=1` each object
  `src/game/krazoa.cpp` places and any of them freed while its map is loaded, each spirit it gives and has taken
  back and each event of Golden Plains' shrine,
  `DINO_LOG_ACTS=1` each map's act `src/game/acts.cpp` moves or leaves, `DINO_LOG_MOONSEEDS=1` each moonseed
  `src/game/moonseeds.cpp` places,
  `DINO_LOG_DRAKOR=1` the end of Drakor's fall, `DINO_LOG_DRAGON=1` the Kamerian Dragon's state, health and end,
  `DINO_LOG_FORCE_POINT=1` the Desert Force Point's lift placed and its place's group turned on,
  `DINO_LOG_WALLED_CITY=1` each object `src/game/walled_city.cpp` places and each step of the chase, the gate and the
  stone, `DINO_LOG_GROUND=1` each step into the ground `src/game/ground.cpp` takes back, `DINO_LOG_EFFECTS=1`
  each effect drawn after its object was freed,
  `DINO_DRAGON_HEALTH=<n>` starts its fight with n health, and `DINO_CHECK_HEAP=1` each block of the game's heaps
  written past its end, with the tag of what asked for it (`src/game/heap.cpp`). On Windows `tools/grab_window.ps1`
  grabs the window as a run plays (its header says how). A grab made another way on a display scaled past 100% can
  hold only the picture's top left, without the HUD's right and bottom: the C menus draw.
- **Story points.** `--story <point>[@<seconds>][,<warp>]` starts a run from a named point of the story, with the
  game's bits, the character and the place set as the save would have them, in memory and with nothing written to
  Flash: `--boot --story before-wg-shrine` is in a Krazoa shrine with Krystal alive. `--place
  <x>,<y>,<z>[,<layer>[,<facing>]]` starts it somewhere the warp table has no entry, `--bits
  <hex>[=<value>][@<seconds>],...` sets more bits, at the start or later (a shrine's own bits are cleared as its map
  starts, so they are set a while after it), and `--report` prints a line a second with the map, the position, the
  health and the magic (and with `DINO_REPORT_OBJECT=<object id hex>` where each object of that id is, its act, its
  use flags and where A uses it); `--watch-bits <hex>,...|all` prints each bit the game sets as it plays.
  `docs/story-points.md` has the points, each bit's evidence and the warp table's map: each index, its area and what
  it needs. None of it is in a player's menus.
