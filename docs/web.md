# The web page

Dinosaur Planet: AI Slop Recomp plays in a browser: built with Emscripten on N64Recomp-browser's `frontend/` into
`DinosaurPlanet.html` (the launcher, the recomp's menus, SDL2's audio and keyboard, the ROM from Load ROM, and the saves
kept in IndexedDB; `lib/N64Recomp-browser/frontend/CMakeLists.txt`'s header says what each part does), drawn through
WebGPU. The page is built from your ROM like the desktop program, so it is never built in CI and never published with
the game in it. It is not published at all yet: the game is held unreleased (N64Recomp-browser's `docs/publishing.md`),
so its page is built and played but not put on the hub.

What is the game's own stays here: the `EMSCRIPTEN` branches of `CMakeLists.txt` that make the game a page,
`tools/build_web.sh`, the prebuilt shader list (`src/rt64_shaders.txt`, recorded by `tools/shader_plan.json`; the
list's header says how to record it again), the play plans (`tools/play_plan.json`, `tools/play_plan_extended.json`)
and the survey they share (`tools/survey.js`). The rest is N64Recomp-browser's: `docs/porting.md` (WebGPU and the
shaders, prebuilt shader lists, Emscripten), `docs/playing.md` (playing and measuring a page), `docs/forks.md` and
`docs/findings.md`. Every play of this game's page is a line in N64Recomp-browser's `plays.jsonl` (game `dino`), never a
record here.

## Build it

On Linux with Docker, with the ROM at `roms/dinoplanet.z64`:

```bash
tools/build_web.sh                  # [cpus] [cmake arguments]
```

It builds `build/web/DinosaurPlanet.html` with its `.js`, `.wasm` (about 22 MB) and `.data` (about 9 MB: the recomp's
fonts and menus, nothing of the game) in `tools/docker/Dockerfile.web`, the image every N64Recomp-browser page is built
in, with ccache in `build/ccache-web`, so a rebuild after a change takes a minute or two (a cold build about 7 on a busy
host). The script's header says each step. Two things differ from the desktop build:

- The patches library (`patches/`, built for MIPS) is recompiled by the native N64Recomp that `tools/recompile.sh`
  builds, not the runtime's, which Emscripten would build for the browser.
- RT64 builds its pipelines from `src/rt64_shaders.txt` (`RT64_PREBUILT_SHADER_LIST`), in sections: the boot's before
  the game starts, and an area's the first time it draws.

The game's scheduler takes the RSP for hung when a task has run 11 retraces, and restarts it, which the runtime can't
do: it runs each task whole, so a task only runs that long while the renderer is behind, and the restart stopped the
game. Firefox's page holds the game for seconds while it builds a section of the shader list, so it stopped there at
the opening every time. A hook in `us.toml` keeps the scheduler from counting those retraces (its comment says how),
on the desktop build too, where the restart can't help either.

Serve `build/web` over HTTP from localhost (any static server; the page brings its own `coi_serviceworker.js` for the
headers threads need) and open `DinosaurPlanet.html` in Chrome or Firefox with WebGPU, then Load ROM. The page
interpolates to the display's rate and draws at the canvas's size, as the desktop build's defaults do (Framerate
Display, Resolution Auto).

## The plans

A page has no command line, so a plan gives the running game the desktop build's development options (`--story`,
`--warp`, `--place`, `--bits`, `--report`: CLAUDE.md, Testing and Story points) through `tools/survey.js`'s
`options()`, which hands them to `src/main/main.cpp`'s `dino_web_options` for the game's main thread to take at its
next pass. Its `go(point, map)` starts a story point (`docs/story-points.md`) with `--report` and waits until the
report puts the player in that map. So a plan reaches an area from the game select, as the desktop build's tests do,
without playing what comes before it.

A story point turns on none of a map's object groups, so a level started from one has none of its grouped enemies and
props (`docs/story-points.md`, Object groups). `go(point, map, more)` passes more options after the story point, and
the plans turn on the groups that file names: SwapStone Hollow's Queen, Tricky and mushrooms, Moon Mountain Pass's,
the Swap Store's, and Cape Claw's sand worm and SandEels, at the worm's fight.

- `tools/play_plan.json`, the regular plan (about 2.5 minutes): the game select and a New Game as a player makes one,
  the opening until Krystal is in a map, then SwapStone Hollow with the pause menu opened and closed, and Cape Claw
  at the sand worm's fight.
- `tools/play_plan_extended.json` (about 5 minutes): those, then Warlock Mountain, DarkIce Mines, CloudRunner
  Fortress, the Walled City and the DF shrine.

Their waits and holds are on the game's own clock (`survey.js`'s `clock()`, 60 a second), so every browser gives the
game the same input for the same game time. The name screen moves its cursor a letter as the stick is pushed and
again each 150 ms or so it stays held, stopping at the bottom row and wrapping along a row, so a plan holds the stick
down to the bottom row and taps it right to END, each tap 100 ms on that clock: one letter, and longer than a game
frame on a busy host, which a shorter tap can fall between. A on END and on the new file is pressed again until the
game leaves that screen. The clock stops while the game is paused, so the pause menu's waits are in the page's own
time.

Play them with N64Recomp-browser's `tools/play_page.mjs` (its `docs/playing.md`).

## The shader list

`src/rt64_shaders.txt` holds the boot's shaders (the launcher, the logos, the game select and a new game's menus) and a
section an area: the opening and each area the warp table reaches, each built the first time the area draws. It is
recorded by code with `tools/shader_plan.json` (the list's header says what each section plays and how to record it
again). A play that misses a shader prints it (`play_page.mjs` counts the misses); add it to the section of the area it
was missed in, or, for a screen any area shows (the pause menu), to section 0.

## How it plays

PLAYS_TBD
