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
  number, the position and the health. A run's `--story:` line says what started.
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
- `before-cloudrunner`: Krystal at Moon Mountain Pass (warp 73), with two spirits and the first SpellStone placed.
- `before-desert-force-point`: Krystal in Golden Plains (warp 31), carrying CloudRunner's SpellStone.
- `before-walled-city`: Sabre in Walled City (warp 91), with the second SpellStone placed.
- `before-third-stone`: Sabre in Diamond Bay (warp 66), carrying Walled City's SpellStone.
- `before-dragon-rock`: Krystal on Dragon Rock's top (its setuppoint 2), with all four spirits and three SpellStones
  placed.
- `before-drakor`: the same, at BOSS Drakor's setuppoint (map 44), which no warp entry reaches.

The later SpellStones are told by the SpellPlace that takes each: a placement's +0x1E is its placed bit and +0x20 the
bit of the stone carried, as VFP_SpellPlace (637) takes the first as 0x877 and 0x123. CloudRunner's, won in its race
(CRSpellstone in map 43 holds 0x49F), is carried as 0x2E8 to the Desert Force Point (DFP_SpellPlace in map 21,
placed 0x5F3). Walled City's is carried as 0x83B to the Volcano Force Point's second place (0x878). The fourth is
carried as 0x7A8 to the Desert Force Point's other place (0x5F4): no code sets 0x7A8 by number, and in map 21 a
DFP_seqpoint (dll642) holds it at +0x1E and DFP_TargetBlock (650) at +0x1E beside 0x7A7, the bit DFP_PitSwitch (308)
holds. (Curves' params hold the low halves of other curves' uids, not bits.) A start in a cell no map fills loads nothing (the player falls and is put back, forever):
Dragon Rock's own setuppoint 0 is in one, so `before-dragon-rock` starts at its setuppoint 2.

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
  Grove's at layer 0, so they are reached on foot; no code warps there by number. DR_SupPortal (754) on the top warps
  to the bottom (warp 88). The Kamerian Dragon's room boots and fights: its acid takes the player's health.
- BOSS Drakor (44): the fight is built and plays, but it never ends. No warp entry and no map of the story reaches
  it: its cells touch no other map's, and the only code that starts it is dll70, the prototype's own development warp
  menu (`func_800141A4(44, ...)`). Drakor (697) starts with 24 health (+0x348, set by dll33's start-up, 0x20F4), and
  nothing lowers it: a hit (697 0x0838) only sends him to his second state, which lets the Diamond go (BossDrakorLevel,
  692 0x02F8: 0x7C6, and 0x7C5 as the state ends), and he has no death state. Walking to the Diamond while it is let
  go sets 0x7AE (692 0x0FE0), which plays the map's sequence 1 once and sets 0x7AF. Nothing reads 0x7AF, and none of
  the map's four BossDrakor_sequ (309) holds a warp entry: no map, warp or credits follows. Played two minutes with
  the player attacking, the level went 0x7AC (its intro), 0x7C6, 0x7C5, 0x7C6 again, the player's health 12/12.
- Cape Claw's Krazoa tablets: file 55 gives bits to tasks 133 to 136 (0x238, 0x239, 0x237, 0x3AC), which the task
  list names only as placeholders. The three receptacles (CCkrazoaRecepta, 308) take tablets counted in 0x23B and
  are done as 0x238 to 0x23A; CCkrazoaTablet (459) sets 0x3A9 once all three are placed. CCkrazoaBits (458) counts
  a tablet into 0x23B and sets 0x23C for each of 0x234, 0x236 and 0x237 newly set: 0x234 by a TriggerPlane in
  Discovery Falls, 0x236 by the tablet (CCKrazoaTabletP) in Cape Claw's settlement room, behind the door
  (CCsettlementroo, 307) that 0x23C opens. Nothing sets 0x237: the third tablet is not in the ROM. CCkrazoabright
  (457) runs the room's puzzle in states: the perch switches' lights, the two levers (0x41E, 0x604), then the three
  beacons with the three tablets placed, then the lights again; it starts at its later states from 0x393, 0x4D6
  and 0x3AC, and only 0x4D6 is set by code. (A LevelName's +0x1E, as Drakor's 0x237, is a text, not a bit.)
- Krazoa Palace (3): its rooms are built and it boots and plays at its setuppoints, but of its 19 objects (KP_*, the
  four consoles, the red Krazoa, the doors) none is placed. It holds mushrooms, crates and two KP_Transporters, each
  beside a setuppoint and each sending to the other's entry, 93 and 94, both zero. Its zone planes mark a hub and
  four wings, and the unplaced objects are a wing's each: a console and a text block (KP_textblock, 654, the Desert
  Force Point's DFP_Textblock: +0x19 its sequence, +0x1E its done bit, +0x20 the bit it needs) for fire, earth, water
  and magic, the section doors and the last door (307), and the red Krazoa. Its cells join only Cape Claw's, whose
  edge there is water: nothing in the ROM leads in.
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

## The bits

Each with what shows it, by the DLL that reads or sets it (`dll <n>`) or the task the table in file 55 gives it.

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
Volcano Force Point takes it as its placed bit, with 0x123 as the stone carried. And 0x4E9, which VFP_LevelControl
(619) sets on that visit and Diamond Bay's act counts (The maps' acts). Carrying is 0x123 or 0x2E8, which
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
bit `group` of its map's group field is set. A map's field is a story value of 32 bits, numbered by the u16 at
+0xF0 + 2 × map in dll29's data, and dll29's export 18 reads it. Some maps' fields: 0x480 Discovery Falls (6), 0x4A6
SwapStone Circle (14), 0x42E Moon Mountain Pass (18), 0x3B7 Cape Claw (29), 0x601 Swap Store (51), 0x5DC Dragon
Rock - Bottom (52).

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
0xE6 Walled City (13). A new game puts every map in act 1 (dll29 0x300-0x334), and the object loader
(`func_8004B4A0`) skips a placement whose byte +3 has bit act - 1 set (byte +5 for acts 9 to 16), so a map holds
other objects in each act. A level's control object sets its act, or a sequence does (dll3's event 0x1D, the act of
the map the playing object is in), and ROM file 13 holds one such event, which sets act 3.

`src/game/acts.cpp` moves two maps' acts with the story, at each map load (the header has the evidence), and
`DINO_LOG_ACTS=1` prints each act it moves:

- The Volcano Force Point: act 2 while Walled City's SpellStone is carried (0x83B), 3 once it is placed (0x878). Its
  second SpellPlace, the podium that takes the stone for the second path, the door switch and the round door load
  in act 2 only, and VFP_LevelControl (619) set the map to act 1 in its constructor, whatever the story, so nothing
  of act 2 or 3 ever played. Its act 3 sets 0x63C, which moves Diamond Bay on to act 3 (DBlevelControl, 663: 1 and
  one each for 0x4E9, 0x63C and 0x63D).
- Walled City: act 2 once Moon Mountain Pass's spirit is released (0x317), when the task list sends Sabre back for
  the Sun and Moon Stones. The stones beside the boss door, the sockets on the TRex statues that take them, and
  WCLevelControl's (779) bits that open the Sun and Moon temples are act 2's only; the RedEye teeth are act 1's.

A map's act is kept in the save, so a death or retry restores the act the map had when the level started, and the
map's next load moves it on again. Diamond Bay moves by itself: act 2 from the first SpellStone (0x4E9) holds the
Pointback's second egg (DBPointMum, 658: step 0x3FF, then the egg, 0x39E), and act 3, the nest and the stealer worms,
comes with 0x63C once the volcano has been entered in its act 3.

## The third SpellStone and the moonseeds

Tasks 84 to 106, as played from `before-third-stone`. An item is used as a player uses it: C-right opens the
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
- The moonseeds: each of Moon Mountain Pass's four MMP_mseedrecept (722) takes one used beside it, lowers 0x86A and
  sets its planted bit (0x868 for the one at -7692,1006,15565, in the pass's group 6). What grows it is Kyte's
  flame (0x46E) at night.
- Walled City in act 2 loads the Sun and Moon Stones (WCSunStone, 272), the TRex statues' sockets (785) and its
  level control's act 2 (779).
- The volcano in act 3 (`--story before-third-stone,81 --bits 878,500=1024`, group 10 holding VFP_LevelControl):
  its act-3 timer (130, less the frame's time each frame, 0x69C) runs out 2 to 4 s in and sets 0x63C and 0x83C, and
  Diamond Bay goes to act 3. That was two runs of three: in the other, nothing was set in 48 s, and why is not known.

Not played from here: the second egg given to the Pointback's mother (talking to her plays her act-2 sequence, and
a script's inventory stays on the SpellStone, above the egg), the log of task 84 (no ThornTail sells one, as for
the first), the meteor of task 94 (MMP_moonrock, 445, three of them into MMP_gyservent's vents, 446, which sets
0x89B at three), the night the Moon Stone shows in, and the statues' sockets taking the stones.

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

ROM file 30 has 96 entries of 16 bytes, x, y, z and a layer, read by `func_8004D1D4`. 54 hold a place and 42 are all
zero, and the game's own code warps to some of them (dll 408 to 26, SHswapstone to 86). Each entry below was booted by
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
