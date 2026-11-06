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
- `--place <x>,<y>,<z>[,<layer>]` starts there instead, for a place the warp table has no entry for.
- `--bits <hex>[=<value>],...` sets more of the game's bits at the start, after the point's own.
- `--report` prints a line a second, from the start or from the game's first pass without `--story`: the clock, the map
  number, the position and the health. A run's `--story:` line says what started.

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

The game's own New Game is dll64's name screen calling export 1, then dll63 calling `func_8005D410(1, ...)`,
`func_80041D20(1)`, `func_80041C6C(1)` and export 7, and `story_start` makes the same calls:

- `func_8005D410` drops from the 640x480 mode the game boots in, for its logos, to play's 320x240. In the boot's mode
  the Z buffer (`0x80093010`) is the first framebuffer, so a level played in it shows depth as colour in a third of its
  frames: bands of magenta and green, white where the depth is clear.
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
- `after-first-spellstone`: Sabre at the Volcano Force Point (warp 81), the first spirit won and the first SpellStone
  placed.
- `before-sand-worm`: Krystal in Cape Claw, on the sand at the map's setuppoint beside the worm. The warp table has no
  entry in Cape Claw.
- `before-df-shrine`, Krystal (warp 36); `before-db-shrine`, Sabre (38); `before-mm-shrine`, Krystal (43);
  `before-ec-shrine`, Sabre (40): the four shrines the task list orders, each with the spirits before it.
- `before-gp-shrine` (42), `before-nw-shrine` (37), `before-cc-shrine` (41), `before-wg-shrine` (39): Krystal with all
  four of those spirits. The task list stops before these (from task 107 it holds placeholders), so their order and
  who plays them are not known from the ROM.

A shrine's warp lands at its Transporter, 1600 units from the shrine object. Its test starts when the player walks up
to that object, which each shrine's DLL checks in its first state.

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
Volcano Force Point takes it as its placed bit, with 0x123 as the stone carried. Carrying is 0x123 or 0x2E8, which
SHswapstone refuses a swap on ("You cannot swap when holding a SpellStone"), so a point leaves both clear.

## Object groups

A map's objects come in groups, and most of a level's enemies and props load only while their group is on. Each
object's setup in ROM file 31 has a flags byte at +4 and its group at +6: with flag 0x10 the object loads only while
bit `group` of its map's group field is set. A map's field is a story value of 32 bits, numbered by the u16 at
+0xF0 + 2 × map in dll29's data, and dll29's export 18 reads it. Some maps' fields: 0x480 Discovery Falls (6), 0x4A6
SwapStone Circle (14), 0x42E Moon Mountain Pass (18), 0x3B7 Cape Claw (29), 0x5DC Dragon Rock - Bottom (52).

A story point turns no group on, so a level started from one has only its ungrouped objects: Moon Mountain Pass's
Lunaimars are in groups 3 to 7, and Cape Claw's worm in group 10, its SandEels in 11 and 12. `--bits <field>=<value>`
sets the field, its value in decimal: `--bits 42e=-1` turns on every group of Moon Mountain Pass, more objects at once
than play ever loads, and `--bits 42e=385` groups 0, 7 and 8, where the pass's Transporter lands.

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

## The warp table

ROM file 30 has 96 entries of 16 bytes, x, y, z and a layer, read by `func_8004D1D4`. 54 hold a place and 42 are all
zero, and the game's own code warps to some of them (dll 408 to 26, SHswapstone to 86). Each entry below was booted by
`--story new-game,<index>`, played 15 s with no story state and looked at. "Boots" is alive and drawn, with the player
on a floor and no crash; it says nothing about what the level needs further in.

- 0, 68: no map holds these places. A running game drops the player through nothing and restores them to the last
  safe place, and a run started there has none: `--story new-game,0` crashes on a null pointer in dll210 at about 9 s,
  and 68 the same. Not usable.
- 2, 26: NewIceMt1, layer 2, the same room. A real New Game and `--warp 26` plays a story scene there, Krystal at a
  campfire, and dll 408 (IMIceMountain) warps to 26 itself. Started there directly it crashes before the first
  report: 26 in `bcopy` under `func_80018314`, 2 reading a null pointer in `func_80021A84` under dll416, with or
  without the bits up to task 6 (0xD0, 0xB4, 0x2B2, 0x70). `--story new-game` then `--warp 26` crashes as well, at
  12 s or 55 s, so it is the route: a story point can't reach this room yet, and a real New Game can.
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
