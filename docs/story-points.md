# Story points and the warp table

A test that needs a level late in the game starts from a named point of the story, in memory, with nothing written to
Flash. The option is development only, like `--warp`: nothing in a player's menus reaches it.

```
DinosaurPlanet --boot --story before-wg-shrine --report
DinosaurPlanet --boot --story new-game,36
DinosaurPlanet --boot --story before-sand-worm --bits 1eb
DinosaurPlanet --boot --story new-game --place 3620,-265,16205,0
```

- `--story <point>[@<seconds>][,<warp>]` starts from that point, 2 s after the game's first controller read unless
  told, at the point's own place or at that entry of the warp table, as a warp there has it: an entry a setting fills
  (docs/restored-content.md, The warp entries the restorations take) starts at its filled place.
- `--place <x>,<y>,<z>[,<layer>[,<facing>]]` starts there instead, for a place the warp table has no entry for, facing
  a 16-bit angle (0, the default, faces -z).
- `--bits <hex>[=<value>][@<seconds>],...` sets more of the game's bits at the start, after the point's own, or one
  given a time when the `--input` clock reaches it: what a test's own object sets, for a test a script can't play.
- `--time <hour>[@<seconds>]` sets the time of day once there is a player in a level, or once the `--input` clock
  reaches that time: night is 21 to 5 (The third SpellStone and the moonseeds).
- `--sidekick kyte|tricky` brings one out at the player whenever none is, as a level's sideloads (269) do where a
  sidekick joins: a `--place` start far from one has none, and a sidekick's command needs one.
- `--report` prints a line a second, from the start or from the game's first pass without `--story`: the clock, the map
  number, the position, the health and the magic. A run's `--story:` line says what started.
- `--watch-bits <hex>,...` prints a `bits:` line each time the game sets one of those bits, with the clock, the value
  and the one it had, and `--watch-bits all` one each time any bit changes: what a test or a cutscene sets, as it
  happens. With or without `--story`.

`src/game/story.cpp` holds the points, and `src/main/main.cpp` reads the options.

A test of a setting (Original against Restored) edits the `general.json` the game reads, which is beside
`portable.txt` in the working directory, and without one there is the user's own (`%LOCALAPPDATA%` on Windows, shared
by every checkout). A build run from the repo's root has none: run it from its build folder with `portable.txt`,
`assets/` and `recompcontrollerdb.txt` there, or every value is whatever the user's file says.

## How the game keeps its story

The story is a list of numbered bits. `func_80014670` sets one and `func_800147EC` reads one, through the table in ROM
file 55: 2336 entries of 4 bytes, a start bit (u16), flags (u8: the bank in the top 2 bits, 0x20 to tell a task, the
length less one in the low 5) and the task. Bank 0 lives at save + 0x17AC and is not saved; banks 1, 2 and 3 are at
+0x13D4, +0x12D4 and +0x15D4 of the save, which `0x800AE2A0` points at.

dll29 is the save (`0x8008C9F4` holds its handle; dll31 is only the Flash driver). What `story_start` uses of it:

- Export 1 (slot, name) clears the save to a new game's, sets bit 0x4E5 and writes the Flash, or with slot -1 doesn't.
- Export 29 (character) sets the character, kept at save + 0x2F7: 1 is Krystal and 0 is Sabre.
- Export 6 (position, yaw, flags, layer) stores where play starts, 16 bytes per character at save + 0x16D4.
- Export 7 starts the world there and keeps a copy of the save that a retry restores, so the bits go in before it.

The game's own New Game is dll64's name screen calling export 1, then dll63 calling `func_8005D410(1, ...)`, its own
`dll63_0E28(1)`, `func_80041D20(1)`, `func_80041C6C(1)` and export 7, and `story_start` makes the same calls:

- `func_8005D410` drops from the 640x480 mode the game boots in, for its logos, to play's 320x240. In the boot's mode
  the Z buffer (`0x80093010`) is the first framebuffer, so a level played in it shows depth as colour in a third of its
  frames: bands of magenta and green, white where the depth is clear.
- `dll63_0E28(1)` lets go of what the title holds for the menus after it: dll60's constructor takes texture groups 0,
  4 and 5 (`func_8000CB00`; `func_8000CBC8` drops one) and loads DLLs 75 and 76 (`func_80014B1C`, kept at
  `0x8008C9EC` and `0x8008CA04`; `func_80014B6C` unloads them). A story start comes from the title, not dll63, so
  `story_start` makes those drops itself. Held through play, they crash NewIceMt1 as it loads (warps 2 and 26).
- The other two set flags in the word at `0x80092A98`: without them no object is drawn and the opening's textures are
  wrong.

`func_800142F0` is the rolling demo, not a new game.

A run from a story point differs from a real New Game in one way a test can see: a strip of stale pixels, text from
the boot, stays in the top rows of some frames, where a real New Game has black. It is not in the framebuffers in
memory (clearing them left it), and it changes nothing in play.

## The points

Played from a boot with `--report` and looked at. Which character plays what comes from the game's own task list (ROM
file 19, each task's name and clue), which says when the player swaps.

- `new-game`: Krystal, no bits, the real opening on the pterodactyl.
- `swapstone-hollow`: Sabre in SwapStone Hollow (warp 15), through task 9, the tunnel out of Ice Mountain: the
  Queen not met yet and no white mushroom collected.
- `after-first-spellstone`: Sabre at the Volcano Force Point (warp 81), the first spirit won and the first SpellStone
  placed.
- `before-sand-worm`: Krystal in Cape Claw, on the sand at the map's setuppoint beside the worm. The ROM's warp table
  has no entry in Cape Claw.
- `before-df-shrine`, Krystal (warp 36); `before-db-shrine`, Sabre (38); `before-mm-shrine`, Krystal (43);
  `before-ec-shrine`, Sabre (40): the four shrines the task list orders, each with the spirits before it.
- `before-gp-shrine` (42), `before-nw-shrine` (37), `before-cc-shrine` (41), `before-wg-shrine` (39): Krystal with all
  four of those spirits. The task list stops before these (from task 107 it holds placeholders), so their order and
  who plays them are not known from the ROM.
- `before-cloudrunner`: Krystal at Moon Mountain Pass (warp 73), with two spirits and the first SpellStone placed,
  and SwapStone Circle at its act 9, where its chief gives the pass's key (CloudRunner Fortress).
- `before-desert-force-point`: Krystal in Golden Plains, at its setuppoint 0 where the way from SwapStone Circle comes
  in, carrying CloudRunner's SpellStone. Its warp, 31, stands under the sand: a start there falls and is put back.
- `before-walled-city`: Sabre in Walled City at its setuppoint 0, where the way from SwapStone Hollow comes in,
  outside the city's gate, with the second SpellStone placed, Tricky's commands (0x4E4) and his flame (0x245,
  with 0x172 and 0x27 as DarkIce Mines leaves them, DarkIce Mines' SnowHorn). With a warp it starts there instead:
  `before-walled-city,91` where BOSS TRex's way back comes out.
- `before-third-stone`: Sabre in Diamond Bay (warp 66), carrying Walled City's SpellStone, with the Volcano Force
  Point in act 2, which handed it over.
- `before-dragon-rock`: Krystal on Dragon Rock's top (its setuppoint 2), with all four spirits, three SpellStones
  placed and Golden Plains' portal spell (0x5BD, its PortalSpell, dll272), which DR_SupPortal on the top opens to.
- `after-dragon-rock`: Krystal in Golden Plains (its setuppoint 0), carrying the fourth SpellStone (0x7A8) that the
  Kamerian Dragon leaves with Kamerian Dragon on Restored, for the Desert Force Point's other place.
- `before-drakor`: the same, at BOSS Drakor's setuppoint (map 44), which no warp entry reaches.

The later SpellStones are told by the SpellPlace that takes each: a placement's +0x1E is its placed bit and +0x20 the
bit of the stone carried, as VFP_SpellPlace (637) takes the first as 0x877 and 0x123. CloudRunner's, won in its race
(CRSpellstone in map 43 holds 0x49F), is carried as 0x2E8 to the Desert Force Point (DFP_SpellPlace in map 21, placed
0x5F3). Walled City's is carried as 0x83B to the Volcano Force Point's second place (0x878). The Desert Force Point's
other place (0x5F4) holds 0x7A8 at +0x20, but 0x7A8 is no stone: it is a target block's landing, and the place takes a
stone used from the item menu by the force point's act (Golden Plains and the Desert Force Point). (Curves' params
hold the low halves of other curves' uids, not bits.) With Kamerian Dragon on Restored the Kamerian Dragon leaves a
stone that sets 0x7A8 (The story after the first SpellStone, below). A start in a cell no map fills loads nothing (the
player falls and is put back, forever): Dragon Rock's own setuppoint 0 is in one, so `before-dragon-rock` starts at
its setuppoint 2.

## The story after the first SpellStone

The task list (ROM file 19, task 50 the first SpellStone placed) goes on to the fourth spirit, and file 55 gives bits
only to its tasks up to 39, so the rest is told by the levels' own objects. In its order, with who plays:

- Sabre: Diamond Bay's shrine opened, the test of strength, the second spirit to Warlock Mountain (tasks 51-54; 55 is a placeholder).
- Krystal: the Moon Mountain Pass key from the chief, Moon Mountain Pass, then CloudRunner Fortress: Kyte's mother,
  the dungeon (map 16), the power, Kyte, five baby CloudRunners, the gold, the treasure caves (15) and the jetbike race
  (43) for the second SpellStone, which Bonehead activates (tasks 56-68).
- Krystal: Golden Plains (9), the portable hole spell, the Desert Force Point (maps 50 and 21), the second SpellStone
  placed, the mirage, and the swap to Sabre (tasks 69-74).
- Sabre: Walled City (13), the Queen and Tricky's mother, ten RedEye teeth, the King Earthwalker, the sun and moon
  beacons, the boss door and BOSS TRex (48) for the third SpellStone, which the King activates (tasks 75-83).
- Sabre: another log from the Thorntail, Diamond Bay's second Pointback egg, the volcano's second path and the third
  SpellStone placed; moonseeds for Krystal (tasks 84-91).
- Krystal: the seed planted in Moon Mountain Pass, the meteor moved, the test of fear and the third spirit (92-97).
- Sabre: Walled City's sun and moon stones, the test of skill and the fourth spirit (98-105), and the swap to Krystal (106).

Tasks 107 on are placeholders ("TASK 107 NAME"). Past them the ROM still has:

- Dragon Rock - Top (map 2) and - Bottom (52), and BOSS Kamerian Dragon (53), placed in full. Their cells join Willow
  Grove's at layer 0, so they are reached on foot; no code warps there by number. Bits 0x95 and 0x96 are constants:
  the getter answers 1 and 0 and nothing sets them, so a placement's 0x95 means always and 0x96 never. The top's
  ProjectileSpell (object 0x4A, dll272, at -3553, -1502, -4662) gives the ice spell (0x5CE; the projectile spell is
  the scroll's 0x2D). A takes it within 16 of it, its def's reach, but the player stops about 18 from it from any
  side, so with Kamerian Dragon on Restored it reaches 24 (`src/game/dragon_rock.cpp`). Its DR_SupPortal (754, in
  group 8) is hidden once 0x776 is set (which only HitAnimator 353 beside it reads); with the player's spell the
  portal spell (0x5BD, from Golden Plains) it plays its sequence, and its trigger plane sets 0x783, on which it warps
  to the bottom (warp 88, at -1350, -1522, -6746 on layer -1). Group 8 is turned on by walking into TriggerPoint
  0x35319 by the ledge, not by a start inside it. C-left opens the spell scroll and each further C-left rolls it one
  item (dll1_1FEC; the stick and the other C buttons don't), and from its first opening two rolls highlight the portal
  spell, which A takes. A spell needs magic (dll210_6DD8 returns at once with none), and a target needs the player
  facing it as well as near: at (-1333, -6466) nothing is targeted, on the lid's line at (-1346, -6444) the portal
  is. Played from `before-dragon-rock` with `--place -1346,-986,-6289` and an `--input` that walks onto the lid's
  line at 26 s, opens the scroll at 30 s, rolls it at 32 s and 34 s and takes the spell at 36 s: the lid opened
  (0x776) at 37 s and the warp to the bottom came at 45 s.
- The bottom is two parts on layer -1. The west part holds warp 88's landing, the tracks under it (DR_PushCart,
  DR_TrackLevers, DR_pulleys and DR_cradles), DR_ExplodeDoors and DRProjectileSwitches for the projectile spell, and a
  shaft at (-1395, -5807) whose TriggerPlane steps up a layer to the top's west (and down again). The landing's way
  out is a gap in its west wall, then a corridor west at y -1522 and a ramp south along x -1870 to a floor at y -1805.
  The east part is the top's east end's layer below: TriggerPlanes at (171, -6198) and (222, -6166) step between
  them, setuppoint 1 at (128, -1426, -6184) on the top's side. From there it goes east past a DR_Platform (0x5CD, its
  DR_Lever at 109, -6103) and setuppoint 4 (572, -1568, -6174) to the Skeetla arena's door at setuppoint 0 (1152,
  -1618, -6692), where TriggerPlane 0x33EAB sets 0x5F9 (DR_Seqobj sequence 0, 0x5FA), and down to the arena (y
  -1831). From the west part a walk east from setuppoint 8 (-817, -1613, -5830) stops at x -721, at a DR_WireFence
  (0x662, 0x663) beside a ProjectileSwitch that sets 0x662; it is not played through.
- The east part is played through from the top. The step down is fussy: the player walks back and forth over the
  TriggerPlanes at (171, -6198) and (222, -6166) until she is on layer -1, then down the stairs east to (287, -6180)
  and on by (540, -6020), (625, -6097) and (738, -6164) to the door. There a wall stands in the ROM's way: HitAnimator
  (353) uid 0x412A9 at (646, -6111), bit 0x95 (always) and group 2, which the top's DR_EarthCallPad leaves open, made
  its change to the block loaded before, and the bottom's block loads again as the player comes down the stairs, with
  its lines as the ROM has them. A HitAnimator makes its change again only when its block is missing or its bit
  changes, so every update now makes it again, always on as a plain bug fix (`src/game/hit_animator.cpp`).
- The arena (groups 0 to 2, `--bits 5dc=7`): DR_Creators (738) send Skeetlas (615), at most 20, at DR_EarthWarrior
  (713), shackled there. It is won when 0x628 is set, by the Warrior's update once 0x789 is 3 and no Skeetla is left,
  which plays DR_Seqobj 0x339C6's sequence 7, and its end sets 0x609 (on the top it opens the CloudRunner's cage,
  DR_Cage, DR_CageControl and DR_CloudRunner). 0x789 counts the three DRLavaControls (737) put out: each loses a
  point of health (3 or 15, its +0x1A) to a hit of kind 0x19 (export 2) and at 0 sets its bit (0x5D8 at 2032,
  -6666, 0x5D9 at 1618, -6347, 0x5DA at 2240, -6492) and raises 0x789. The staff (B) and the projectile spell don't
  hit as 0x19; the ice spell (0x5CE) does, a blast a point of magic (dll277 0x0080), and a control of 15 takes about
  20 blasts as it plays. With three of them, the most magic 25, none to be had on Dragon Rock and none coming back on
  its own, the fight can't be won as the ROM has it. With Kamerian Dragon on Restored the arena fills the player's
  magic while its fight is on, once it is below a fifth, as the dragon's fight a layer up fills it each update.
- The ice spell is cast from the scroll as the projectile is aimed, with Kamerian Dragon on Restored: in the ROM the
  scroll aims only the projectile (0x2D, dll210_1DDC), and the aim selects the projectile again before A casts, so the
  ice blast (dll210_1DC48) is never reached. C-left opens the scroll and C-left again rolls it to the ice spell, A
  aims, and A held casts the blast straight ahead; for the next cast B puts the staff away, and C-left opens the
  scroll on the ice spell again. Stood about 100 north of a control and facing it, a held A puts it out in about 3 s.
- The Warrior's health (+0xA56 of his state) starts at 1, and each Skeetla that reaches him bites it lower, one every
  1-3 s once the arena is full; below -20 he dies (0x5DE, DR_Seqobj 0x33EAE's sequence 5, which puts the player out of
  the arena at 1312, -6733). The arena's DR_Creators (kind 0) make a Skeetla every few seconds from the moment the
  arena loads, as the player passes TriggerPlane 0x33E62 by the door (group 1), while their control burns and fewer
  than 21 Skeetlas are about, and faster with each control out; the bites start about 35 s after it loads. The door's
  two DR_Creators (kind 1, group 3, on from TriggerPlane 0x33E61 at 577, -6121 to the door) send Skeetlas at the
  player, which follow her in and knock her about. Each ice blast that hits a Skeetla kills it, and the fight is won
  only once none is left, the door's too. Item 0xC1 used on him adds 12 (0x0974), as his export 20 does, which
  SHbluemushroom (dll502 0x0698) calls as he walks onto one. Dragon Rock places eleven blue mushrooms (dll502, model 0):
  four by the arena's east side, at about 2260, -6200, and on the top three south of the start (34885 to 34887, about
  -3480, -5350) and four by its east end (40091 to 40094, about -390, -6850). One of each of the top's groups (34887, 40093)
  runs from the player once she is within its setup's +0x19 (49 and 80) and the others stand still. Each is picked up
  with A once it can be picked out: one at rest (state 0) has +0xAF bit 3 set, and only its state 9, now and then,
  clears it, so A pressed each second beside one takes it within about 6 s; A pressed on the move, with none in reach,
  has her creep (her act 55) after the one in view, slower than it runs. Each raises 0xC1, and the first plays text
  sequence 0x169 (0x911). But only Sabre's item menu (dll1's table at +0x2E4) lists 0xC1, not Krystal's (+0x128); the
  Warrior's update sets his own +0xAF bit 3 while the fight is on (0x0540), so he can't be picked out, and he walks
  onto none until he is freed (0x655). So the fight has about 45 s of bites, which in a run from the top run out
  before the third control. With Kamerian Dragon on Restored Krystal's menu lists 0xC1 first, and he can be picked out
  while she carries one: C-right, then A, from where A uses him (his focus point, about 1580, -6104, south of him)
  gives it, 12 health and about 4 s of eating, in which he can't be picked out again. From 30 units further south a
  Skeetla nearer her is picked out in his place, and in a full arena ten tries running failed there; on the focus
  point each give landed within five. The C-right menu keeps its place between opens and C-right pressed while it is
  open rolls it on, so with her food bag (0x164) in it a try that leaves it open rolls the next one onto the bag: B
  twice before each shuts it. The Skeetlas the door's DR_Creators send bite Krystal there too, about every 2 s, and at
  0 the game over's menu (9) continues on A where she fell, at full health and with the fight as it was, while C-right
  on it moves to its other choice, a load of the last save. He is bitten about every 2 s with the arena full, so each
  mushroom buys about 24 s. The controls nearest him go last: 0x5DA, then 0x5D8, then 0x5D9, then the blast on the
  Skeetlas left.
- The Warrior is freed by 0x5FD, which DR_NiceSharpy (739, uid 0x33EB2, on the gallery above the arena's east end at
  2413, -6793) sets when it is given item 0x380 or 0x381 (its 0x07B4 reads dll210's export 74, and calls export
  75 with 1). 0x380 is meat: the two Crates beside it (0x40C16 and 0x40C17, about 2059 to 2479, -6800), broken with
  the staff, each drop a meatPickup (object 0xB), which A takes into the food bag. The gallery is the arena's own
  layer: from the door (1410, -6634) by (1491, -6670), (1548, -6700), (1623, -6753), (1694, -6791) and (2000, -6800).
  The bag is given from the C-right menu, Krystal's first (dll1, the player's object 0x1F), with the staff put away
  (the give is tested in dll210_BA38, her idle state): C-right shows her bags (0x163, 0x164 and 0x165, each its own
  bit), A opens them, C-right twice rolls to the food bag (0x387, icon 0x2EF), A opens it, and A on the meat gives it
  to what she faces. Only 0x164 has a setter, WL_LevelControl (dll607 0x0194) as Warlock Mountain loads in act 6,
  which Cape Claw's shrine won sets (dll486 0x0C44), and `before-dragon-rock` and `after-dragon-rock` set it. Then
  DR_Seqobj 0x339C4 (group 1, on from the door's trigger) plays sequence 2, the lever pulled, and sets 0x5FE, which
  opens the DR_Shackles (747), and his update (dll713 0x0670), while his health is above 0, sets 0x654. The lever's
  scene is about 15 s: NiceSharpy eats and pulls the lever on the gallery, the shackles open on the arena's floor, and
  it ends with the player where she gave the meat. The give names NiceSharpy as the next sequence's origin (dll3.30,
  next_sequence_origin, with its flag set) for its own start, and the seqobj's start comes in the same frame, before
  the give's end (dll3.18) clears that origin: dll3_8900 cleared only the flag after a start, so the lever's scene was
  placed and turned around NiceSharpy, its camera outside the walls over cells never loaded (func_80044810 -1 at the
  player), and it ended with her at (2446, -1610, -6731), past the gallery's east end over nothing. The void's respawn
  by the door then reloaded the bits as they were there, the meat, 0x5FD, 0x5FE and 0x654 gone. Always on, the
  origin is cleared with its flag (us.toml). A sequence's actors and keys are ROM files 63 and 64 (its actors: object
  ids, 0xFFFF the origin, 0xFFFE the AnimCamera), 15, 14 and 13 (each actor's keys: x, y and z offsets turned by the
  origin's facing, yaw, pitch); sequence 2's camera stands on the gallery, then at (1586, -1804, -6127) by the
  shackles, from the seqobj. From the gallery she drops into the arena. DR_Seqobj 0x348DC (no
  group, flags 0x04: it loads with the player within its +6 byte, 14, times 8, so 112, of it) stands at (1563,
  -1782, -6188) on the arena's south side, and on 0x654 plays sequence 0xC and sets 0x655. Freed (his state's
  +0xA58 bit 0x20) he no longer reads the win (0x628, 0x0508): the ROM frees him after the arena is won, and needs
  him fed for it. The TriggerPlane 0x33F78 in the middle of the arena would set 0x5FD, but its only action's flags
  (0x04, repeat, without 0x01, forward) never run.
- Played from `before-dragon-rock` with `--place 750,-1620,-6440,-1 --bits 5ce,5d8,5d9,5da` (the controls out, no
  Skeetlas, his health at 1) and an `--input` that walks the gallery's route, breaks the crates, takes the meat and
  gives it: the meat at 67 s (0x380), 0x5FD and 0x5FE at 91 s, the scene on the gallery and the shackles, its end
  with her where she gave at 106.6 s with 0x654, and from the gallery down into the arena and to 0x348DC, 0x655 at
  122 s, her health untouched. With the origin left set the scene showed the lever against a purple void and the
  void's respawn took the bits back.
- Played from `before-dragon-rock` with `--place 750,-1620,-6440,-1 --bits 5ce`, past the door's DR_Creators' trigger,
  and an `--input` that walks to the door, then to each control from its north and casts the ice: the door's
  sequence at 14 s (0x5FA), the controls out at 54 s (0x5DA), 66 s (0x5D8) and 81 s (0x5D9), the fight won (0x628,
  0x609) at 92 s with the Warrior one bite from death, and the warp into the dragon's room at once.
- Played from `before-dragon-rock` in one run, with no `--place` or `--bits`: `tools/dragon_rock_input.txt` takes the
  ice spell (8.6 s), picks up five of the top's blue mushrooms on its way south and east (0xC1 at 38, 44, 121, 130 and
  135 s), steps down to layer -1, walks to the door (0x5FA at 221 s), and, each part waiting on the bit before it
  (`sync`), gives the Warrior the five from his focus point once she is by him (at 252, 258, 265, 271 and 278 s),
  casts at each control from its north until it is out (0x5DA at 308 s, 0x5D8 at 333 s, 0x5D9 at 373 s), sweeps the
  ice over the Skeetlas at him and walks at each one left (`chase 43E`): the fight won (0x628, 0x609) at 386 s with
  his health at 7, never below 1, and the walk to the north wall into the dragon's room (map 53) at 406 s. Two more
  runs won, at 403 s with five given and at 402 s with four, Krystal's one game over there continued (`menu 9 8000`);
  one lost the Warrior after the controls were out at 383 s, the Skeetla left found by none of the walks in 200 s.
- The arena and the dragon's room share the world cell (2, -8) and their ground: the room is the arena on layer 0,
  its KamColumns' pits where the arena's DRLavaControls stand. Nothing in the ROM leads into the room: no warp entry
  lies in it, and its entrance, a TriggerPlane at its north (0x41AF3, which sets 0x4D7 for its KD_Sequences), faces
  cells with no blocks, where the player falls and is put back. With Kamerian Dragon on Restored, the arena's north
  wall under that entrance (x 1860 to 2160, z -6160 on, where the floor slopes up to the wall at about -6120) leads
  into the room by the empty warp entry 89 once the arena is won (0x609), and the fourth SpellStone, 3 s after it is
  picked up, back to the arena by the empty entry 84 (`src/game/dragon_rock.cpp`). At the wall, not at the win, so
  the Warrior can be freed after it. Played from `before-dragon-rock` with `--place 1659,-1822,-6121,-1 --bits
  5dc=7,628,609` and an `--input` that walks to (2009, -6300) and north: the warp into the room from (2003, -1739,
  -6160). In the room, with `7a8@20`, the warp back to the arena came at 23 s, where she stayed.
- BOSS Kamerian Dragon (53): KamerianBoss (dll707) and the Nurse (708). The fight is built and can't be won: each hit
  in its last stage lowers its health (10 at the start), nothing reads it, and at 0 it wraps and the fight goes on.
  Nothing anywhere gives the fourth SpellStone (0x7A8), which the Desert Force Point's other place takes. Dragon Rock
  is the one place the task list's story doesn't reach that has a boss, and the fourth SpellStone the one thing the
  story still lacks, so Kamerian Dragon on Restored, the default, has the dragon die at 0 and leave that SpellStone in
  the player's hands (that it gives the fourth is read from the story's shape, not from the ROM); Original plays as
  the console does (`src/game/dragon_rock.cpp` has the stages and the evidence). Played from `before-dragon-rock`
  with `--place 1925,-1805,-5965 --bits 5ce` and an `--input` that every 10 s opens the spell scroll (C-left), takes
  the projectile spell into aim view (A), pitches up a little more each time and fires (A): the stages set at 31 s
  and 35 s and a hit lowered its health at 51 s. With `DINO_DRAGON_HEALTH=1` that hit killed it: 0x7A8 set at 51.9 s, and the dragon
  and the Nurse were gone. The Nurse knocks the player about, into the pit beside the dragon too (y -1993), and a
  KamSwitch (285) hit by the projectile raises a KamColumn (709) out of it.
- BOSS Drakor (44): the fight is built and is won, but nothing followed the win. No warp entry and no map of the story
  reaches it: its cells touch no other map's, and the only code that starts it is dll70, the prototype's own
  development warp menu (`func_800141A4(44, ...)`). Drakor (697) starts with 24 health (+0x348, set by dll33's
  start-up, 0x20F4), and nothing lowers it, nor needs to: a hit (697 0x0838) sends him to his second state, which lets
  the Diamond go (BossDrakorLevel, 692 0x02F8: 0x7C6, sequence 2), and the player, Drakor and the Diamond drop into the
  room under the fight. Walking to the Diamond there sets 0x7AE (692 0x0FE0), which plays sequence 1, Drakor's fall
  (and sets 0x7AF, which nothing reads). His second state gives the player 43 s from the hit; then the Diamond goes
  back (0x7C5, sequence 3) and the fight goes on. Sequence 1 sends no event and none of the map's four
  BossDrakor_sequ (309) holds a warp entry, so on the console the fight starts again after his fall. The Ending
  setting, Restored, plays the ROM's credits there (dll78, menu 15, which nothing opens), then the title
  (`src/game/drakor.cpp`). Played from `before-drakor` with
  `--input`: A every half second from 12 s to 25.6 s hits him (0x7C6 at 25 s), the stick held up from 27 s walks the
  player to the Diamond (0x7AE at 47-49 s), the credits follow at 83 s, their ten pages on black, and the title at
  206 s, as the credits' clock (7350 ticks) runs out.
- Cape Claw's Krazoa tablets: file 55 gives bits to tasks 133 to 136 (0x238, 0x239, 0x237, 0x3AC), which the task
  list names only as placeholders. The three receptacles (CCkrazoaRecepta, 308) take tablets counted in 0x23B and
  are done as 0x238 to 0x23A; CCkrazoaTablet (459) sets 0x3A9 once all three are placed. CCkrazoaBits (458) counts
  a tablet into 0x23B and sets 0x23C for each of 0x234, 0x236 and 0x237 newly set: 0x234 by a TriggerPlane in
  Discovery Falls, 0x236 by the tablet (CCKrazoaTabletP) in Cape Claw's settlement room, behind the door
  (CCsettlementroo, 307) that 0x23C opens. Nothing sets 0x237: the third tablet is not in the ROM. CCkrazoabright
  (457) runs the room's puzzle in states: the perch switches' lights (sequence 0, whose data sets 0x393), the two
  levers (0x41E, 0x604; 0x4D6 and sequence 1), then the three beacons with the three tablets placed (sequence 3,
  whose data sets 0x3AC), then the lights again (sequence 5); it starts at its later states from 0x393, 0x4D6 and
  0x3AC. The last lights end in a state with no handler, and sequence 5 sets no bit and warps nowhere: played to it,
  the room stays as it is. (A LevelName's +0x1E, as Drakor's 0x237, is a text, not a bit.) Krazoa Tablets on
  Restored, the default, places the third tablet in the sand worm's arena once the worm is dead, and the last lights
  set 0x52B, which shows a Transporter in the tablet room to Krazoa Palace's east end (warp entry 94; 72 is its own,
  93 the palace's west Transporter). `src/game/krazoa.cpp`'s header has the evidence.
- Krazoa Palace (3): its rooms are built and it boots and plays at its setuppoints, but of its 19 objects (KP_*, the
  four consoles, the red Krazoa, the doors) none is placed. It holds mushrooms, crates and two KP_Transporters, each
  beside a setuppoint and each sending to the other's entry, 93 and 94, both zero. Its zone planes mark a hub and
  four wings, and the unplaced objects are a wing's each: a console and a text block (KP_textblock, 654, the Desert
  Force Point's DFP_Textblock: +0x19 its sequence, +0x1E its done bit, +0x20 the bit it needs) for fire, earth, water
  and magic, the section doors and the last door (307), and the red Krazoa. Its cells join only Cape Claw's, whose
  edge there is water: nothing in the ROM leads in. Its east end has the local coordinates of Cape Claw's tablet room,
  and Krazoa Tablets on Restored leads in from there (above), filling entries 93 and 94. Krazoa Palace on Restored,
  the default, places them: a Transporter pair down the shaft from the high hall to the hub (entries 45 and 46, both
  empty), and in each wing a section door, a console and a text block, which go off in turn, fire, earth, water and
  magic (0x6A7-0x6AA, saved bits nothing else uses), each opening the next wing's door and one leaf of the last door.
  Past it the red Krazoa sends the player to BOSS Drakor (entry 44, empty, filled at its setuppoint). Played from the
  tablet room's Transporter (`--place 4776,-8,17606,0 --bits 52b,2c0,912`; 0x912 is the Transporter's first-use
  tutorial, and A on a Transporter is taken about 20 s after an arrival) through the four wings to BOSS Drakor.
  `src/game/krazoa.cpp`'s header has the layout and the evidence. A real save has the palace in act 14 from Kyte's
  rescue, which loads as act 1 (The maps' acts): played from `--story before-sand-worm --place 1660,150,16976,0 --bits
  52b,2c0,912`, walking east across TriggerPlane 30ec4 (0x8EC to 14), then `--warp 72@20` and A on the tablet room's
  Transporter, to the palace's east KP_Transporter, which sent Krystal to the west one and back.
- CloudRunner - TrapRooms (17), Krazoa Shrine (20), krazchamber (22) and MikesLava (49) have no record in file 21:
  nothing reaches them.

A shrine's warp lands at its Transporter, 1600 units from the shrine object. Its test starts when the player walks up
to that object, which each shrine's DLL checks in its first state.

A place that `--place` jumps to has none of the map's groups a player would have walked through. A map loads an object
whose header byte +4 has 0x10 only while its group (byte +6) is on, a bit each in a word the save keeps per map (bit
0x452 for SwapStone Hollow's maps 7 and 8), and TriggerPlanes turn groups on and off as the player crosses them:
action 0x13 turns one on, 0x14 off and 0x22 toggles it (dll325's actions, through dll29's exports 18 and 19). In map 7
group 0 holds the SwapStone and the vine doors, 1 the Queen Earthwalker, 5 the flowers, 6 Tricky's sequence, 7 and 8
the white mushrooms by the river and in the grove, so `--story swapstone-hollow --place ... --bits 452=450` has 1, 6,
7 and 8 on, enough for the Queen, Tricky and the mushrooms. Groups 0 and 5 with those (`452=483`), or all of them at
once (`452=65535`), load and play too, at warp 15 and beside the Magic Plant (`--place -7750,-716,-1207`). The object
create, `func_800213EC`, passes what it made to `func_80021A84` with no test, so a create that failed for want of
memory crashed there, under the MagicPlant (dll290) or SHBirchTree (dll520) that asked for it, as all of them on did
with what the title holds still held (src/game/story.cpp lets go of it). us.toml's hook on `func_80021A84` now returns
for no object, and the create returns 0.

A place in Cape Claw's walled part needs a bit too. A VisAnimator (354) hides the shapes of its groups in its block
while its bit is clear, and TriggerPlanes toggle those bits as the player crosses them (action 0x12, with 0x8000 on
the bit). In map 29, 0x2C0 shows the tablet room and the rooms around it, 0x2C1 the rooms north of them and 0x2B4 the
sea and the east tunnel. The planes that toggle 0x2C0 are in the hall west of the room (uid 41FC0) and the tunnel east
of it (352FB), so a player who walks in sees the room (played, both). `--place` crosses neither, and draws night sky
over black with the objects floating, so a test there needs the bit: `--story before-cc-shrine --place 4400,-7,17600
--bits 2c0`. Warp entries 71 and 72 land in the room and set it themselves (`src/game/krazoa.cpp`).

## CloudRunner Fortress

Tasks 56 to 68, played from `before-cloudrunner` on the desktop build a link at a time, each started with `--place`
and the bits of the links before it. Two links needed a fix to the game (the treasure lift and the babies, each behind
a setting), and the story point did too (SwapStone Circle's act, and the SharpClaw disguise for the dungeon).
World coordinates: the fortress (map 12) lies at cell (-15, 29), its treasure caves (15) at (-20, 28) on layer -1 and
the race (43) at (-20, 29) on layer -2.

- The key. SwapStone Circle's chief (SC_chieflightfoot, 518) gives it only at the circle's act 9, the 4-bit field
  0xE7, which SC_levelcontrol (519) moves on as the player enters map 14. Talked to, he sets 0x60E and 0x60F, and his
  sequence sets 0x611, the bit Moon Mountain Pass's padlock needs. Played.
- The courtyard. Its roof (CFBlastedRoof, 389, about 110 above the courtyard's floor at its middle, group 8 of 0x458) is
  the deck Kyte is shut in on, and blows at the third barrel blown under it: it counts hits of type 5, a CFbarrel's
  explosion (388 0x12A4-0x1314, reach 100, 10 up), one a barrel, each moving 0x2DE+n to 0x2DF+n and 0x900 to n+1, and
  the hit that finds n at its +0x1A (2) sets 0x2E1 (389 0x01CC-0x0398). A barrel on the courtyard's floor under the
  roof's middle reaches it; none is placed there, and the four behind the explosive key's door are one for the wall and
  three for the roof. With 0x2E1 the floor below (CFExplodeFloor, 390) blows, its pieces set 0x43, the force field fades
  (0x34F, 0x359), and Kyte joins: her mother hid her five brothers and sisters. Played: three barrels placed there by
  script and blown set 0x2E1; one, 2e32, lifted in the room behind the key's door, carried out along z 18655, up the
  corridor and over the wind bridge (the shaft, below), set down under the middle and swung at (0x2DF set, 0x900 at 1).
  No barrel reaches the deck itself: the courtyard's ladders (a wall of collision type 0x0A, dll210_BA38) can't be
  started while carrying, and a carried barrel can't be thrown.
- The explosive key. The courtyard's guard, beaten, sets 0x4DF, and CFExplosiveKey (272) appears, 0x8FC once picked
  up. Its door, CCExplosiveDoor (308) in the corridor's east wall at (-13003, 2049, 18679), takes it from the item
  menu: 0x8FC is taken and the door's sequence sets 0x8FF. Played, the key's pickup by bit.
- The wall. Behind that door are four CFbarrels (388). A within 20 units of one, facing it, lifts it (A farther off
  swings the staff), and it is carried through the door and round the corridor to the cracked wall, CFBlastedWall (389)
  in the same wall at z 18879. A beside the wall sets nothing down; a step back it does. A barrel explodes at its third
  hit (one set down counts them too), or at once to an explosion or fire: set down at (-13012, 18891) and swung at from
  (-13016, 18860) it sets 0x8C9, CFExplodeWall (390) blows (0x8CA) a second later, and Krystal loses 4 health and is
  thrown through. Played from `before-cloudrunner`, walked by a script that steers by her position.
- The shaft. Past the wall is CFTreasWindLift (370), at (-12902, 1840, 18886): its wind blows while 0x30D is set, which
  the TriggerPlanes in the hole, on the east lip and at the bottom's two sides flip on each crossing, and its direction
  is 0x8C8, which only the two CFWindSwitch levers (386) at the bottom flip. Clear, it holds the player at the
  corridor's height, a bridge to the courtyard on the east (played); set, it lowers her and lets her go at the bottom.
  So a player from the top hovered there and no button let her down (played: Z, B, A, R, L, Z+R). The lift is in
  group 13 of 0x458 (`--bits 458=8192`). Fixed (Treasure Lift on Restored, `src/game/cloudrunner.cpp`): stepping in
  from the top sets 0x8C8. Played: lowered to the bottom (map 15, y 1859), on north into the treasure caves, and A at
  the south lever turned 0x8C8 back (once of four presses beside it: one walked in from the north swings the staff);
  back in the wind she rides up and steps off west into the corridor (played). One carrying a barrel fell every time
  (four tries): the walking states skip BA38, where the wind's push (+0x808) puts the player on the wind (state 0x49),
  while she carries (dll210 0xA470, 0xAAB4, 0xAF28). Fixed (Wind Bridge on Restored,
  `src/game/cloudrunner_barrels.cpp`): she goes on the wind carrying too, and Treasure Lift leaves her to it. Played:
  carried across and up into the courtyard. The bottom leads north and south into the caves; east of it is a grate.
- The way to the race. In the hall east of the courtyard, on the fortress's floor (y 1866), a pool fills a pit at
  (-11960, 18867), its water at y 1824 over a tunnel that runs west under the courtyard. Swimming, no button dives (Z,
  B, A, R and L played). XYZAnimator (356, 32DDB) drains it on 0x4BE, the gold's (below), and sets 0x4BF once drained.
  The tunnel crosses map 12's TriggerPlane at (-12285, 1753, 18871), layer 0 to -1, and map 15's at y 1630, to -2,
  and drops into map 43, 1100 units east of the bikes. Played: from `--place -12068,1870,18867,0` with `--bits
  458=8448,2e1,43,34f,359` and 0x4BE set 3 s in (`4be@6`), the pool drained in 6 s and she walked west into map 43, at
  y 1263, 10 s later. Without 0x34F and 0x359, Kyte's Guardian scene pulls her into the courtyard and holds her there.
- The race. Its bikes are in map 43's group 1 (`--bits 4a3=7`). Beating the ClubSharpClaw sets 0x4BA. A beside
  CRSnowBike mounts it (0x49D, 0x497) and the race runs; a lost race (0x499) puts the player back beside the bike. Five
  rams on CRSnowClaw drop the SpellStone (CRSpellstone, 730), and picking it up sets 0x2E8, on which CRCloudRace (729
  0x0308) ends the race. A ram is CRSnowBike's hit of type 0x14, made only while the player rides it faster than 2
  (732 0x1188-0x1214); CRSnowClaw (728 0x0418) counts four then drops the stone at the fifth, if CRSpellstone is within
  500 units of it; the race is lost when the fuel runs out (732 0x0DB4, CRFuelTank adds 3000), not to time; stick Y is
  the throttle and B the one button read. The stone's scene plays, and as it ends, about 12 s after 0x2E8, the game
  sets 0x49A, though no call in code does (`dino.py calls bit 0x49a` finds only Bonehead's read); map 43's CFseqobject
  30CCA plays sequence 0x18 on it, a CloudRunner over the treasure, which sets 0x49F. The start's sequence (CFseqobject
  30c35, 0x206) launches the bike west out of the bay, over its bank into a pocket below it (x -13860 to -13665), and
  CRSnowClaw (389, at -13165, 18760) stands in the bay, so the way back in is east over the pocket's edge, which the
  launch crossed going down. The course of 31 checkpoints and 13 CRFuelTanks runs west and north of it. At the recomp's
  30 fps that edge stopped the bike dead wherever it came at it (tens of plays): CRSnowBike's wall test sees it only in
  a frame of fewer than 3 ticks, and a 30 fps frame is 2. Fixed, always on (`src/game/cloudrunner_race.cpp`, whose
  header has the code behind it): a short frame's wall test is made as one of 3's. Even so a bike that comes at the edge
  aslant slides along it, so it is taken head on, from the pocket's west end at z 18935. The rams are quickest backing
  off a second and riding at the claw again, and the fifth drops the stone by the claw, where the bike picks it up: a
  ram with a turn back past the stone's place (-13416, 18825) took 11-14 s, and two of six plays so ran out of fuel
  before the fifth. `tools/cloudrunner_race_input.txt` is the `--input` script that wins it, from `--story
  before-cloudrunner --place -13460,1262,18866,-2,0 --bits 4a3=7,4a0,4ba`, steering the bike with `ride` lines
  (main.cpp). Played four times, four wins: in the bay at 22-23 s, the stone at 42.5-45.1 s, 0x49A 9.7 s after it and
  0x49F at 65.4-68.1 s.
- Bonehead, CFGuardian (369) in map 43, keeps its state in the field 0x4B and starts from it: 6 with the power on
  (0x57), 9 with 0x43, 10 with 0x4BE (which needs 0x8D1 and 0x300), 12 with 0x4C1. At 12, 0x4B7 plays its sequence 11
  (map 43's TriggerPlane 30c34 sets 0x4B7 as the bay is entered, 30e8c clears it), and 0x49A moves it to 13; at 13 the
  SpellStone used on it from the item menu sets 0x4AB, its sequence sets 0x4AA, and it goes to 14. Played from 13.
- The dungeon (map 16, warp 12). Krystal starts in a cell; A held while walking into the green CFDungeonBlock pushes
  it, and she gets out behind it. The prison key (0x44) lies past CFPrisonGuard (375), who catches her within 150 of
  him and puts her back in the cell (his sequence, 0x574) unless she holds the key, 0x50 is set, or she casts the
  SharpClaw disguise (spell 0x40, her state 0x40): disguised, she walks past him to the key (played;
  `before-cloudrunner` gives 0x40). The three cell locks (dll308) take the key from the item menu and open their doors, 0x45 0x257, 0x46
  0x258 and 0x47 0x259. Bonehead's cell's lock has to be targeted from its east: from the west he, talking, takes A.
  The cage switches (CFCageSwitch, 384) take the key the same way and play their sequence, whose message 0xA0005 sets
  the switch's bit: the uncle's, at (-11708, 1368, 20410), sets 0x4D (played), and CFPrisonUncle (376) sets 0x50 as the
  dungeon next loads (played), so the guard lets her by from then on.
- The power. With his door open (0x257) Bonehead (CFGuardian in map 16) leads Krystal east to a room with a wall valve
  and gives her the power item, 0x60, as she walks into him (played). CFPowerLock in the fortress at (-11407, 17950)
  takes it from the item menu for 0x5F, which opens CFPowerDoor1 (played). The three power crystals are in baskets
  (map 12's group 7): lifting one breaks it (0x8B2 to 0x8B4) and A picks up its crystal (items 0x51 to 0x53). In the
  power room, whose radiation costs about 1 health in 3 s until they are in, each base takes its own crystal from the
  item menu (0x54 takes 0x51, 0x55 0x52, 0x56 0x53), each base's sequence sends 0xA0005, and with all three 0x4E0 and
  then 0x57, the power on, are set (played).
- The baby CloudRunners. Each CFCloudBaby (373) waiting to be found (map 12's groups 20 to 22, `--bits 458=7340032`)
  comes out only when Kyte's export 24 answers yes, a flag of hers nothing in the ROM sets, so on the prototype four of
  the five never come out. Found, it faces Kyte, and A within its reach rescues it: 0x901 counts it, its bit (0x24F to
  0x253) is set, and it flies to its golden perch (0x2FC to 0x300). Fixed (Baby CloudRunners on Restored,
  `src/game/cloudrunner.cpp`): Kyte finds a baby while she is within 400 of it. Kyte is loaded by the sideload objects
  (269), so a `--place` far from one has her nowhere (`--sidekick kyte` brings her). Played: 236A, from `--place
  -10428,1895,18659,0 --bits 43,458=7340032`, found with Kyte 240 away, rescued (0x901, 0x250) and on its perch (0x2FD);
  29FA, on a ledge 300 above the corridor, found with Kyte 382 away and held out while she stays near.
- The gold. Each rescued baby's perch bit is one of 0x2FC to 0x300, and CFLevelControl (400) sets 0x4BE (Bonehead's
  state 10) from 0x8D1 and 0x300. 0x8D1 is the +0x32 bit of the curve (429D9, at (-12672, 2239, 18806)) that
  PerchObject (718, 429D8) above CFKyteGoldPerch sends Kyte to: with Kyte out and Krystal within 134 of it, Kyte's
  command scroll (C-down, C-down rolls it, A uses it) offers its star, which sets 0x46E to 0xC0 (played), and Kyte's
  message 5 as she lands there sets 0x8D1. Map 12's group 8 plays Kyte's Guardian scene on the way (`--bits
  34f,359,2e1` skips it) and 0x4DF has the courtyard's guard beaten.

An object that wants an item (a lock, dll308, its item at +0x1E; CFGuardian) takes it only as used from the item menu,
never from the bit alone. C-right opens Krystal's items (dll1's menu 1), A uses the one shown and closes the menu, and
the object takes it in the frame the player presses A with it targeted: dll2 sets the target's +0xAF bit 0 then, with
the player inside its radius and within 100 units of its height. dll1's export 7 answers whether the item used is the
one asked for. In an `--input` script, standing at the object: A, C-right, A.

Left to play: 29FA's rescue (no way up to its ledge found), the babies 29B3, map 30's and map 15's (from the master
chest), Kyte landing at the gold perch (she stays by the sideload that loaded her, so 0x8D1, 0x4BE and 0x4C1 are read,
not played; the way to the race waits on 0x4BE too). Read from the ROM, nothing sets 0x4B8 (CFGuardian's sequence at
0x1D5C), and Krystal makes no hit of type 0xF, which the ProjectileSwitches by the main door take: the two ZBombs there
make it (their setup's +0x18). Neither is needed to finish (docs/restored-content.md, The plan). 0x4DC is TriggerPlane
31a68's, at (-9917, 1445, 18272), set on one crossing and cleared on the other, and CFLevelControl turns it into 0x4DD,
whose sequence sets 0x4DE.

## Golden Plains and the Desert Force Point

Tasks 69 to 74, played from `before-desert-force-point` on the desktop build a link at a time, each started with
`--place` and the bits of the links before it. World coordinates: Golden Plains (map 9) lies at cell (-6, 19), the
force point's top (50) at (1, 17) and its bottom (21) at (1, 16), both on layer -1 (a `--place` there gives layer
-1: at layer 0 the bottom's floor never loads and the player falls), SwapStone Circle (14) at (-3, 24).

- The portable hole spell. Its pickup crashed the game (us.toml's hook on dll3_9E58 says why); fixed, it is taken by
  playing (0x5BD). Cast at PortalSpellDoor (map 9 uid 34d9c) it sets 0x796, and the hole leads into the top.
- The top. Two torches (dll640) each want Krystal's fire bolt, the second within 0x7D0 ticks of the first, and set
  0x5E2. The door switch (643, uid 33eda) then takes the stone used from the item menu, plays sequence 0x78C and sets
  0x5E3, and DFPLift1 (641) rises to the player and carries them down to the bottom (about 48 s). Played.
- The bottom's power. A door switch at (692, -1459, 9922) takes the stone again (0x5E7) and opens the round door
  west. Walking west plays Kyte's cutscene (0x5F2, then 0x5EB). Four pads (fff, 639) each send a ScorpionRobot (0x427),
  whose death sets the next pad's bit, 0x5EE, 0x5EF, 0x5F0 and 0x5F1. With 0x5F1 DFP_LevelControl (638) turns group 4
  on and DFP_SmallDoor (uid 34662, north-west of the platform) opens into the room under it, whose force beams take a
  health a few seconds. Its DFP_ForceSwitch (308, uid 346d5), pressed with A, sets 0x5ED: the power flow shut down.
  Out again, the door switch south of the platform (uid 346bc) takes the stone, plays 0x78C and sets 0x5E8, and
  DFP_RoundDoor (uid 3393d) opens on a corridor south to the rotating ring puzzle (647). Played, every link.
- The place. The second SpellStone's place (DFP_SpellPlace, 652, uid 35111) stands on a platform at y -1255, 204 above
  the floor, and is in group 7, which the TriggerPlanes at the target block room's door (955, -1435, 8973) and at
  (1896, -1459, 8623) turn on as the player walks in (command 0x13; 0x14 turns a group off). In act 1 it takes the
  stone (0x2E8) used from the item menu within 100 units, plays sequence 0x373 and sets 0x5F3, then 0x78C, and 0x2E8
  goes at the sequence's end, some 34 s later. The platform's walkway is one surface with a closed duct at y -1249
  that rings the rooms in two C shapes, with zone planes, setup points and lights along it. Eighteen stubs leave the
  duct and open high in the corridors' walls, with no collision on their mouths: a lip is at -1286, the floor at -1459
  and the slope under a lip stops at -1395, and no surface in the ROM joins the floor to a lip. The force field under
  the platform (DFP_ForceAway, 646 kind 3, uid 347f2) takes two health every 200 ticks and lifts nobody, and
  DFP_Textblock (654) and DFP_Platform1 (655) move only from 0x7BF, in act 3. Desert Force Point on Restored stands a
  second DFPLift1 (641) under the stub at (-275, 9280), in the corridor behind the round door, with that corridor's
  travel (`src/game/force_point.cpp`). Played on Restored from the corridor (`--place -324,-1459,9660,-1` with the
  power's bits, 0x5E2 to 0x5E8): onto the lift, which rises once the player stands on it, off it into the stub, along
  the duct and the walkway to the place, A, C-right, A: 0x5F3, 0x78C, and 0x2E8 cleared; then back along the duct
  and down the lift to the corridor. The lift waits at the top while the player is off it and takes them down again,
  as the force point's own lift does. On some of those plays the game crashed a second into the sequence, drawing an
  effect whose object was freed (`src/game/objects.cpp`, fixed). On Original no lift stands there.
- The acts. DFP_LevelControl keeps the force point at act 1, and nothing moves it to 2
  or 3. The other place (uid 40323, east) takes a stone only with 0x7A8 set: in act 1 the stone carried as 0x2E8
  (played with 0x7A8 set: 0x5F4 at 17.5 s), in act 2 the one carried as 0x123. 0x7A8 is DFP_TargetBlock 401bb's
  landing, the end of a chain only half of which a player can reach: 0x7A0, which nothing in map 21 sets, lets a door
  switch (uid 40143) take the stone for 0x7A1, the pit switch (401a8) then sets 0x7A7, and the block slides to 0x7A8,
  which also plays DFP_seqpoint 4031e's sequence 1. Its twin in the west starts at 0x681 (group 6, and the pit switch
  3461b's 0x69A) and ends at 0x766, Kyte's cutscene by the pit (DFP_seqpoint 35104) and the duct's lights. With
  Kamerian Dragon on Restored the dragon under Dragon Rock sets 0x7A8 as it dies, so the place takes the stone
  carried after Dragon Rock (`after-dragon-rock`).
- The mirage. Golden Plains' level control (GP_LevelControl, 547) sets 0x5F3 each time the map loads (dll547_0018,
  0xBC-0xC8), so a player who comes through Golden Plains finds the place holding a stone already, and the stone stays
  carried (0x2E8), which the swap to Sabre refuses. Desert Force Point on Restored keeps 0x5F3 as it is there. The two
  TriggerPlanes at the force point's door (map 9 uids 4031d and 4032c) wait on 0x5F3: crossing 4031d at
  (1141, 334, 12593) sets 0x651, and GPSeqObject 348b8 plays the mirage (a cloaked figure, Krystal, Kyte) and sets
  0x652; 4032c, by the hole, sets 0x7B7 for GPSeqObject 4059e (0x7B3). Played, 4031d.
- The swap to Sabre ("RETURN TO SWAPSTONE CIRCLE"). The stone (SHswapstone, 515) and its pad (SHswaplift, 514, uid
  42238, where warp 14 lands) are in group 0 of 0x4A6, which the circle's planes turn on as the player walks in. A with
  the player on the pad (its hit list holds the player) sets 0x3EB, and SHseqobject 41fd9 plays sequence 0x107: the
  pad lifts the player to the stone, which meets them the first time ("Hello there little fellow, I'm Rocky.", 0x883)
  and opens SWAP, SHOP and WARP. Stick left, then A, swaps to Sabre (0x885, or 0x884) and warps to SwapStone Hollow;
  it is refused while a spirit, 0x2E8 or 0x123 is carried. 0x883 only skips the meeting the next time, and 0x4B8 only
  moves the circle's act (sequence 0x2AB, at CloudRunner's race). Played with 0x883 clear (`--story
  before-desert-force-point,14 --bits 2e8=0,5f3=1,652=1,4a6=1`, A, then stick left and A): 0x3EB, 0x883, 0x885, and
  Sabre in map 7.

## The bits

Each with what shows it, by the DLL that reads or sets it (`dll <n>`) or the task the table in file 55 gives it.
`tools/dino.py calls bit <bit>` lists every call that sets or reads one with an immediate bit, and a name added to
`syms/names.toml` shows wherever the tool prints it.

`to_first_shrine`, what the game has set by the first shrine:

- 0xD0: task 3, Randorn. Set by WM_Wizard (598), read by SB_KyteCage (581) and SB_CageKyte (583).
- 0xB4, 0x2B2, 0x70: tasks 4, 5 and 6, the warp crystal. 0x70 is read by IMIceMountain (408).
- 0x37A, 0x4D3, 0xAB: tasks 7, 8 and 9: Prince Earthwalker, the race, the tunnel.
- 0xBF: task 10, SwapStone Hollow, set by SHqueenearthwal (507).
- 0x15A: task 12, read by SHswapstone (515).
- 0x3A0: task 14, swapped to Krystal.
- 0x88, 0x3B0, 0x2C5, 0x615: tasks 15 to 18 in Cape Claw: the bribe (CCguardgiving, 447), Lightfoot (CClightfoot, 449),
  Kyte, the courtyard (0x2C5 and 0x615 are read by CClevcontrol, 460).
- 0x2D, 0x1D7, 0x40: spells, the bits that show them on the spell scroll (dll1's menu 6), and the value its message
  0x80002 hands the player (dll210 0x1DDC), held at her state's +0x87C (dll210's export 50). CClightfoot sets 0x1D7
  with task 16, "COLLECTED FORCEFIELD SPELL". WL_LevelControl (607 0x1118) sets all three in its first update each
  time it loads, until the fourth spirit (0x318), so Krystal has them from her first visit to Warlock Mountain; the
  MMShrine's and WGShrine's controls (466, 488) set 0x40 too. 0x40 is the SharpClaw disguise: cast, it puts her in a
  SharpClaw's shape and drains her magic, and CFPrisonGuard (375) and ClubSharpClaw (215) let her by.

The spirits, set in this order by WL_seqpoint (611) and read by WL_LevelControl (607):

- First: 0x316, 0x21F. Second: 0x277, 0x21B. Third: 0x317, 0x21C. Fourth: 0x318, 0x21D.

`first_stone`: 0x104 (task 29) and 0x22E (task 39) on the way there, and 0x877, placed: VFP_SpellPlace (637) in the
Volcano Force Point takes it as its placed bit, with 0x123 as the stone carried. And 0x4E9, which VFP_LevelControl
(619) sets on that visit and Diamond Bay's act counts (The maps' acts). Carrying is 0x123 or 0x2E8, which
SHswapstone refuses a swap on ("You cannot swap when holding a SpellStone"), so a point leaves both clear.

SwapStone Hollow's own, as played: 0xC2 counts the white mushrooms given to the Queen Earthwalker and 0xB0 is her
quest's step. Given them at step 4, five or ten, she plays her sequence, sets 0x486 and step 5, and the game warps to
the Volcano Force Point (map 4) for its cutscene; 0x487 and step 6 come with a second visit there, and step 7 back in
the hollow. 0xC3 starts Tricky's run to her (SHtricky, 501, and an SHseqobject, 309, wait on it), and nothing in the
game sets it: `patches/swapstone_hollow.c` does, with Tricky's Run on Restored. 0x887 is the SwapStone's zone
(SHswapstone answers only while it is set), kept by TriggerPlane 40b79 (`us.toml` says how).

The way on to the first SpellStone is down the hollow's river: the only warp into the Volcano Force Point is the
Queen's (entry 81, dll507 0x4F8), and map 4 borders only Diamond Bay. The river is water at y -797 over a bed 3 to 38
below it, and Sabre wades it: from its last bend (`--place -8160,-790,-2340,0,22665`) he is in Diamond Bay (map 35)
in 5 s and drops into its pool by the dock 41f04, which spawns that map's own log. The task list puts a log between
the Guardian's activating the SpellStone (0x22B, dll496 0x2254) and Diamond Bay ("BOUGHT LOG FROM A THORNTAIL",
"RIDE THE LOG DOWNSTREAM"), but those tasks have no bit in file 55 and nothing completes them, no ThornTail (508)
sells anything, and the hollow has no dock, flow or log: SHriver (uid 0x30934, local 2350,-713,606) is a placement
with no code. A dock placed there with flows down the river gave a log that was boarded and never moved, on a bed
a tenth of Diamond Bay's depth. A BWLog (793) moves only on DFriverflows' pushes: A while riding makes the rider hop
(0x2B8), the stick turns it, and A twice with the stick to a side gets off (dll793 0x1600).

## Object groups

A map's objects come in groups, and most of a level's enemies and props load only while their group is on. Each
object's setup in ROM file 31 has a flags byte at +4 and its group at +6: with flag 0x10 the object loads only while
bit `group` of its map's group field is set (`tools/dino.py map <map>` prints the field and each placement's group).
A map's field is a story value of 32 bits, numbered by the u16 at
+0xF0 + 2 × map in dll29's data, and dll29's export 18 reads it. Some maps' fields: 0x480 Discovery Falls (6), 0x4A6
SwapStone Circle (14), 0x42E Moon Mountain Pass (18), 0x458 CloudRunner Fortress (12), 0x45A its treasure caves (15),
0x4A3 its race (43), 0x3B7 Cape Claw (29), 0x601 Swap Store (51), 0x5DC Dragon Rock - Bottom (52).

A story point turns no group on, so a level started from one has only its ungrouped objects: Moon Mountain Pass's
Lunaimars are in groups 3 to 7, Cape Claw's worm in group 10 and its SandEels in 11 and 12, and the Swap Store's
shopkeeper in group 0 and its item beams in group 1 (`--story new-game,86 --bits 601=2`). `--bits <field>=<value>`
sets the field, its value in decimal: `--bits 42e=-1` turns on every group of Moon Mountain Pass, more objects at once
than play ever loads, and `--bits 42e=385` groups 0, 7 and 8, where the pass's Transporter lands.

A TriggerPlane turns a group off as the player crosses it, and the group's objects are freed then, whatever holds
them. SwapStone Circle's small baskets and crates are in its group 1, which the planes at about (-50, 377, 17013) and
(220, 325, 17301) turn off, in world coordinates (the map lies at -1920, 15360). Dragon Rock's fight is in groups 0
to 2 (`--story new-game,88 --bits 5dc=7`): DR_Creator (738) sends Skeetlas (615), at most 20, at DR_EarthWarrior
(713), in group 2 at (1576, -1831, -6066), the only object they go for there.

## The maps' acts

Besides its groups, each map has an act, a story value of 4 bits that dll29's export 15 reads and export 16 sets,
numbered by the u16 at +0x1F90 + 2 × map in dll29's data: 0x4FE the Volcano Force Point (4), 0x3EE Diamond Bay (35),
0xE6 Walled City (13), 0x8EC Krazoa Palace (3). A new game puts every map in act 1 (dll29 0x300-0x334), and the object loader
(`func_8004B4A0`) skips a placement whose byte +3 has bit act - 1 set (byte +5's bit 16 - act for acts 9 to 16, and
`tools/dino.py map <map> --act <act>` lists what loads), so a map holds
other objects in each act. A level's control object sets its act, or a sequence does (dll3's event 0x1D, the act of
the map the playing object is in), and ROM file 13 holds one such event, which sets act 3.

`src/game/acts.cpp` moves three maps' acts with the story, at each map load (the header has the evidence), and
`DINO_LOG_ACTS=1` prints each act it moves or leaves:

- The Volcano Force Point: act 2 from the King's activation of Walled City's SpellStone, 3 once it is placed (0x878).
  Act 2's level control hands the stone over (0x83B) 130 ticks after the level control's DLL loads. Its second
  SpellPlace, the podium that takes the stone for the second path, the door switch and the round door load in act 2
  only, and VFP_LevelControl (619) set the map to act 1 in its constructor, whatever the story, so nothing of act 2 or 3
  ever played. Its act 3 sets 0x63C, which moves Diamond Bay on to act 3 (DBlevelControl, 663: 1 and one each for 0x4E9,
  0x63C and 0x63D).
- The Volcano Force Point's act 11 is the Queen Earthwalker's: she sets it before each warp to entry 81 (0x486, then
  0x487), and only it loads the two SHseqobjects that play her cutscenes (30afa, done bit 0x484; 30afc, 0x485) and a
  WarpPoint, without the level's podium, SpellPlaces and puzzles. It is left while a visit is to play, and once both
  have played the map's next load moves it on, as VFP_LevelControl would, which neither visit loads. Played from
  `--story swapstone-hollow --bits b0=4,c2=5,452=450` (the Queen beside warp 15, chased with A pressed): both
  cutscenes on Original and Restored (0x484 at 44.5 s, 0x485 at 65.9 s, then step 7), and on Restored warp 81 after
  them with the first SpellStone (0x123) loaded act 1, whose podium beside the entry took the stone (0x514 at 5.5 s,
  from `--story swapstone-hollow,81 --bits 4fe=11,484,485,486,487,b0=7,123`). On Original that warp finds act 11.
- Walled City: act 2 once Moon Mountain Pass's spirit is released (0x317), when the task list sends Sabre back for
  the Sun and Moon Stones. The stones beside the boss door, the sockets on the TRex statues that take them, and
  WCLevelControl's (779) bits that open the Sun and Moon temples are act 2's only; the RedEye teeth are act 1's.
- Krazoa Palace, with Krazoa Palace on Restored: act 14 loads as act 1. Cape Claw's TriggerPlane 30ec4 (acts 1, 12,
  13, 15 and 16) sets Cape Claw's act 2 (0xEE) and the palace's act 14 as Kyte is freed (0x2C5, task 17), and act 14
  loads 151 of the palace's 415 placements, none of its own: not its two KP_Transporters, setuppoints, crates or
  mushrooms. Act 1 loads all 415.

A map's act is kept in the save, so a death or retry restores the act the map had when the level started, and the
map's next load moves it on again. Diamond Bay moves by itself: act 2 from the first SpellStone (0x4E9) holds the
Pointback's second egg (DBPointMum, 658: step 0x3FF, then the egg, 0x39E), and act 3, the nest and the stealer worms,
comes with 0x63C once the volcano has been entered in its act 3.

## The third SpellStone and the moonseeds

Tasks 84 to 106, as played from `before-third-stone`, with Third SpellStone on Restored (on Original the volcano and
Walled City stay in act 1 and no moonseeds come). An item is used as a player uses it: C-right opens the
inventory with it selected, and A uses it on what is in reach (`--input` lines `5 0001`, `5.2 0000`, `6.5 8000`,
`6.7 0000`).

- The volcano's second path: the podium of act 2 (VFP_PodiumPoint, 633, at -14788,-1568,-1161) takes the stone
  from 100 units, plays its sequence, which opens the round door down a corridor, and the sequence's event sets
  0x515 (dll633 0x3EC, by the map's act).
- The third SpellStone: the SpellPlace of act 2 (637) loads within 224 units of (-14849,-1169,-1279), above the
  core, and takes the stone with no reach of its own: it sets 0x878, and `src/game/moonseeds.cpp` gives four
  moonseeds then (0x86A, which nothing in the ROM raises). The SpellPlace's seqpoint (630 @1380) holds 0x878 as
  both its trigger and its done bit, so by its code its sequence, the only one that sets 0x63C (sequence 1032), never
  plays; VFP_LevelControl's act 3 sets it instead.
- The moonseeds: each of Moon Mountain Pass's four MMP_mseedrecept (722, groups 2, 3, 4 and 6) takes one used on it,
  lowers 0x86A and sets its planted bit (0x868 for the one at -7692,1006,15565, in group 6). Krystal's inventory (dll1's
  list 0) shows the moonseed while 0x86A is above 0, and A uses it on the receptacle only while she faces it and has it
  as her target (dll722 0x2CC, its use flag 1 and dll1's export 7 on 0x86A). At night (dll7's export 8, 21 to 5) a
  planted receptacle waits for Kyte: in her reach with her out, it offers her command 4 (use flag 4), her flame, the
  second row of the scroll C-down opens, and that command sets 0x46E, Kyte's curve, to its setup's +0x1A (0xAD for
  41a5c), the curve that takes her to it (0x6A0-0x6F0). Her flame takes it to its state 3 and sets 0x85A (export 7,
  dll722_0D40), the trigger of the MMP_seqobject beside it (419CC, sequence 10), which grows MMP_moonseedbus and
  MMP_moonseedvin (0x853, 0x855, then 0x852, 0x854) and sets 0x85B. The vine climbs from the receptacle's hollow to a
  ledge at y 1099 east of it (-7650,15571), which drops into the canyon that the pass's Transporter (warp 73) opens on.
  Each of the other three receptacles has its own bush, vine and sequence (0x856, 0x864, 0x858), and its own curve for
  Kyte (0xAC, 0xAE, 0xAA). Kyte comes out of the pass's sideloads, and its TriggerPlanes move her from curve to curve
  after Krystal (0x46E 164 to 166 as she walks, their action 0x1C).
- The test of fear: the pass's Transporter (2BA7, at warp 73, setup `0049 2b00`) sends Krystal to MMShrine (map 32,
  warp 43) by A, about 20 s after an arrival, as EC's does, with or without the meteor's bits (0x803, 0x804).
- Walled City in act 2 loads the Sun and Moon Stones (WCSunStone, 272), the TRex statues' sockets (785) and its
  level control's act 2 (779).
- The volcano in act 3 (`--story before-third-stone,81 --bits 878,500=1024`, group 10 holding VFP_LevelControl):
  its act-3 timer (130, less the frame's time each frame, 0x69C) runs out 2 to 4 s in and sets 0x63C and 0x83C, and
  Diamond Bay goes to act 3. That was three runs of four (4.2 s, on Third
  SpellStone's Restored): in the other, nothing was set in 48 s, and why is not known.

Played from here on the desktop build, each from the one before it but where a `--place` or `--bits` start is named:

- The second egg (Diamond Bay, act 2): Sabre at the mother's head, the far side from the way in, talks to her (A),
  which plays her sequence 3 and sets 0x3FF; DB_FlameSwitch rises with it, Tricky's flame from beside it lights it
  (0x3D9), the bridge rises, the light beams carry Sabre to the egg (0x86D as A takes it), and the egg used on her
  from the items menu sets 0x39E (`src/game/diamond_bay.cpp` has the steps and what stopped each).
- The moonseed (tasks 92 and 93): on Restored, at night, from the receptacle's hollow (`--story before-dragon-rock
  --place -7725,1030,15565,0 --bits 86A=4,42e=64 --time 22 --sidekick kyte`): Krystal faces the receptacle 41a5c (a `to`
  line at strength 0.5), A, C-right twice (the food bag is the first row), A plants a moonseed (0x86A 4 to 3, 0x868);
  C-down twice and A send Kyte's flame (0x46E 0xAD), and 9 s later the seed grows (0x85A, 0x85B, the bush and the vine).
  Krystal climbs the vine with the stick up (acts 27 and 28) to the ledge above, and steps off it east into the canyon.
  A walk to the receptacle with Kyte from a sideload was not found: the canyon's slope up to that ledge is a drop the
  other way, a walk from warp 73 stops at its foot (-7389,924,15535), and one from the pass's west setuppoint (41324,
  -8629,1028,15874) at the moon rock's ledge (-8131,1066,15569).
- The test of fear's way in: from warp 73 (`--bits 42e=385`), A on the pass's Transporter 24 s in lands Krystal in
  MMShrine (map 32) 16 s later. The test itself is the shrine's (The shrines).
- The meteor (task 94): the three MMP_moonrock carried into vents 3, 4 and 6 count 0x88C to 3 and set 0x89B, and
  with 0x803 the meteor's sequence plays (0x804).
- The night: the sun temple's inner rooms played through (0x2B0, 0x2F8, 0x2D1, 0x2D4) give the Sun Stone (0x201).
  In the moon temple at 22:00, with what its push blocks and symbol set already set (`--bits 813,25d,829,244`), the
  Moon Stone shows and is taken (0x264). Each temple's bridge of light holds Sabre (`src/game/temples.cpp`).
- The moon temple's way in, played as Sabre in one run from the east courtyard by night (`--story before-walled-city
  --place -6008,-976,-7336 --bits 317,36a=136 --time 21@4`, `tools/moon_temple_input.txt`, whose header has the pushes).
  The push blocks are pushed only by night (dll7's export 8), and only from inside the grid, which runs wall to wall:
  six pushes land the four blocks on the moon tiles, 0x811 counts to 4, and 0x813 and 0x25D set with the last, 45 to 57
  s in over nine runs. The pollen cannon at the grid's north-west corner (WG_PollenCannon 41b22) knocks Sabre out of a
  push now and then, so the script presses A again until the block moves. He waits at the lift's foot (-5952,-6720) for
  the next night (`sync hour`), and steps on the lift as the clock reaches 21:15: it carries him up as he steps on, and
  once it stops on top, 196 from the symbol, C-up and the stick held a little down (-0.4) tilt the first person view up
  at it as the clock nears 22:00, which sets 0x829 and 0x244 about 254.6 s in and opens the way in: three runs of four
  (the fourth, played without the symbol's report, stayed dark and shows no why). The lift moves only for Sabre (object
  id 0, dll786_0030), and it waits 5 s at the top before it goes down again; C-up pressed before it stops does nothing.
  The symbol lights by how near the screen's middle it is and how near 22:00 the time is (`src/game/temples.cpp`): the
  view's tilt follows the stick and goes back to level when it is let go, the whole way down tilts it past the symbol,
  and looked at level, at the same hour, it stops at about 180, under its 200. It takes the light only once it has
  taken the door's 0x25D, which the ROM's symbol, loaded with the courtyard before the door opens, did only as it loaded
  again (docs/restored-content.md, Plain bugs).
- The sockets: from the chamber under the statues (`--place -8000,-1080,-7400`), the Sun Stone used on the sun
  statue's socket (-7908,-1102,-7220) sets 0x202 and the Moon Stone on the moon's (-8091,-1102,-7219, by way of
  -7990,-7380 and -8088,-7285) 0x243. The sockets take an item from 48 units. The chamber's bats take the target
  from a socket, so a script uses the stone again until the bit is set. With both, EC's Transporter stands at
  (-7939,-1123,-7441); its tutorial hint (0x912) holds the player still the first time, and A on it about 20 s after
  the stones sends Sabre to the EC shrine (map 33, entry 40).
- The test of skill: TriggerPlane a2b in the shrine's corridor sets 0x592, the ECSH_Creators' enemies come, and
  beating them sets 0x593 to 0x595; with 0x595, crossing z -14008 sets 0x596 and the door opens. On the ring,
  ECSH_Shrine (dll469, 1619 at 14077,44,-14551) plays three rounds (its +0x1B 3, 4 and 5) of six cups (ECSH_Cup,
  dll470): each round draws a cup (+0x1A), shuffles (+0x17 phases 6, 8, 2, 1, 0, 4) and waits about 16 s in phase 5
  for the player to stand within 30 of a cup, whose index is its +0x23. A player who stands among the cups as they
  shuffle stops them for good, so a script stands in the middle until phase 5 and then walks to the cup
  (`--input`'s `chase 1BF 10 match 23 1B9 1A`). The third right pick sets 0x128, the win gives the spirit (mask 8)
  and 0xFD, and Sabre is back on Warlock Mountain's pad, where its act 5 sets 0x142 (`src/game/krazoa.cpp`).

From the pad to EC's release (WL_seqpoint 2983 at 13929,-117,1106, mode 2 on 0x142), played from the end of the pad's
walkway east along z 2960, past the crates and two GuardClaws, at (13700,-300,3320): a fallen column (WMfallencol,
dll331, uid 11 at 13743,-292,3090) lies across the way north in acts 5 to 13, and nothing moves it (dll331 only turns
it as it is made, and its update and exports never test its object), nor goes round it: the pool south of the corridor
ends in a wall at z 3818, burned away or not by the vine's 0x2DB, and the top Transporter's room (entry 6) is shut.
Krazoa Shrines leaves it out of acts 5, 7 and 9 (`src/game/krazoa.cpp`). Past it the way is the ROM's: a ramp up west
to (13558,-205,2734), and the hall east along z 2735, which the three WL_LaserBeams (dll594, 291f, 2923 and 2924, at x
13753, 13951 and 14155) cross from the north: each beam holds the player back while it is on and lets him by when its
timer turns it off (a few seconds to 16 s), and none is on once its WL_LaserTarget (dll595, high on the north wall)
is hit and sets its bit (0x2E2 to 0x2E4). The hall rises east to (14430,-99,2740), and north of it TriggerArea 2972
sets 0x2EE, which WL_LevelControl (dll607 0x13B0) turns into a warp back to entry 57 (13553,-170,2732) unless the
player's spell is the SharpClaw disguise (0x40) or the forcefield (0x1D7), or 0x2F3 is set. C-left, C-left and A
cast the disguise, which drains magic (about one a second and a half) and keeps the warp off, but the two
ClubSharpClaws and the GuardClaw stand across the way at z 2068 and hold the player there, disguised or not. What
lets them go is LaserTarget 29c1 (14429,237,2152, high above them): its bit 0x2FA sets 0x2F7, which frees them, and
after a while 0x2F3. Played in one walk from (13700,-300,3320): the ramp, the hall, the disguise and the guards'
hold. Played from 0x2F3 on: WL_seqpoint 2973 (mode 3, radius 200) plays sequence 8 and sets 0x2F2; up to y
13 and west along z 1600, 297b (radius 101) plays sequence 9 ("Father.") and sets 0x2F4; down the walkways
(WM_Walkway1 and 2, dll331, at y -44) to the release, whose sequence 0xA sets 0x21D, 0x318, 0x2F5 and 0x2F6. Acts 7
and 9 need the same way past the column: WG's column (WL_colrise @1AB0, 14433,-101,2028), which carries the player
up to WG's site in act 7 and NW's in act 9, stands where 2973 does. Played: in act 7 from (13700,-300,3320) through
the hall and onto the column, which rises to y 22; in act 9 the same walk to its pit, and from the hall's end onto it,
to y 25 (acts 7 and 9 have no guards and no TriggerArea 2972). CC's column (@1358, act 6 and GP's act 8) and MM's
release (act 4) are west of the pad.

Not played from here: a walk into the moonseed's hollow from where a player can be with Kyte, and the other three
receptacles' seeds; task 84's log (no ThornTail sells one, as for the first); and a hit on a WL_LaserTarget (dll595, its
hit is func_80025F40's): a dozen of Sabre's aimed shots at 29c1 from z 2330 to 2700, and some at the hall's, set no bit.

## The shrines

Each shrine has its own DLL: DFSH 433, MMSH 466, ECSH 469, GPSH 472, DBSH 477, NWSH 481, CCSH 486, WGSH 488. Its
constructor sets 0x129, clears the shrine's own bit (0x125 DF, 0x12B MM and GP, 0x143 EC, 0x15F DB and NW, 0x1CF CC,
0x1D2 WG) and sets 0x126 and 0x127. 0xFD is read and set near the end and 0x128 at the end.

## The sand worm

CCsandwormBoss (461) is in map 29 at (2155, -276, -206). Map 29's record in ROM file 21 is at grid cell (2, 26) of 640
units, so that is (3435, -276, 16434). Bit 0x3FB says it is dead, and then 0x3D4 picks how far it has gone. Three
SandEels beside it hold the bits its constructor reads: 0x1EB at (1661, -263, -296), 0x1EC at (2040, -270, -189) and
0x22A at (2051, -268, -341). The worm is in Cape Claw's group 10 and the SandEels in 11 and 12, which a story point
leaves off (Object groups), so `--bits 1eb` alone loads none of them. With all three bits set its constructor starts
it at state 2, and its first pass moves it to state 4, the fight:

```
DinosaurPlanet --boot --story before-sand-worm --bits 3b7=7168,1eb,1ec,22a --place 3620,-265,16205,0 --report
```

Each SandEel's bit (+0x18 of its setup) is set by the eel itself once it is beaten (dll216's state at 0x34A0), and
the worm moves the eels' groups by them: in state 0 it waits for 0x1EB with group 12 on (that eel), then turns group
12 off and group 11 on (the other two) for state 1, and turns group 11 off once 0x1EC and 0x22A are both set, for the
fight. Its constructor rotates them each time the map loads: with 0x1EB set and not all three, it clears 0x1EB and
sets 0x22A, or 0x1EC once 0x22A is set, so the first eel is played again with one more of the other two beaten, and
that one loads hidden when group 11 comes on. A hidden eel with its bit set stays hidden (us.toml's hook on
dll216_02C0 says why it needs one). The command above has all three groups on with all three bits set, which play
never has: the eels load hidden beside the fight. `--bits 3b7=1024,1eb,1eb@30` plays the map loaded again after the
first eel: the constructor clears 0x1EB and sets 0x22A, and 0x1EB set again at 30 s, the first eel beaten again, has
the worm turn group 11 on with that eel hidden.

## DarkIce Mines' SnowHorn and Tricky's flame

Tricky's flame (0x245, command 4 of his scroll) is DarkIce Mines' to give, and the ROM gives it as it stands:

- Map 19 places four DIMSnowHorn1 (dll712, object 0x18F). Each placement's byte +0x19 (its state +0x904) says which
  it is, and its picker (act 0, 0x1D68) starts that one's acts: 0, uid 158c, is the shackled one, 1, uid 1a5e at
  (-6602, -1269, 5259) in group 8, teaches the flame, 2 is uid 2596, and 3, uid 302e9, is the one Sabre rides (act 8,
  where A sets 0x3E3 and 0x5BA).
- 1a5e's act 5 (0x2470) takes it out of the targeting (obj+0xAF bit 8) until 0x172 is set. Then A by it plays its
  sequence 0, which sets 0x245 and 0x27, and from 0x27 on its act 6 takes an Alpine Root (0x170, which DIMAlpineRoots
  16a4 and 16aa count up) for sequence 2 and 0x28, and its act 7 the second for sequence 4 and 0x16F, after which it
  is ridden.
- 0x172 is the SnowClaw's beside it (dll215, uid 1602 at (-6699, -1269, 5277), group 6): its placement's +0x18, which
  dll33.21 copies to its state at +0x39C and its death (dll215 0x49E4) sets. `tools/dino.py calls bit` lists only bits
  given as constants, so it shows nothing setting 0x172.
- Groups 6 and 8 are on with bits 6 and 8 of 0x493 (`--bits 493=320`), as a TriggerPlane (uid 30c55) at (-7467, 5088)
  turns them on.
- A uses the SnowHorn within 60 of its head at (-6664, 5263), west of its body: the targeting (dll2 0x1FA0) wants the
  player within 4 times its focus record's reach of the focus point in use, and up to 100 above or below it (dll2
  0xE58). After a fight Sabre holds his staff and A swings it, until Z puts it away. `DINO_REPORT_OBJECT=18F` with
  `--report` prints each SnowHorn's act, use flags and focus point.
- Played from `--story swapstone-hollow,55 --place -6640,-1269,5420,0,0 --bits 493=320` with
  `tools/darkice_mines_input.txt`: the SnowClaw fell to A at 8.7 s, which set 0x172, and the SnowHorn's talk set 0x245
  and 0x27 at 19.7 s. With `--bits 172` and no fight, A by its head set them at 10.0 s.

## Walled City and BOSS TRex

`--story before-walled-city` starts Sabre outside Walled City's gate (its setuppoint 0), and `before-walled-city,91`
where BOSS TRex's way back comes out. From there to the third SpellStone, as played:

- The beacons. Each of the two pressure switches (WCPressureSwitc, 781) arms its beacon (WCBeacon, 780) for about a
  minute (0x7EF, 0x7F0), and any hit lights an armed one (0x7F9, 0x7FA). With both lit, WCLevelControl (779) plays
  its sequence 0, which sets the slab door's locks 0x7F7 and 0x802 (779 0x12F8). The door leads down to the chamber
  under the city (group 4). A switch that was stood on crashed the game, and no attack reached a beacon
  (`patches/walled_city.c`).
- The teeth. The two RedEye teeth (WCTrexTooth, a collectable, 272) lie in that chamber, at local (856,-1112,-4932)
  and (1011,-1114,-4933), and A picks one up: 0x81E the sun's, 0x81D the moon's. Picking up any item hung the game
  (us.toml, beside dll3_8900). Bats fly there and take A's target, and a sword swing steps the player out of a tooth's
  pickup radius of 15, so the player stands on the tooth with no bat in front.
- The statues. A WCInvUseObj (785) beside each statue takes its tooth: C-right, then A with the tooth chosen. It plays
  the statue's sequence and sets 0x25A (sun) or 0x25B (moon).
- The boss door (WCBossDoor, 307) opens a half on each of 0x25A and 0x25B. Sabre then walks down between the statues
  to a plane, which sets 0x81B, and to the WarpPoint (275) it shows, which goes to warp 90, BOSS TRex (map 48). The
  door's sequence would set 0x819 at its event 2 (307 0x5A4-0x5E4), but nothing on the way needs it.
- BOSS TRex. The arena is a square ring of four corridors (east x -12703, west x -14177, north z -8222, south z -9697),
  with a laser wall (KT_Lazerwall, 704) across the middle of each and two floor switches (KT_RexFloorSwit, 703) in each,
  one either side of its wall. The TRex (KT_Rex, 702) has 4 health and runs the ring on a path. Two corridors are live,
  east and west (0x55A) as the fight starts or north and south (0x55B), and the TRex pauses (its acts 5, then 4, in
  dll18's state at +0x26C) and charges (act 1) when the player is ahead of it on its way round, often a corner or two
  away. A player standing on a live switch charges its corridor's wall (0x56A-0x56D, a step each 10 frames to 15, about
  2.7 s). At 15 the wall fires (0x560-0x563) for 120 frames (dll704 0x144-0x1F4), and the switch clears its charge and
  swaps the live pair (dll703 0x5F0-0x6B0), so a wall fires once a round and the player times it: on the switch about 3
  s into the pause's act 4, whose charge starts 5.6 s in, beyond the wall from the TRex. The TRex charging into the
  firing wall is thrown back (act 6, 2.8 s) and then stunned (act 7) for 600 frames. A sword hit on its head (its hit
  spheres 2 and 3, dll702 0x2010), which the stun lowers from about 330 up to the ground about 370 ahead of its feet,
  then costs it one health and ends the stun at once. After the stun, hit or not, its act 8 stamps for about 10 s and
  its contact reaches about 260 from its middle, a hit every 2.2 s that holds the player stunned, so a player swings at
  the head and steps back. Its stomps (0x554) drop rocks round the player (KT_Fallingrocks, 706). The round (0x572)
  counts on as the stun after a hit ends, and again at the end of the fast lap that follows, which turns the next
  round's switches on at new places nearer the walls (dll324's curves, by 0x572), and the TRex turns round, sometimes
  twice. In the last round the switches stand within the TRex's contact of the wall, so the wall has to fire before the
  TRex reaches the player: on the switch 2.5 s into act 4. As played, the rounds' walls were the east, south, west and
  south ones.

  Four things stopped the fight. The door behind the player (KT_RexDoorPlaye, 307) shut it out of the arena as the intro
  ended (us.toml, beside dll210_0934). A switch's collision mesh lies 28 off the pad drawn round it, so a player
  standing on the pad never charged it, and no wall ever fired. After the first hit the TRex went round its corners in
  place and never charged again. And a charge that ran on round a corner kept the TRex's facing, so it ran sideways, its
  later corners swung it out of the ring, and a stun could leave its head beyond the corridor's outer wall
  (`patches/boss_trex.c` has the last three, with their evidence). Once beaten, it loaded the front end and gave no
  stone. With BOSS TRex on Restored it gives the stone as won (0x82E) and opens the player's door and the WarpPoint
  (0x81C) back to warp 91 (`patches/boss_trex.c`). The beaten TRex lies across its corridor, so a player beyond it from
  the south-east corner, where the door is, goes back the other way round the ring. `DINO_REPORT_OBJECT=3D9` with
  `--report` reports where the TRex is and its act, and `sync act 3D9 <act>` in an `--input` script waits for one.
  `tools/boss_trex_input.txt` is the `--input` script that wins the fight and walks out to the WarpPoint, from `--story
  before-walled-city,90` with `DINO_REPORT_OBJECT=3D9`; its comments say how each round is timed.

The task list has more here than the ROM:

- Tasks 76 and 77, the Queen and Tricky's mother: the Queen (WCQueenEarthWal, 773) has code, but no map places
  her, and she reads and sets no bit.
- Task 78, ten RedEye teeth: the chases (WCTrexChase, 777, and WCSabreChase, 778) are empty DLLs, placed nowhere.
  The two RedEyes (WCTrex, 775) give no tooth, and the two teeth above are all the ROM holds.
- Task 83, the King activating the stone: the King (WCKingEarthWalk, 774) talks once a load (0x7FB, sequence 2 of
  seqobject 411bc) and sets nothing else, and nothing in Walled City or the arena sets 0x83B.

Tricky's flame, which task 77 needs, is the ROM's own: he learns it in DarkIce Mines (DarkIce Mines' SnowHorn and
Tricky's flame).

Fixed (Walled City on Restored, `src/game/walled_city.cpp`, whose header has the evidence):

- The Queen stands in the corridor outside the gate, at (-8330, -733, -4720). Sabre coming to her with Tricky near
  meets her (0x80A). She gives nothing: Tricky's flame is the one he learned in DarkIce Mines. Tricky comes from a
  sideload beside her: he walks only inside his zones (type-0x26 curves), and the corridor's are unlinked, so he is
  made in the RedEye's, 184.
- A RedEye (WCTrexChase) waits by the arch, in Tricky's target list (0x33), and offers his flame within 300. Its flame
  point, a type-0x24 curve of kind 4 at (-7900, -735, -4800), is added to the curves' table with the map. Tricky's
  flame reaching it makes it mad: it runs at Sabre, roars as it catches him, and a tooth (WCTrexTooth) falls between
  them. Standing on the tooth, A picks it up and the 4 bits at 0xD5 count it. At ten, 0x821 lets the bowl
  (WCTeethBowl, 308) by the arch be used, and A by it opens the gate (0x82D). The bowl's def has no focus point, so
  the game never targets it, and Restored marks it used for A within 80 as the targeting would. Until then the arch
  holds Sabre back at x -7660.
- BOSS TRex's defeat gives the stone as won (0x82E), and talking to the King with it activates it: the Volcano Force
  Point moves to act 2, whose level control gives the stone carried (0x83B) on the next visit there.
- Played from `--story before-walled-city --bits d5=9`, in one run: the Queen met with Tricky, the flame offered and
  breathed, the RedEye mad, the tenth tooth dropped and picked up (its first pickup's scene waits for A), A by the
  bowl, and Sabre through the gate. With no teeth the gate held. From `--place -7987,-874,-6790,0,0 --bits
  82e,36a=32` the King's talk activated the stone. The fight giving 0x82E was not played to its end. On Original none
  of it is placed and the arch is open. With the flame from the story point and none from the Queen, the same start
  met the Queen at 8.8 s, and the RedEye got mad at Tricky's flame, roared and dropped the tenth tooth.

## The warp table

ROM file 30 has 96 entries of 16 bytes, x, y, z and a layer, read by `func_8004D1D4` (`tools/dino.py warp` prints
each with the maps whose cells hold it and the code that warps there). 54 hold a place and 42 are all zero, and the
game's own code warps to some of them (dll 408 to 26, SHswapstone to 86). Each entry below was booted by
`--story new-game,<index>`, played 15 s with no story state and looked at. "Boots" is alive and drawn, with the player
on a floor and no crash; it says nothing about what the level needs further in.

- 0, 68: each is a Transporter's own spot in a map the world grid gives no cell, 0 map 20's (an older Krazoa Shrine,
  its pad sending to 14) and 68 map 36's (the EarthWalker Temple, an older layout of Walled City's temples, its pad
  sending to 40, ECShrine, whose pad sends back to 68). In world units 0 lies in no map's cells, and 68 in the Swap
  Store's. A running game drops the player through nothing and restores them to the last safe place, and a run
  started there has none: `--story new-game,0` crashes on a null pointer in dll210 at about 9 s, and 68 the same on
  Original. Krazoa Shrines on Restored fills 68 at EC's pad in Walled City (`src/game/krazoa.cpp`).
- 2, 26: NewIceMt1 (map 23), layer 2, the same room. Boots into the story scene a real New Game then `--warp 26`
  plays, in the snow, and dll 408 (IMIceMountain) warps to 26 itself. `--story new-game,26`, `--story new-game,2` and
  `--story new-game` then `--warp 26` each played 60 s alive. No named point starts here.
- 5, 6, 25, 34, 35, 48-53, 56-59, 63, 76, 78: Warlock Mountain (map 11), its deck. Boots, all at one spot.
- 12: CloudRunner - Dungeon (16). Boots, at a barred door.
- 14, 85: SwapStone Circle (14). Boots. 80 too, by the fence.
- 15, 82: SwapStone Hollow (7). Boots.
- 29, 30, 54, 92: BOSS Galdon DIM3 (28), layer -2. Boots; 92 in a dark tunnel.
- 31: Golden Plains (9). Boots, under a hint scroll.
- 36-43: the shrines, each on its Transporter: 36 DFShrine (31), 37 NWShrine (40), 38 DBShrine (39), 39 WGShrine (42),
  40 ECShrine (33), 41 CCShrine (41), 42 GPShrine (34), 43 MMShrine (32). Each boots with no bits, but a test needs the
  story point's.
- 55: DarkIce Mines Level 1 (19). Boots, on the ice.
- 60: CloudRunner Fortress (12). Boots, above the clouds.
- 61: InsideGalleon (30). Boots.
- 64: Discovery Falls (6). Boots, on a pad.
- 66: Diamond Bay (35). Boots, on a pad.
- 73: Moon Mountain Pass (18). Boots, on a pad.
- 81: Volcano Force Point (4), on the SpellPlace. Boots.
- 86: Swap Store (51). Boots.
- 87: Magic Cave - Small (54). Boots.
- 88: Dragon Rock - Bottom (52), layer -1. Boots, dark.
- 90: BOSS TRex (48). Boots, with the camera close on the player.
- 91: Walled City (13). Boots, in a corridor.
- 95: Northern Wastes (10). Boots.

Cape Claw has no entry: `before-sand-worm` and `--place` go there. On Krazoa Shrines' Restored, 77 is filled at Cape
Claw's setuppoint 7 by its west entrance (1457, 156, 16940), which its act 9 loads, where Warlock Mountain's sequence
0x12 sends the player after Willow Grove's release: it sets Cape Claw's act 9 (0xEE), 0x367, whose WM_WarpPoint
(radius 0) warps the player to 77 at once, and 0x368, and clears 0x366 (docs/restored-content.md).

## Choices

Where the ROM says nothing and a restoration had to choose, the choice is in docs/restored-content.md, with every other
fix sorted by what shows it.
