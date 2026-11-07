// Dinosaur Planet's survey for the page's plans (N64Recomp-browser's tools/play_page.mjs and record_shaders.mjs load it
// with an evalFile step). A page has no command line, so options(line) gives the running game the desktop build's
// development options (--story, --warp, --place, --bits, --report: CLAUDE.md, Testing and Story points), which
// src/main/main.cpp's dino_web_options takes on the game's main thread at its next pass. So a plan starts a level from a
// story point or the warp table, as the desktop build's tests do, without playing the levels before it.
//
// go(point, map, more) starts the story point (with ",<warp>" for another entry of the warp table: docs/story-points.md),
// with the options in more (a --bits that turns on the map's object groups, which a story point leaves off, a --place)
// and --report, whose line a second ("story: <s> s map <n> at <x> <y> <z> menu <m>, health <h>/<max>, magic <m>/<max>")
// the survey reads from the console, and waits for the player to be in that map. map() is the map of the last report,
// where() the report.
//
// clock() is the game's own clock, the word at 0x800AE674: 60 a second in its menus and in play, standing still while
// the game is paused (found by reading RDRAM as the game played; the VI count at 0x800918F0 runs on through a pause).
// The plans' clock steps read it, so a hold is so many 60ths of the game's time in every browser. A menu moves its
// cursor a step as the stick is pushed and again each 150 ms or so it stays held, so a plan's menu taps are holds of
// 100 ms on that clock: one step, and longer than a game frame on a busy host, which a shorter hold can fall between.
(() => {
  if (window.dino) return;
  const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
  let report = '', reportAt = 0;
  // The game's threads print through the page's console.error (Emscripten's err), the reports among them.
  for (const name of ['error', 'warn', 'log']) {
    const print = console[name].bind(console);
    console[name] = (...args) => {
      const text = args.join(' ');
      if (text.startsWith('story: ')) { report = text; reportAt = Date.now(); }
      print(...args);
    };
  }
  const dino = window.dino = {
    // Gives the game one line of options; false while the line before is still waiting to be taken.
    options: (line) => {
      const p = _dino_web_options_line();
      if (!p) return false;
      const bytes = new TextEncoder().encode(line);
      HEAPU8.set(bytes, p);
      HEAPU8[p + bytes.length] = 0;
      _dino_web_options();
      return true;
    },
    clock: () => n64r(0x800AE674),
    where: () => report || 'no report yet',
    map: () => { const m = / map (-?\d+) /.exec(report); return m ? Number(m[1]) : null; },
    // Waits up to ms for a report with the player in a map (play, not a menu or a cutscene without one), true once one
    // comes. The reports start with options('--report').
    playing: async (ms = 60000) => {
      for (const t0 = Date.now(); Date.now() - t0 < ms; await sleep(200)) {
        if (reportAt > t0 && dino.map() !== null) return true;
      }
      console.log(`[survey] no player after ${ms} ms: ${dino.where()}`);
      return false;
    },
    // Starts a story point with more options, and waits up to ms for a report from that map, true once one comes.
    go: async (point, map, more = '', ms = 60000) => {
      const t0 = Date.now();
      while (!dino.options(`--story ${point}@0 ${more} --report`)) {
        if (Date.now() - t0 > ms) return false;
        await sleep(100);
      }
      for (; Date.now() - t0 < ms; await sleep(200)) {
        if (reportAt > t0 && dino.map() === map) return true;
      }
      console.log(`[survey] not in map ${map} after ${ms} ms: ${dino.where()}`);
      return false;
    },
  };
})();
