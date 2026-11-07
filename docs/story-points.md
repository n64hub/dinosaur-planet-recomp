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
  told, at the point's own place or at that entry of the warp table.
- `--place <x>,<y>,<z>[,<layer>[,<facing>]]` starts there instead, for a place the warp table has no entry for, facing
  a 16-bit angle (0, the default, faces -z).
- `--bits <hex>[=<value>][@<seconds>],...` sets more of the game's bits at the start, after the point's own, or one
  given a time when the `--input` clock reaches it: what a test's own object sets, for a test a script can't play.
- `--report` prints a line a second, from the start or from the game's first pass without `--story`: the clock, the map
  number, the position, the health and the magic. A run's `--story:` line says what started.
- `--watch-bits <hex>,...` prints a `bits:` line each time the game sets one of those bits, with the clock, the value
  and the one it had, and `--watch-bits all` one each time any bit changes: what a test or a cutscene sets, as it
  happens. With or without `--story`.

`src/game/story.cpp` holds the points, and `src/main/main.cpp` reads the options.

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
- `before-sand-worm`: Krystal in Cape Claw, on the sand at the map's setuppoint beside the worm. The warp table has no
  entry in Cape Claw.
- `before-df-shrine`, Krystal (warp 36); `before-db-shrine`, Sabre (38); `before-mm-shrine`, Krystal (43);
  `before-ec-shrine`, Sabre (40): the four shrines the task list orders, each with the spirits before it.
- `before-gp-shrine` (42), `before-nw-shrine` (37), `before-cc-shrine` (41), `before-wg-shrine` (39): Krystal with all
  four of those spirits. The task list stops before these (from task 107 it holds placeholders), so their order and
  who plays them are not known from the ROM.
- `before-cloudrunner`: Krystal at Moon Mountain Pass (warp 73), with two spirits and the first SpellStone placed,
  and SwapStone Circle at its act 9, where its chief gives the pass's key (CloudRunner Fortress).
- `before-desert-force-point`: Krystal in Golden Plains, at its setuppoint 0 where the way from SwapStone Circle comes
  in, carrying CloudRunner's SpellStone. Its warp, 31, stands under the sand: a start there falls and is put back.
- `before-walled-city`: Sabre in Walled City (warp 91), with the second SpellStone placed.
- `before-third-stone`: Sabre in Diamond Bay (warp 66), carrying Walled City's SpellStone.
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
  ProjectileSpell (dll272, at -3553, -1502, -4662) gives the projectile spell (0x5CE). Its DR_SupPortal (754, in
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
  -1831). Neither part is played through yet: from setuppoint 8 (-817, -1613, -5830) a walk east stops at x -721,
  and from setuppoint 1 on layer 0 one stops at (154, -1470, -6221) a step past the layer change.
- The arena (groups 0 to 2, `--bits 5dc=7`): DR_Creators (738) send Skeetlas (615), at most 20, at DR_EarthWarrior
  (713), shackled there. It is won when 0x628 is set, by the Warrior's update once 0x789 is 3 and no Skeetla is left,
  which plays DR_Seqobj 0x339C6's sequence 7, and its end sets 0x609 (on the top it opens the CloudRunner's cage,
  DR_Cage, DR_CageControl and DR_CloudRunner). 0x789 counts the three DRLavaControls (737) put out: each loses a
  point of health (3 or 15, its +0x1A) to a hit of kind 0x19 (export 2) and at 0 sets its bit (0x5D8, 0x5D9, 0x5DA)
  and raises 0x789. Neither the staff (B) nor the projectile spell put one out in a run, so what hits as 0x19 is not
  known yet. The Warrior is freed by 0x5FD (DR_NiceSharpy, 739, given item 0x380 or 0x381, which item not known yet:
  sequence 2 sets 0x5FE, which frees the DR_Shackles, 0x654, sequence 0xC and 0x655); 0x5DE is his death. The
  TriggerPlane 0x33F78 in the middle of the arena would set 0x5FD, but its only action's flags (0x04, repeat, without
  0x01, forward) never run.
- The arena and the dragon's room share the world cell (2, -8) and their ground: the room is the arena on layer 0,
  its KamColumns' pits where the arena's DRLavaControls stand. Nothing in the ROM leads into the room: no warp entry
  lies in it, and its entrance, a TriggerPlane at its north (0x41AF3, which sets 0x4D7 for its KD_Sequences), faces
  cells with no blocks, where the player falls and is put back. With Kamerian Dragon on Restored, the arena once won
  (0x609) leads into the room by the empty warp entry 89, and the fourth SpellStone, 3 s after it is picked up, back
  to the arena by the empty entry 84 (`src/game/dragon_rock.cpp`). Played from `before-dragon-rock` with `--place
  1659,-1822,-6121,-1 --bits 5dc=7,609`: the warp into the room came at once, and with `7a8@20` added, the warp back
  to the arena at 23 s, where she stayed.
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
  setting, Restored, goes back to the title there (`src/game/drakor.cpp`). Played from `before-drakor` with
  `--input`: A every half second from 12 s to 25.6 s hits him (0x7C6 at 25 s), the stick held up from 27 s walks the
  player to the Diamond (0x7AE at 47-49 s), and the title follows at 82 s.
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
  `src/game/krazoa.cpp`'s header has the layout and the evidence.
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
create, `func_800213EC`, passes what it made to `func_80021A84` with no test, so a create that fails for want of
memory crashes there, under the MagicPlant (dll290) or SHBirchTree (dll520) that asked for it, as all of them on did
with what the title holds still held (src/game/story.cpp lets go of it).

A place in Cape Claw's walled part needs a bit too. A VisAnimator (354) hides the shapes of its groups in its block
while its bit is clear, and TriggerPlanes toggle those bits as the player crosses them (action 0x12, with 0x8000 on
the bit). In map 29, 0x2C0 shows the tablet room and the rooms around it, 0x2C1 the rooms north of them and 0x2B4 the
sea and the east tunnel. The planes that toggle 0x2C0 are in the hall west of the room (uid 41FC0) and the tunnel east
of it (352FB), so a player who walks in sees the room (played, both). `--place` crosses neither, and draws night sky
over black with the objects floating, so a test there needs the bit: `--story before-cc-shrine --place 4400,-7,17600
--bits 2c0`. Warp entries 71 and 72 land in the room and set it themselves (`src/game/krazoa.cpp`).

## CloudRunner Fortress

Tasks 56 to 68, played from `before-cloudrunner` on the desktop build a link at a time, each started with `--place`
and the bits of the links before it. None of it needed a fix to the game; the story point did (SwapStone Circle's act).
World coordinates: the fortress (map 12) lies at cell (-15, 29), its treasure caves (15) at (-20, 28) on layer -1 and
the race (43) at (-20, 29) on layer -2.

- The key. SwapStone Circle's chief (SC_chieflightfoot, 518) gives it only at the circle's act 9, the 4-bit field
  0xE7, which SC_levelcontrol (519) moves on as the player enters map 14. Talked to, he sets 0x60E and 0x60F, and his
  sequence sets 0x611, the bit Moon Mountain Pass's padlock needs. Played.
- The courtyard. Its roof (CFBlastedRoof, 389, about 110 above the courtyard's floor at its middle, group 8 of 0x458)
  is the deck Kyte is shut in on, and blows at the third barrel blown under it: it counts hits of type 5, a CFbarrel's
  explosion (388 0x12A4-0x1314, reach 100, 10 up), one a barrel, each moving 0x2DE+n to 0x2DF+n and 0x900 to n+1, and
  the hit that finds n at its +0x1A (2) sets 0x2E1 (389 0x01CC-0x0398). A barrel on the courtyard's floor under the
  roof's middle reaches it; none is placed there, and the four behind the explosive key's door are one for the wall
  and three for the roof. With 0x2E1 the floor below (CFExplodeFloor, 390) blows, its pieces set 0x43, the force field
  fades (0x34F, 0x359), and Kyte joins: her mother hid her five brothers and sisters. Played: three barrels blown
  there; one carried over the wind bridge (the shaft, below), set down under the middle and swung at (0x2DF set,
  0x900 at 1). No barrel reaches the deck itself: the courtyard's ladders (a wall of collision type 0x0A,
  dll210_BA38) can't be started while carrying, and a carried barrel can't be thrown.
- The explosive key. The courtyard's guard, beaten, sets 0x4DF, and CFExplosiveKey (272) appears, 0x8FC once picked
  up. Its door, CCExplosiveDoor (308) in the corridor's east wall at (-13003, 2049, 18679), takes it from the item
  menu: 0x8FC is taken and the door's sequence sets 0x8FF. Played, the key's pickup by bit.
- The wall. Behind that door are four CFbarrels (388). A within 20 units of one lifts it (A farther off swings the
  staff), and it is carried through the door and round the corridor to the cracked wall, CFBlastedWall (389) in the
  same wall at z 18879. A beside the wall sets nothing down; a step back it does. A barrel explodes at its third hit
  (one set down counts them too), or at once to an explosion or fire: set down at (-13012, 18891) and swung at from
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
  carried across and up into the courtyard. The bottom leads north to the caves and east to the
  drop to the race, a TriggerPlane at (-12285, 1753, 18871) that takes the layer from 0 to -1, then map 15's at y 1630
  (0x0774) to -2, map 43 (read, not played).
- The race. Its bikes are in map 43's group 1 (`--bits 4a3=7`). Beating the ClubSharpClaw sets 0x4BA. A beside
  CRSnowBike mounts it (0x49D, 0x497) and the race runs; a lost race (0x499) puts the player back beside the bike. Five
  rams on CRSnowClaw drop the SpellStone (CRSpellstone, 730), 0x2E8 once picked up, and 0x49A then runs the
  sequence that sets 0x49F. Played to a lost race; the win is read from the code: a ram is CRSnowBike's hit of type
  0x14, made only while the player rides it faster than 2 (732 0x1188-0x1214); CRSnowClaw (728 0x0418) counts four
  then drops the stone at the fifth, if CRSpellstone is within 500 units of it; the race is lost when the fuel runs out
  (732 0x0DB4, CRFuelTank adds 3000), not to time; stick Y is the throttle and B the one button read. Nothing in code
  or placements sets 0x49A.
- Bonehead, CFGuardian (369) in map 43, keeps its state in the field 0x4B and starts from it: 6 with the power on
  (0x57), 9 with 0x43, 10 with 0x4BE (which needs 0x8D1 and 0x300), 12 with 0x4C1. At 12, 0x4B7 plays its sequence
  11 and with 0x49A moves it to 13; at 13 the SpellStone used on it from the item menu sets 0x4AB, its sequence sets
  0x4AA, and it goes to 14. Played from 13.

An object that wants an item (a lock, dll308, its item at +0x1E; CFGuardian) takes it only as used from the item menu,
never from the bit alone. C-right opens Krystal's items (dll1's menu 1), A uses the one shown and closes the menu, and
the object takes it in the frame the player presses A with it targeted: dll2 sets the target's +0xAF bit 0 then, with
the player inside its radius and within 100 units of its height. dll1's export 7 answers whether the item used is the
one asked for. In an `--input` script, standing at the object: A, C-right, A.

Not played: the roof's two barrels, the dungeon (map 16), the power, the five baby CloudRunners (the fifth is in map
15, from the master chest), the gold, the way from the caves to the race and the race won. Read from the ROM, nothing
sets 0x8D1 (so 0x4BE, Bonehead's state 10, never comes), 0x4B8 (CFGuardian's sequence at 0x1D5C) or 0x49A, and Krystal
makes no hit of type 0xF, which the ProjectileSwitches by the main door take, but by the two ZBombs there.

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
- 0x2D, 0x1D7: spells. dll 210's function behind its export 51 switches on these, and CClightfoot sets 0x1D7 with task
  16, "COLLECTED FORCEFIELD SPELL". WL_LevelControl (607) sets both too.

The spirits, set in this order by WL_seqpoint (611) and read by WL_LevelControl (607):

- First: 0x316, 0x21F. Second: 0x277, 0x21B. Third: 0x317, 0x21C. Fourth: 0x318, 0x21D.

`first_stone`: 0x104 (task 29) and 0x22E (task 39) on the way there, and 0x877, placed: VFP_SpellPlace (637) in the
Volcano Force Point takes it as its placed bit, with 0x123 as the stone carried. Carrying is 0x123 or 0x2E8, which
SHswapstone refuses a swap on ("You cannot swap when holding a SpellStone"), so a point leaves both clear.

SwapStone Hollow's own, as played: 0xC2 counts the white mushrooms given to the Queen Earthwalker and 0xB0 is her
quest's step. Given them at step 4, five or ten, she plays her sequence, sets 0x486 and step 5, and the game warps to
the Volcano Force Point (map 4) for its cutscene; 0x487 and step 6 come with a second visit there, and step 7 back in
the hollow. 0xC3 starts Tricky's run to her (SHtricky, 501, and an SHseqobject, 309, wait on it), and nothing in the
game sets it: `patches/swapstone_hollow.c` does. 0x887 is the SwapStone's zone (SHswapstone answers only while it is
set), kept by TriggerPlane 40b79 (`us.toml` says how).

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

## The warp table

ROM file 30 has 96 entries of 16 bytes, x, y, z and a layer, read by `func_8004D1D4` (`tools/dino.py warp` prints
each with the maps whose cells hold it and the code that warps there). 54 hold a place and 42 are all zero, and the
game's own code warps to some of them (dll 408 to 26, SHswapstone to 86). Each entry below was booted by
`--story new-game,<index>`, played 15 s with no story state and looked at. "Boots" is alive and drawn, with the player
on a floor and no crash; it says nothing about what the level needs further in.

- 0, 68: no map holds these places. A running game drops the player through nothing and restores them to the last
  safe place, and a run started there has none: `--story new-game,0` crashes on a null pointer in dll210 at about 9 s,
  and 68 the same. Not usable.
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

Cape Claw has no entry: `before-sand-worm` and `--place` go there.
