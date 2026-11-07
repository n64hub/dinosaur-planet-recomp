# Dinosaur Planet: AI Slop Recomp

> ⚠️ **This is AI slop.**
> AI agents wrote every line of this project: the code, the fixes, the docs. It aims to be the whole prototype,
> playable to its end, but no human has played or tested it, so a fix may fall short where nobody has looked.
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

This project only accepts AI contributions: code, fixes and docs written by AI agents. Work written by hand belongs in
Dinosaur Planet: Recompiled instead.

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
Framerate Display by default, Original for the console's). The prototype's progression fixes are under way
(Progression fixes).

## Progression fixes

The December 2000 prototype is unfinished: as it is on the ROM, places in it crash, softlock or have no way on, so it
can't be played to its end. This recomp aims to make it completely playable and restored: every place that stops a
player fixed, and the story's unfinished parts finished from what the game's own code and data already hold. Each fix
is an attempt, written by AI agents from the game's own code (the patches in `patches/` and the hooks in `us.toml`),
and none has been played by a human, so report one that falls short.

Fixed:

- Crashes in the game's own code: the Lunaimars of Moon Mountain Pass and Cape Claw's sand worm with no Kyte beside
  Krystal, the Swap Store's item beams as the player walks in, a crash after a long while in Cape Claw, where more
  sounds at once than an audio frame's command list holds wrote past its end, a small basket or crate carried out
  of the place it was found, which the game freed in the player's hands, Dragon Rock's fight with the Skeetlas,
  which after a minute or two drew more than the game had made room for and wrote over the models, Dragon Rock's
  top at its east end, where more of the level is loaded at once than the game kept room for, a swing
  pressed on the very frame the player comes back after a death, before the staff or sword was made, and Golden
  Plains' portable hole spell, whose pickup made a new player every frame until the game ran out of memory.
- Cape Claw's SandEels: a beaten eel that loads again, as one does when the player comes back to the sand worm,
  started its sound every frame until no other sound in Cape Claw could play.
- SwapStone Hollow's white mushrooms: one of the three down under the hollow can get stuck where it stands, and caught
  there it vanished without counting, so only nine of the ten for the Queen Earthwalker could be brought to her.
- The Krazoa shrines (Krazoa Shrines on Finished): all eight finished, a way into the five that have none, and their
  spirits taken to Warlock Mountain, and Walled City's shell game, which a player who stood still won as its answer
  time ran out. Two of the fixes are always on: Cape Claw's test, which never ended, and Golden Plains' fly baddie,
  whose every shot made a second player who took the camera.
- SwapStone Hollow: Tricky's run to his mother, the Queen Earthwalker, never started, and with no Tricky beside Sabre
  it would have crashed. And the SwapStone could stop answering in its own corner of the hollow, and answer only
  outside it, after a trip through the Swap Store or the SwapStone's warp.
- Cape Claw's Krazoa tablets (Krazoa Tablets on Restored): the third tablet, which the prototype never placed, is
  found where the sand worm fell, and the tablet room's last puzzle, which led nowhere, opens a Transporter to Krazoa
  Palace.
- The Kamerian Dragon under Dragon Rock (Kamerian Dragon on Restored): its health ran out and the fight went on, and
  the fourth SpellStone, which the Desert Force Point's second place takes, was nowhere. It now dies once its health
  runs out and leaves the fourth SpellStone in Krystal's hands, and stays gone. Nothing led to its room: the room is
  the Skeetla arena at the bottom of Dragon Rock on the level's next layer, and the arena, once won, now leads into
  it, and the SpellStone, once picked up, back out.
- Drakor's end (Ending on Restored): the last fight is won as the ROM has it, by a hit with the staff and a walk to
  the Kamerian Diamond in the room below, and the end of Drakor's fall now goes back to the title with the save as it
  was. The prototype started the fight again after his fall, so the game never ended.
- The Desert Force Point's second SpellStone (Desert Force Point on Restored): Golden Plains filled its place each time
  it loaded, so the stone stayed carried and SwapStone Circle refused the swap back to Sabre, and nothing led up from
  the force point's floor to the duct that reaches the place. Golden Plains now leaves the place to the player, and a
  second of the force point's own lifts stands in the corridor behind the round door, under the duct.
- CloudRunner Fortress's treasure lift (Treasure Lift on Restored): past the blasted wall the wind lift only ever
  blew up for a player who came from the top, holding her at the corridor's height, so the treasure caves below and
  the way to the race couldn't be reached. Stepping in from the top now lowers her to the bottom, where the levers
  turn it back for the way up.
- Picking up an item, a key, a crystal, a tooth or a SpellStone: the item's scene made a second Sabre and never
  ended, and could crash.
- Walled City: standing on either pressure switch crashed the game, and nothing could hit the sun and moon beacons,
  so the city's slab door never opened.
- BOSS TRex: the door behind Sabre shut the player out of the arena as its intro ended, standing on a floor switch never
  charged it, so no laser wall fired and the TRex couldn't be hurt, after its first hit the TRex circled its corners in
  place and never charged again, and a charge that ran on round a corner left it facing sideways, so its stunned head
  could lie out of reach.
- BOSS TRex's SpellStone (BOSS TRex on Restored): beating the TRex went back to the title with no SpellStone. The
  third SpellStone is now carried out, through the arena's own way back to Walled City.
- Krazoa Palace (Krazoa Palace on Restored): the prototype built its rooms and left them empty, and nothing past
  Cape Claw led on to Drakor. Its four wings now each have a door, a console and a text block, played in turn, fire,
  earth, water and magic, and the last door they open leads to the red Krazoa, who sends the player on to Drakor.

Under way:

- Crashes in the game's own code still to find: the skeletons' hits.
- The rest of the story, played from a new game to its ending, and whatever stops a player there.
- Dragon Rock's bottom: the way from the portal's landing, or from the top's east end, to the Skeetla arena, and the
  arena's win, which the way into the Kamerian Dragon's room waits for.

## The ROM

The prototype's ROM as a big-endian `.z64` of 64 MiB with MD5 `49f7bb346ade39d1915c22e090ffd748` (SHA-1
`cb9760b49198f7b754593a01c8ed642211caacde`). No other build of the game is supported.

## Options

Each option a modern port adds has a value that plays as the console does. The defaults are the modern ones.

- **Aspect Ratio** (Graphics): Expand widens the world to the window, and a cutscene's bars and the screen's clear
  reach its sides. Original is the console's 4:3.
- **HUD Placement** (Graphics): the minimap and the panels at the picture's sides move out to 16:9's edges with
  16:9 and the window's with Expand. Original keeps them where the console puts them. Dialogue and subtitles stay
  centred.
- **Free Camera** (General): the right stick and the mouse turn the camera around the player, who walks by the turned
  view. A turned view keeps to the player's side of the walls, and the D-pad plays the C buttons in the stick's place.
  Original is the console's camera, which follows by itself, with the right stick on the C buttons.
- **Mouse Sensitivity** (General): how far the mouse turns the camera, with left click B and right click Z. 0% is no
  mouse, as on a console, and leaves the pointer free.
- **Invert Camera Y** (General): moving the mouse or the right stick up looks down.
- **Krazoa Shrines** (General): Finished makes all eight Krazoa shrines reachable and playable, and gives each spirit
  its release at Warlock Mountain, from what the prototype's own maps and code hold. Original is the prototype as it
  was left: three shrines are reached from the world, and three spirits have nowhere to go.
- **Krazoa Tablets** (General): Restored places Cape Claw's third Krazoa tablet where the sand worm fell, and has the
  tablet room's last puzzle open a Transporter to Krazoa Palace. Original is the prototype as it was left: the third
  tablet is nowhere, and the puzzle's end leads nowhere.
- **Kamerian Dragon** (General): Restored has the Skeetla arena at the bottom of Dragon Rock, once won, lead into the
  Kamerian Dragon's room, and the dragon die once its health runs out and leave the fourth SpellStone for the Desert
  Force Point, which leads back out. Original is the prototype as it was left: nothing leads to the room, the fight
  never ends, and the fourth SpellStone is nowhere.
- **Desert Force Point** (General): Restored leaves the second SpellStone's place empty until the player places the
  stone, and adds a lift up to the duct that leads there, in the corridor behind the round door. Original is the
  prototype as it was left: Golden Plains fills the place, and nothing leads up to it.
- **Treasure Lift** (General): Restored has CloudRunner Fortress's wind lift past the blasted wall lower a player who
  steps in from the top, the way down to the treasure caves. Original is the prototype as it was left: from the top
  the wind only ever blows up, and the caves can't be reached.
- **Krazoa Palace** (General): Restored places Krazoa Palace's doors, consoles, text blocks and red Krazoa, so its
  four wings are played in turn and the red Krazoa sends the player on to Drakor. Original is the prototype as it was
  left: the palace's rooms stand empty and lead nowhere.
- **Ending** (General): Restored goes back to the title once Drakor is beaten and his fall has played, as the game's
  own menus go back to it. Original is the prototype as it was left: after his fall the fight starts again.

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

`tools/dino.py` reads the game's code from the ROM in names, for working out what it does: `dis` prints one function
with its registers, GOT loads, DLL calls and bits named from `syms/names.toml`, `calls` every call to a function, an
export or a bit with its arguments, `map` a map's placements, `def` an object def and `warp` the warp table. Its
header says how; it needs the rabbitizer spimdisasm installs.

## License

GPL-3.0 (`LICENSE`), as the recompilations it follows.
