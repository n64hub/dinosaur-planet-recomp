# Restored content

The December 2000 prototype can't be played to its end as the ROM has it. This recomp changes it in three kinds of
way, kept apart here because each is judged in its own way:

1. Crashes: always fixed, and always on.
2. Restored from what the ROM shows: the game's own code, data, placements or task list say what was meant, and the
   fix does that. As much as can be goes here.
3. Choices: the ROM is silent and a session decided. A choice sets bits that later stretches read, so one can steer
   the rest of the game down a path the developers never meant. Choices come last, the fewest and simplest that make
   the game finishable with all its content, each in the form most likely the developers' own.

A choice the ROM turns out to answer is a clash. A clash is fixed the ROM's way, and the stand-in goes, with no
fallback. A restoration goes behind its General setting, Restored by default, whose Original plays the prototype as
the ROM has it (README, Options). A crash fix, or a fix of a plain bug in what the prototype has, is always on.

Each line gives the fix, its setting or "always on", its source and the PR that landed it. "Open" marks what isn't on
main yet. How a stretch plays, and the bits behind it, are in docs/story-points.md.

The evidence comes from the ROM: `tools/dino.py calls bit <bit>` lists every setter and getter of a bit that is given
as a constant. It misses a bit held in a placement's setup, as DarkIce Mines' SnowClaw holds 0x172 at +0x18, so a bit
it finds no setter for is looked for in the maps' placements too (`tools/dino.py map <map>`) before it is called unset.

## 1. Crashes

Always on, so each setting's Original has them too.

- Moon Mountain Pass's Lunaimars with no Kyte out (us.toml, dll243_0C44; PR 9).
- Cape Claw's sand worm with no sidekick out, three hooks (us.toml, dll461; PR 9).
- The Swap Store's item beams with no item under them as the player walks in (us.toml, dll772_0088; PR 9).
- The audio frame's command list running past its end onto DLL 1, after a while in Cape Claw (us.toml,
  func_800642F4 and the rest, `src/game/audio.cpp`; PR 13).
- An object that fails to be made, read at address 0x30 (us.toml, func_80021A84; PR 15).
- Golden Plains' shrine: its timeout freed the object the player carried (us.toml, dll472_0290, `src/game/krazoa.cpp`;
  PR 15).
- SHtricky with no Tricky out, in SwapStone Hollow, two hooks (us.toml, dll501; PR 16).
- A small basket or crate freed in the player's hands as its group goes (us.toml, func_80022F94,
  `src/game/objects.cpp`; PR 18).
- Dragon Rock's Skeetla fight running out of vertex room and writing over the models (us.toml, func_8004D470; PR 22).
- The scheduler's RSP restart, a hang after a long frame (us.toml, func_8003BE0C; PR 23).
- A swing on the frame the player comes back after a death, before the staff or sword is made (us.toml, dll210_0934,
  `src/game/objects.cpp`; PR 26).
- Golden Plains' portable hole spell, whose pickup made a new player every frame (us.toml, dll3_9E58; PR 34).
- An effect drawn after its object was freed, in the Desert Force Point's place sequence (us.toml, dll13_1080,
  `src/game/objects.cpp`; PR 34).
- More than 40 map blocks at once at Dragon Rock's top's east end (us.toml, func_80041E84; PR 36).
- An item's scene started again every frame, crashing in dll3_8900 (us.toml, dll210_16220; PR 39).
- Walled City's pressure switches writing past their list (us.toml, dll781_05DC, `patches/walled_city.c`; PR 39).

Open:

- CFMainSlideDoor (dll393) when CloudRunner Fortress's group 4 loads with no player.
- Falling into Walled City's moon temple's pit, which crashes making the player's side food bag.
- The skeletons' hits.

## 2. Restored from what the ROM shows

### Plain bugs, always on

Each is a bug in what the prototype has, so it needs no evidence beyond the bug.

- SwapStone Hollow: a white mushroom stuck where it stands vanished without counting (us.toml, dll502_0D74; PR 8).
- Golden Plains' shrine: the fly baddie's shot made a second player (us.toml, dll475_07F8; PR 15).
- Moon Mountain Pass's shrine: the pad the player arrives on sent a player who stepped back on it to an empty warp
  entry; it sends to 73, the pass's own (`src/game/krazoa.cpp`; PR 15).
- SwapStone Hollow: the SwapStone's zone bit 0x887 was turned the wrong way (us.toml, dll325_09B4; PR 16).
- Cape Claw: a beaten SandEel loaded again started its sound every frame (us.toml, dll216_02C0; PR 19).
- An item's scene made a second Sabre and never ended (us.toml, dll3_8900; PR 39).
- Walled City: nothing could hit the sun and moon beacons (us.toml, dll780_011C, `patches/walled_city.c`; PR 39).
- BOSS TRex: the door shut the player out as the intro ended; a floor switch's collision lies 28 off its pad, so
  none charged; the TRex circled its corners after a hit; a charge round a corner left it sideways (us.toml,
  `patches/boss_trex.c`; PR 39).
- Dragon Rock's bottom: a HitAnimator's wall stood again as the player came down the stairs (us.toml, dll353_0160,
  `src/game/hit_animator.cpp`; PR 45).

### Tricky's Run

`patches/swapstone_hollow.c`, `src/game/swapstone_hollow.cpp`; PRs 16 and 40.

- Tricky's run to the Queen Earthwalker is built (SHtricky, dll501, and SHseqobject 34841 wait on 0xC3, and dll501
  clears it), and task 10's clue asks the player to watch where he runs; nothing sets 0xC3. The trigger is a choice
  (part 3).

### Krazoa Shrines

`src/game/krazoa.cpp`, its header the evidence; PRs 15, 17, 40 and 42.

- Each shrine's win sets Warlock Mountain's act and gives a spirit mask, and WL_seqpoint plays each release: the
  prototype's own way, followed for all eight.
- The shrines' own Transporters send the player back to warp entries 65 (NW), 67 (WG), 68 (EC), 70 (GP) and 71 (CC),
  which are filled (their places are choices, part 3).
- Bugs in the shrines' code: MM's, EC's, CC's and WG's release seqpoints all have done bit 0, so the first stopped
  the other three; Walled City's shell game won for a player who stood still (PR 17) and asked for Diamond Bay's mask;
  Golden Plains' "already passed" read Moon Mountain Pass's 0x12A; Willow Grove's torches light themselves and set
  0x1D1, which took the spirit away, and its win set no act 7; Golden Plains' exit went to Diamond Bay's pad.
- Cape Claw's test waited for 0x5B2, which nothing sets; it reads its one SabreBaddie's kill bit 0x1E7, as Discovery
  Falls' (dll433) reads its baddie's.

### Krazoa Tablets

`src/game/krazoa.cpp`; PRs 27 and 31.

- CCkrazoaBits counts three tablet bits, 0x234, 0x236 and 0x237, and nothing sets 0x237: the third tablet is missing.
  Where it lies is a choice (part 3).
- The tablet room's puzzle ends in a state with no handler, and CCkrazoaBits, placed by distance in the room, missed
  a tablet picked up elsewhere: the pickup counts the third itself.
- Entries 71 and 72 land in the tablet room with no plane crossed, so they set 0x2C0, which the planes on the way in
  set (PR 31).
- Krazoa Palace's two KP_Transporters send to each other, entries 93 and 94, both empty: they are filled where the
  Transporters stand.

### Krazoa Palace

`src/game/krazoa.cpp`; PRs 37 and 42.

- Its 19 objects have defs and none is placed: the four consoles (0x58F-0x592), the four text blocks in the same
  order (0x593-0x596, DFP_Textblock's DLL), the section doors, the last door and the red Krazoa. Restored places them;
  where, and in which order, are choices (part 3).
- Every object a restoration places has flag 1 at +4, or the object loop frees it once the camera is far (PR 42).

### Kamerian Dragon

`src/game/dragon_rock.cpp`; PRs 30, 36 and 45.

- The fight is built and can't end: its health runs out in its last stage, nothing reads it, and it wraps.
- The Desert Force Point's other place (uid 40323) takes a stone only with 0x7A8 set, and nothing sets 0x7A8: a fourth
  SpellStone was meant. That the dragon gives it is a choice (part 3).
- The ice spell (0x5CE) on the top: A takes it within 16, its def's reach, and the player stops about 18 from it from
  every side. The scroll aims only the projectile, so the ice blast is never reached.
- The arena's three DRLavaControls go out only to the ice spell's hit, and need about three times the magic Krystal
  holds, with none on Dragon Rock: the refill is a choice (part 3).

### Ending

`src/game/drakor.cpp`; PR 29.

- Drakor's fight is won as the ROM has it (a hit, the Diamond, his fall), and nothing follows his fall: sequence 1
  sends no event and no BossDrakor_sequ holds a warp entry. What follows is a choice (part 3).

### Desert Force Point

`src/game/force_point.cpp`; PR 34.

- Golden Plains' level control sets 0x5F3, the second SpellStone's place, each time the map loads (dll547_0018), so
  the stone stays carried and the swap to Sabre is refused: Restored leaves the bit as it is.
- The place stands on a duct with 18 stubs open high in the corridors' walls, and no surface joins the floor to a
  lip: nothing leads up to it. The lift that does is a choice (part 3).

### Treasure Lift

`src/game/cloudrunner.cpp`; PR 35.

- CFTreasWindLift's direction is 0x8C8, which only the two CFWindSwitch levers at the bottom flip, so a player from
  the top hovers at the corridor's height and can't get down. How she gets down is a choice (part 3).

### Baby CloudRunners

`src/game/cloudrunner.cpp`; PR 38.

- A waiting CFCloudBaby comes out only when Kyte's export 24 says yes, a flag of hers nothing in the ROM sets, so four
  of the five never come out. When Kyte finds one is a choice (part 3).

### Wind Bridge

`src/game/cloudrunner_barrels.cpp`; PR 44.

- The courtyard roof blows at its third barrel hit, and its four barrels stand behind the explosive key's door, west
  of the treasure shaft. A player carrying skips the wind's check (dll210 0xA470, 0xAAB4, 0xAF28) and falls down the
  shaft, so no barrel reaches the courtyard: Restored puts a carrier on the wind as an empty-handed one.

### BOSS TRex

`patches/boss_trex.c`, `src/game/boss_trex.cpp`; PR 39.

- The arena's way back is placed: WarpPoint 413b0 to entry 91, shown by 0x81C, which nothing sets. The defeat loads
  the front end (dll702_2D98). Restored sets 0x81C and the stone (part 3) instead.

### Third SpellStone

`src/game/acts.cpp`, `src/game/moonseeds.cpp`; PR 25.

- The Volcano Force Point's act 2 holds a second SpellPlace that takes Walled City's stone (carried 0x83B, placed
  0x878), its podium, door switch and round door, and VFP_LevelControl set act 1 in its constructor whatever the
  story. Its act 3 sets 0x63C, which moves Diamond Bay to act 3. Restored moves the act with the stone: 2 while it is
  carried, 3 once it is placed.
- Walled City's act 2 holds the Sun and Moon Stones, the statues' sockets and the temples' bits; no code in the ROM
  sets Walled City's act. When it moves is a choice (part 3).
- The moonseeds (0x86A) are counted down by each MMP_mseedrecept and raised by nothing. Where they come from is a
  choice (part 3).

### Walled City

`src/game/walled_city.cpp`; PRs 43 and 46.

- Tricky's flame (0x245) is the ROM's own: DarkIce Mines' SnowHorn (uid 1a5e) teaches it once its SnowClaw (uid 1602,
  0x172 at its placement's +0x18) is beaten (PR 46, docs/story-points.md).
- The task list has tasks 75 to 83 here, and the ROM has the Queen (WCQueenEarthWal, dll773, code and no placement),
  the chases (WCTrexChase 777 and WCSabreChase 778, DLLs that only draw) and a bowl (WCTeethBowl), placed nowhere. The
  King (dll774) says he activates the stone and sets nothing. All the rest is choices (part 3).

### Open

The third SpellStone's links, on a branch: DBlgtbeam's sign bug taken without its sign.

## 3. Choices

Each: what was chosen, every ROM setter and getter of what it touches, which other restoration reads what it sets,
and what else the ROM offers. "Clash" marks a ROM source found for what the choice stands in for: each is fixed the
ROM's way. "(?)" marks a reading not checked in full.

### Landed

- **Tricky's run's trigger** (Tricky's Run): the player within 300 (200 in height) of SHseqobject 34841 with Tricky
  out and the Queen's step 0xB0 at 0 sets 0xC3. 0xC3: dll501 reads and clears, 34841 waits; the TriggerPlanes in maps
  4 and 35 that name 0xC3 play music (action 3), they set nothing. Cascade: none (bank 0, not saved). The ROM offers:
  0xBF, task 10, which the Queen sets, drives two HitAnimators in group 6 beside 34841 (?).
- **The shrines' ways in** (Krazoa Shrines): a Transporter beside Walled City's Sun and Moon stones (EC, shown by
  0x264, which WCInvUseObj sets), Cape Claw's tablet receptacles (CC, 0x318), Willow Grove's sundial (WG, 0x221) and
  where Northern Wastes is entered (NW, 0x21E, ours); Golden Plains' pad (map 9, own entry 31) sent to the empty entry
  32, which goes to GP's shrine (42). The return entries are filled
  with those places: 65 NW, 67 WG, 68 EC, 70 GP, 71 CC. Cascade: CC's lands in the tablet room (0x2C0). The ROM
  offers: a WarpPoint 33514 to 65 on 0x5C2 and one 32857 to 67 (?). **Clash:** entry 68 is not empty: it holds
  (319, -371, -959), in the Swap Store's cells, and EC's own Transporter sends there.
- **Golden Plains' and Northern Wastes' spirits** (Krazoa Shrines): masks 0x10 and 0x80, which nothing gives; acts 8
  and 9 at Warlock Mountain give them, raise the column, and place release seqpoints at CC's and WG's sites playing
  CC's and WG's sequences; their trigger bits 0x21E and 0x220 have no ROM setter or getter. Cascade: CC's sequence
  sets 0x221, 0x31C-0x31E, 0x1CF and 0x227 again (?). The ROM offers: WL_seqpoint 30055 (below).
- **Acts 6 and 7's release check** (Krazoa Shrines): they read CC's and WG's released bits in place of 0x366, which
  nothing sets to 1 (dll611 clears it, dll607 reads it). **Clash (?):** 0x366 is the trigger of WL_seqpoint 30055,
  whose sequence 0x12 sets Cape Claw (map 29) to act 9 (dll611 0x664) and sets 0x367 and 0x368, and 0x367 shows
  WM_WarpPoint 3008c, which sends to the empty entry 77: the ROM's own way on after the shrines, unbuilt.
- **Warlock Mountain's bits each visit** (Krazoa Shrines): act 5 sets 0x142, EC's release trigger (no ROM setter),
  act 7 sets 0x1D1, WG's. The ROM offers nothing else.
- **"Already won"** (Krazoa Shrines): Golden Plains' as act 8 or later, Willow Grove's as act 7 or later.
- **The third tablet's place** (Krazoa Tablets): a CCKrazoaTabletP with 0x237 at (3574, -257, 16514), where the sand
  worm's arena has CCnewseqobject 42BDF, which nothing triggers (its trigger 0x96 is never), placed once the worm is
  dead (0x3FB, dll461). 0x237: no ROM setter; dll458 reads it. Cascade: 0x23B, 0x23C (the settlement room's door),
  0x238-0x23A, 0x3A9. The ROM offers: 42BDF's sequence 8, untriggered (?).
- **The tablet room's end** (Krazoa Tablets): the last lights set 0x52B, a saved bit with no ROM setter, getter or
  placement, which shows a Transporter in the room, own entry 72, sending to 94. Cascade: Krazoa Palace's fire door
  opens on 0x52B, so the palace needs Krazoa Tablets. The ROM offers: Warlock Mountain's WM_WarpPoint 2ac6 sends to 72
  once 0x31E (CC's release) is set, so the ROM sends a player from Warlock Mountain to whatever 72 holds, now the
  tablet room; Cape Claw's act 9 from 0x366 (above).
- **Krazoa Palace's shaft** (Krazoa Palace): a Transporter pair down the shaft from the high hall to the hub, entries
  45 and 46, both empty, with no ROM sender. The ROM offers: setuppoints 1 (4534, 960, 23412) and 7 (5731, 0, 23358).
- **Krazoa Palace's wings** (Krazoa Palace): which wing is which element (fire north-west, earth south-east, water
  south-west, magic north-east), the doors', consoles' and text blocks' places, and their order, fire, earth, water,
  magic, the defs' own order (0x58F-0x592); each block sets one of 0x6A7-0x6AA, saved bits with no ROM setter, getter
  or placement, and each opens the next wing's door and a leaf of the last door. The fire block goes off within 60
  in place of DFP_Textblock's own 0x7BF. The ROM offers: setuppoints 2 to 6 in the wings. Which wing is which has no
  evidence (?).
- **The red Krazoa** (Krazoa Palace): walked up to within 100 once 0x6AA is set, sends the player to entry 44, empty,
  filled at BOSS Drakor's setuppoint. KP_RedKrazoa is dll214, a sequence's actor only, and KP_AnimKrazoa is unplaced.
  The ROM offers: entry 77 (above); WM_BossDrakor and WMKrazoa actors hint at a Warlock Mountain scene (?).
- **The fourth SpellStone** (Kamerian Dragon): the dragon dies at 0 health and leaves a CRSpellstone (0x7A8, then 0x5F4
  once placed), and it and the Nurse stay gone. 0x7A8: no ROM setter; read by DFP SpellPlace 40323, seqpoint 4031e,
  target block 401bb and FXEmits. Cascade: 0x5F4 drives SwapStone Circle's meter blocks 32fcd and 32ff7, and
  `after-dragon-rock` carries it. The ROM offers: a CRSpellstone (35335) in Golden Plains' group 3 with no bits (?);
  four KD_Sequences nothing triggers (?); DFP_TargetBlock 401bb's chain, half unreachable, whose end is 0x7A8.
- **The dragon's room's door** (Kamerian Dragon): the arena once won (0x609) warps into the room by entry 89, and the
  stone, 3 s after it is picked up, back by entry 84; both empty, no ROM sender. The ROM offers: entry 83, empty too;
  the room's own entrance plane (0x41AF3) faces cells with no blocks.
- **The ice spell** (Kamerian Dragon): A reaches 24 in place of 16, and the scroll's aim casts the ice blast as it
  casts the projectile.
- **The arena's magic** (Kamerian Dragon): filled while the fight is on once it is below a fifth, as the dragon's fight
  fills it. The ROM offers: two Crates (40c16, 40c17) by the arena, contents not read (?).
- **The Earth Warrior's food** (Kamerian Dragon): fed 12 once his health is down to -10, once in the fight and once per
  control out. 0xC1: DR_EarthWarrior (dll713) reads and lowers it, Tricky (dll211) lowers it. **Clash:**
  SHbluemushroom (dll502) gives item 0xC1, and Dragon Rock's bottom places four by the arena (map 52, uids 348d8 to
  348db, at about (2260, -1740, -6200)), and more on the top.
- **The ending** (Ending): back to the title by dll67's way after 15 quiet updates, the save as it was. The ROM offers:
  the TRex's defeat loads the front end the same way (dll702); no credits or text.
- **The force point's second lift** (Desert Force Point): a second DFPLift1 under the stub at (-275, 9280), its top at
  -1313 and bottom at -1486, on 0x5E8 (the round door's switch). The ROM offers: its own lift (33ea7, on 0x5E3); the
  act-3 DFP_Textblock chain (0x7BF to 0x7C4, blocks at y -1212 and -1223) and DFP_Platform1, which move only from
  0x7BF, set only by a debug branch of dll638 (?: whether they reach the walkway isn't checked).
- **The place's group** (Desert Force Point): group 7 on while the player is between y -1300 and -1200. The ROM offers:
  the planes at (955, -1435, 8973) and (1896, -1459, 8623), which turn it on.
- **The treasure lift's way down** (Treasure Lift): 0x8C8 set as the wind starts with the player more than 150 above
  its base and not carrying. 0x8C8: CFWindSwitch (dll386) sets, dll370 reads. Cascade: the treasure caves (map 15).
  The ROM offers: the levers only. The race isn't reached this way (the bottom's east is a grate) but by the pool on
  0x4BE.
- **When Kyte finds a baby** (Baby CloudRunners): within 400. Cascade: 0x901, 0x24F-0x253 and the perches 0x2FC-0x300;
  CFLevelControl turns 0x2FC-0x2FF into 0x30A (the treasure door), and 0x300 with 0x8D1 into 0x4BE, which drains the
  pool to the race and moves Bonehead to state 10; CFseqobject 32dda sets 0x4C1 from 0x300 (Bonehead's state 12).
  The ROM offers: nothing sets Kyte's flag.
- **BOSS TRex's stone** (BOSS TRex): the defeat sets 0x83B (0x82E with Walled City on Restored), 0x81C and 0x56E.
  0x83B: VFP_LevelControl's act-2 timer sets it with 0x22B, VFP_SpellPlace 41e21 takes it, `src/game/acts.cpp` reads
  it. Cascade: the volcano's act 2, the King. The ROM offers: the volcano's act 2 itself gives 0x83B (?), which with
  `acts.cpp` moving to act 2 on 0x83B goes round in a circle.
- **The volcano's acts** (Third SpellStone): act 2 on 0x83B, act 3 on 0x878. Cascade: act 3's timer sets 0x63C
  (Diamond Bay's act 3) and 0x83C; act 2 sets 0x22B, the first stone's, again. The ROM offers: no TriggerPlane act
  command in maps 4 or 13. **Clash, a bug of ours:** the Queen Earthwalker sets the volcano to act 11 before her two
  cutscene warps there (dll507_04F8, after 0x486 and 0x487), and `acts.cpp`, at each map load, makes it act 1 again.
- **Walled City's act 2** (Third SpellStone): from 0x317, Moon Mountain Pass's spirit (dll611 sets it; dll519, dll607
  read it). Cascade: act 2 loads WCSunStone, the sockets (0x25A, 0x25B, 0x202, 0x243) and the temples (0x812, 0x813),
  and drops act 1's teeth. The ROM offers: nothing sets Walled City's act.
- **The moonseeds** (Third SpellStone): four given as the third stone is placed (0x878). Cascade: the four receptacles,
  then the plants, which want Kyte's flame (0x46E) at night. The ROM offers: task 90, "COLLECTED MOONSEEDS FOR KRYSTAL",
  is Sabre's, with the clue "SEARCH IN DARK, WET PLACES", a talker says "Moonseeds grow in dark damp places.", the
  pickup MMP_moonseed has a def and is placed nowhere, and WM_MoonSeedMoun ("MoonSeed Mound") is Warlock Mountain's.
  So the ROM has them found, not given, and doesn't say where.
- **The Queen in Walled City** (Walled City): at (-8330, -733, -4720), in the corridor outside the gate; met (0x80A,
  no ROM setter or getter) with Sabre within 150 and Tricky within 400. The ROM offers: task 75's clue "CHECK AGAIN
  WHERE TRICKY'S MOTHER ONCE STOOD", where she stood in SwapStone Hollow (SHqueenearthwal 120e, at -8656, -835,
  -3165), and task 76's "ONLY TRICKY CAN FIND HIS MOM".
- **The RedEye chase** (Walled City): Tricky's sideload at (-8250, -733, -4800) in zone 184; a RedEye (WCTrexChase) at
  (-7820, -735, -4810) with a flame point (a type-0x24 curve) at (-7900, -735, -4800); offered within 300, flame 150,
  catch 70, give-up 600. Task 78's "TRICKY'S FLAME BEGINS THE CHASE!" is the ROM's; the rest is ours.
- **Ten teeth** (Walled City): a tooth per catch, counted in the 4 bits at 0xD5 (no ROM setter or getter); at ten,
  0x821 lets the bowl at (-7720, -748, -4700), used by A within 80, set 0x82D, the gate; the arch holds Sabre back at x
  -7660 till then. 0x821 and 0x82D: no ROM setter or getter. The ROM offers: only the boss door's two teeth (0x81E,
  0x81D).
- **The King's activation** (Walled City): the defeat gives 0x82E and the King turns it into 0x83B. 0x82E: no ROM
  setter or getter. The ROM offers: the volcano's act 2 gives 0x83B (above); the second SpellStone has no won and
  activated pair either.

### Open

- **The Earth Warrior freed**: DR_NiceSharpy (0x5FD) wants item 0x380 or 0x381, which nothing gives, and TriggerPlane
  0x33F78 in the arena would set 0x5FD, but its only action's flags never run. Not restored: the Warrior stays shackled.
- **The race won** (CloudRunner's race, on a branch): no game change. 0x49A is set about 12 s after 0x2E8 with no
  code call, and seqobject 30cca's sequence 0x18 then sets 0x49F; the race is reached by the pool on 0x4BE, so it needs
  0x8D1 and 0x300.
- **The third SpellStone's links** (on a branch): the meteor's 0x89B also sets 0x803 (seqobject 412AF's trigger,
  which nothing sets; HitAnimator 4267A reads 0x89B); DB_FlameSwitch takes its unread +0x20 (0x3FF) for 0x83E and a
  flame blast in its column counts as a hit of 0x1A; the moon symbol reaches 210 (the lift's top is 200.25 from it);
  the EC Transporter shows on 0x202 and 0x243 (0x264 on Original).
- **CloudRunner Fortress's gold and Bonehead**: 0x8D1 is set from curve 429d9's +0x32 as Kyte lands at the gold perch
  (read by 429d6 and dll400); 0x4C1 is set by CFseqobject 32dda on 0x300. 0x4B7 (dll369 reads and clears it,
  TriggerPlane 30e8c clears it) and 0x4B8 (dll519, dll400 read it) have no setter. 0x4DC is set and cleared by
  TriggerPlane 31a68 at (-9917, 1445, 18272), and dll400 turns it into 0x4DD, whose sequence sets 0x4DE.
- **The babies left**: 29FA's ledge, 29B3, map 30's and map 15's from the master chest.
- **The logs** (tasks after 0x22B and task 84): no ThornTail sells one, SHriver is a placement with no code, and the
  hollow has no dock.

## The warp entries the restorations take

Every one was empty in ROM file 30 but 68.

- 32: Golden Plains' pad sends there, to Golden Plains' shrine (Krazoa Shrines).
- 44: BOSS Drakor's setuppoint, from the red Krazoa (Krazoa Palace).
- 45, 46: Krazoa Palace's shaft, the bottom pad and the west Transporter (Krazoa Palace).
- 65, 67, 68, 70, 71: where the NW, WG, EC, GP and CC shrines send the player back (Krazoa Shrines). 68 overwrites
  the ROM's place.
- 72: the tablet room's Transporter's own (Krazoa Tablets), which Warlock Mountain's WM_WarpPoint 2ac6 also sends to.
- 84: back from the Kamerian Dragon's room to the arena; 89: into the room (Kamerian Dragon).
- 93, 94: Krazoa Palace's west and east Transporters (Krazoa Tablets).

The ROM's own entries the restorations use as they are: 90 and 91 (BOSS TRex in and out), 81 (the volcano), 73 (Moon
Mountain Pass, the MM shrine's pad). Still empty: 1, 3, 4, 7-11, 13, 16-24, 27, 28, 33, 47, 62, 69, 74, 75, 77, 79
and 83, 77 the one Warlock Mountain's WM_WarpPoint 3008c sends to.
