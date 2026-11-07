# Half-done work on Dragon Rock's way down: delete this folder before the card lands

- `autopilot.diff`: a debug-only patch over `src/` (git apply it, never commit it to `src/`). `DINO_WALK=<file>` runs
  steps from a file: `at <s>`, `w x z [r]` walk to a world x,z, `b <hex> <s>` hold buttons, `s x y <s>` hold the stick,
  `p <s>` wait, `t x y z` teleport (stops at the first wall on the line), `x <cell> <s>` flood-fill the reachable cells,
  logging `cell`/`wall` lines. `DINO_STUCK=<s>` is how long `w` waits on no progress. Also prints a `dbg:` line a second
  (target, its id, item, magic).
- `in8.txt`: the `--input` that opens the portal and drops through it, with
  `--story before-dragon-rock --place -1346,-986,-6289`: in map 52 by about 46 s.
- `w7.txt`, `map.py`, `explored-e1.txt`: the explorer's steps, its log to a map (`map.py 48 <stderr.log>`), the map so far.

## Where this got to

Played, from `before-dragon-rock` placed on the ledge (`in8.txt`):

- The portal opens. C-left opens the spell scroll, and each further C-left rolls it one item (dll1_1FEC counts a press
  of the menu's own button; the stick and the other C buttons don't move it). From the scroll's first opening two
  rolls highlight the orange portal spell, and A takes it.
- A spell needs magic: dll210_6DD8 returns at once when the short at stats +4 is 0, so the start point now gives 100
  (commit a98b8df; stats +6 is the most, capped at 100). Whether a player arriving by the story has any is not looked at.
- A target needs the player facing it as well as near it: at (-1333, -6466) nothing is targeted, on the lid's own line
  at (-1346, -6444) DR_SupPortal is. Group 8 loads only by walking into TriggerPoint 35319, not by starting inside it.
- The lid's cutscene plays, 0x776 sets and a square hole opens. Walking in crosses plane 351c9 (0x783) and warps to
  entry 88 in map 52, at (-1350, -1522, -6746).

Bottom, in world coordinates (local + 1280, 0, -5120):

- The landing room is x -1421..-1139, z -6933..-6681. Its "green panel" to the north is an opening onto a lower green
  room, blocked at the ledge. The way out is a gap in the WEST wall, z about -6780..-6681.
- From there a torch-lit corridor runs west at y -1522, then a ramp runs south along x about -1870 down to y -1700 at
  z -5760 (`explored-e1.txt`, y = character index * 25 - 2000). At about (-1731, -6569) a drop lands on the y -1805
  floor; the fall cost 4 of 12 health and the explorer did not come back up. Its 230 s ran out there.
- `--place x,y,z,layer` takes a layer, so a start straight into Bottom (layer -1) skips the 46 s lead. Not tried yet.
- Teleporting (`t`) stops at the first wall on its line, so it can't cross the level eastwards.

Read, not played:

- A second way down: Top's PortalSpellDoor 34166 (local 1121,-1725,-1613, +0x1E 0x64A) and plane 348f0 lead to Bottom's
  plane 34c5e in the west. A vertical shaft at Bottom local (-2670, -690) lines up with Top local (2450, 590).
- DR_PushCart (dll734, map 2 group 7, at Bottom local -3483,-1768,-2046) runs on rails under the landing room: bits
  0x660 and 0x661 cycle at about 52.9-58.7 s of every run, and it is what sets 0x649 and 0x788. Bottom's middle may
  depend on it, and on DR_TrackLever, DRProjectileSwi, DR_ExplodeDoor (hit by the projectile spell), DR_CaveIn.
- Bottom plane 348ef (local -1058,-1492,-1046) is the layer -1 to 0 step eastwards, towards map 53.

Next:

1. Explore on from the ramp's foot and the y -1805 floor, `x` steps started from `--place` into Bottom, until a way
   east past local x -1058 shows, or what stops one does (a door, the cart, a switch).
2. Bottom into the dragon's room, the fight with `DINO_DRAGON_HEALTH=1`, the stone carried out.
3. Whether the ledge is reachable on foot from setuppoint 2 (the CloudRunner chain, or the second way down above).
4. docs/story-points.md (Dragon Rock: 0x95 and 0x96 constants, group 8, the scroll, magic, facing, entry 88, the
   "loaded only once" text), README Progression fixes, `releases/next.md` for a98b8df if it isn't dev-only, then delete
   this folder and land with `--whole-card`. None of that is done, and no PR is open.
