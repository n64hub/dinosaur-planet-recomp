# Half-done work on Dragon Rock's way down: delete this folder before the card lands

- `autopilot.diff`: a debug-only patch over `src/` (git apply it, never commit it to `src/`). `DINO_WALK=<file>` runs
  steps from a file: `at <s>`, `w x z [r]` walk to a world x,z, `b <hex> <s>` hold buttons, `s x y <s>` hold the stick,
  `p <s>` wait, `t x y z` teleport (stops at the first wall on the line), `x <cell> <s>` flood-fill the reachable cells,
  logging `cell`/`wall` lines. `DINO_STUCK=<s>` is how long `w` waits on no progress. Also prints a `dbg:` line a second
  (target, its id, item, magic).
- `in8.txt`: the `--input` that opens the portal and drops through it, with
  `--story before-dragon-rock --place -1346,-986,-6289`: in map 52 by about 46 s.
- `w7.txt`, `map.py`, `explored-e1.txt`: the explorer's steps, its log to a map (`map.py 48 <stderr.log>`), the map so far.
