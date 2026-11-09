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
- CloudRunner Fortress's slide doors set up with no player, as a save made with the upper level open (0x458's group
  4) loads: the door starts shut (us.toml, dll393_0018; PR 49).
- An object id with no def, made from its placement: Walled City's object 0x336 before each temple's gap, made again
  as a fall into the pit loads the city, read before the def tables (us.toml, func_80022AA4, `src/game/objects.cpp`;
  PR 55).
- More than 420 late frees in a frame, writing over the loaded models' table, as Dragon Rock's arena unloads after
  the Earth Warrior is freed (us.toml, func_800175D4, `src/game/heap.cpp`; PR 56).

Open:

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
- CloudRunner's race: at the recomp's 30 fps the snow bike couldn't ride back over the edge into the bay where
  CRSnowClaw stands, so the race couldn't be won. Its wall test sees that edge only in a frame of fewer than 3 ticks,
  and a 30 fps frame is 2; a short frame is now tested as one of 3 (us.toml, dll732_3AF8,
  `src/game/cloudrunner_race.cpp`; PR 48). The console's frame rate in the race isn't known: 3 ticks, 20 fps, is the
  fastest that lets the bike over. The race itself sets everything it needs (0x49A comes about 12 s after 0x2E8 with
  no code call, and seqobject 30cca's sequence 0x18 then sets 0x49F), and is reached by the pool on 0x4BE, so it
  needs 0x8D1 and 0x300.
- Diamond Bay: DBlgtbeam's test of Sabre on a beam (dll677_1164) took the cosine and sine of its yaw with their sign,
  so beams 2 and 3 took him from about 60 off and a leap from 3 to 4 stepped the bridge back from under him; both are
  taken without their sign (us.toml, `src/game/diamond_bay.cpp`; PR 54).
- Walled City: the temples' bridges of light (WCTempleBridge, dll790) loaded only the model they draw, so the floor
  test found no floor in the slot it reads and the player fell through; their export 5 returns 0, as the moon lift's
  does, and all three models load (us.toml, `src/game/temples.cpp`; PR 54).
- Dragon Rock's bottom: the lever's scene, which feeding DR_NiceSharpy plays (DR_Seqobj 0x339C4's sequence 2), left
  the player past the gallery's east end with nothing under her, and the void's respawn took back the meat, 0x5FD,
  0x5FE and 0x654, so the Earth Warrior couldn't be freed; she is caught as she falls there and held where she gave
  the meat until the gallery is loaded again (us.toml's hook on func_800145AC, `src/game/dragon_rock.cpp`; PR 56).

### Tricky's Run

`patches/swapstone_hollow.c`, `src/game/swapstone_hollow.cpp`; PRs 16 and 40.

- Tricky's run to the Queen Earthwalker is built (SHtricky, dll501, and SHseqobject 34841 wait on 0xC3, and dll501
  clears it), and task 10's clue asks the player to watch where he runs; nothing sets 0xC3. The trigger is a choice
  (part 3).

### Krazoa Shrines

`src/game/krazoa.cpp`, its header the evidence; PRs 15, 17, 40, 42 and 51.

- Each shrine's win sets Warlock Mountain's act and gives a spirit mask, and WL_seqpoint plays each release: the
  prototype's own way, followed for all eight.
- The shrines' own Transporters send the player back to warp entries 65 (NW), 67 (WG), 68 (EC), 70 (GP) and 71 (CC),
  which are filled (their places are choices, part 3).
- EC's way in is the ROM's own pad: map 36, the EarthWalker Temple, an older layout of Walled City's temples with no
  cell in the world grid, places a Transporter (@00C7, own entry 68, to 40) at (319, -371, -959), and entry 68 holds
  that spot in map 36's units, which in world units fall in the Swap Store's cells (map 51) with no floor. Entry 0 is
  map 20's Transporter, an older Krazoa Shrine with no cell, the same way. Walled City (map 13), which replaced map
  36, has none, so Restored stands that pad there, own entry 68, and fills 68 where it stands (where, part 3).
- Bugs in the shrines' code: MM's, EC's, CC's and WG's release seqpoints all have done bit 0, so the first stopped
  the other three; Walled City's shell game won for a player who stood still (PR 17) and asked for Diamond Bay's mask;
  Golden Plains' "already passed" read Moon Mountain Pass's 0x12A; Willow Grove's torches light themselves and set
  0x1D1, which took the spirit away, and its win set no act 7; Golden Plains' exit went to Diamond Bay's pad.
- Cape Claw's test waited for 0x5B2, which nothing sets; it reads its one SabreBaddie's kill bit 0x1E7, as Discovery
  Falls' (dll433) reads its baddie's.
- After the shrines (PR 51): Warlock Mountain's act 7, WG's, places WL_seqpoint 30055 beside WG's release, playing
  sequence 0x12 once 0x366 is set (mode 1), and acts 6 and 7 count 0x366 as the release done. Sequence 0x12 is the
  only code that sets Cape Claw's act 9 (dll611 0x664), and it sets 0x367, which shows WM_WarpPoint 3008c to entry 77,
  and in its mode 2 a radius of 0 warps at once, at any distance (dll275 0x3E0). Nothing sets 0x366: WG's release does
  (part 3). 30055's done bit was bit 0, shared, and is 0x367. Cape Claw's act 9 loads act 8's placements less 28 of
  its curves, none of its own and no trigger that sets an act: entry 77 is filled at setuppoint 7, by its west
  entrance, which act 9 loads.

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
- The Earth Warrior is freed the ROM's way, with nothing restored but the lever's fall (plain bugs, above): the two
  Crates on the gallery above the arena's east end (40c16, 40c17), broken with the staff, drop meat (meatPickup,
  0x380), and the meat given from Krystal's food bag to DR_NiceSharpy (dll739) beside them sets 0x5FD. DR_Seqobj
  0x339C4 then plays the lever's scene and sets 0x5FE, the shackles open, and DR_EarthWarrior, with his health above
  0, sets 0x654. DR_Seqobj 0x348DC, at the arena's south side, loads only with the player within 112 of it, and on
  0x654 plays sequence 0xC and sets 0x655. The food bag shows on 0x164, which WL_LevelControl (dll607) sets as
  Warlock Mountain loads in act 6, Cape Claw's spirit carried back. Freed, he no longer reads the win (0x628, dll713
  0x0508), so the ROM frees him after it, and the room's door waits for him (part 3).

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

`src/game/acts.cpp`, `src/game/moonseeds.cpp`; PRs 25 and 52. `src/game/diamond_bay.cpp`, `src/game/temples.cpp`,
`src/game/krazoa.cpp`; PR 54.

- The Volcano Force Point's act 2 holds a second SpellPlace that takes Walled City's stone (carried 0x83B, placed
  0x878), its podium, door switch and round door, and VFP_LevelControl set act 1 in its constructor whatever the
  story. Its act 3 sets 0x63C, which moves Diamond Bay to act 3. Restored moves the act with the stone: 2 while it is
  carried, 3 once it is placed.
- Walled City's act 2 holds the Sun and Moon Stones, the statues' sockets and the temples' bits; no code in the ROM
  sets Walled City's act. When it moves is a choice (part 3).
- The moonseeds (0x86A) are counted down by each MMP_mseedrecept and raised by nothing. Where they come from is a
  choice (part 3).
- DB_FlameSwitch (dll689) stands 250 below where it works, disabled, until 0x83E, which nothing sets; its setup's
  +0x20 holds 0x3FF, the bit the Pointback mother's act-2 talk sets, which its code never reads. Restored takes that
  bit for 0x83E, and the switch rises as its load would have raised it (PR 54). How Tricky's flame reaches it is a
  choice (part 3).

### Walled City

`src/game/walled_city.cpp`; PRs 43 and 46.

- Tricky's flame (0x245) is the ROM's own: DarkIce Mines' SnowHorn (uid 1a5e) teaches it once its SnowClaw (uid 1602,
  0x172 at its placement's +0x18) is beaten (PR 46, docs/story-points.md).
- The task list has tasks 75 to 83 here, and the ROM has the Queen (WCQueenEarthWal, dll773, code and no placement),
  the chases (WCTrexChase 777 and WCSabreChase 778, DLLs that only draw) and a bowl (WCTeethBowl), placed nowhere. The
  King (dll774) says he activates the stone and sets nothing. All the rest is choices (part 3).

### Open

Not played yet, so what they need isn't known (docs/story-points.md, The third SpellStone and the moonseeds): the
moon temple's push blocks by night and its symbol's moonlight; the walk into the moonseed's hollow with Kyte (planted
and grown there, the ROM's own code needs no fix: the moonseed is used facing the receptacle, and Kyte's flame is her
scroll's second row at night); a hit on Warlock Mountain's laser targets (WL_LaserTarget, dll595), whose 29c1 lets
EC's guards go (0x2FA), and which a script's aimed shots have not set.

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
  offers: a WarpPoint 33514 to 65 on 0x5C2 and one 32857 to 67 (?). EC's pad and its entry 68 are the ROM's (part 2),
  and where it stands in Walled City and what shows it are ours: map 36 shows its pad always (+0x20 0xFFFF), on its
  temple's axis over the aperture symbol (@22AF), a place Walled City's two temples have no one match for.
- **Golden Plains' and Northern Wastes' spirits** (Krazoa Shrines): masks 0x10 and 0x80, which nothing gives; acts 8
  and 9 at Warlock Mountain give them, raise the column, and place release seqpoints at CC's and WG's sites playing
  CC's and WG's sequences; their trigger bits 0x21E and 0x220 have no ROM setter or getter. Cascade: CC's sequence
  sets 0x221, 0x31C-0x31E, 0x1CF and 0x227 again (?). The ROM offers: nothing, 30055 being WG's act's (part 2).
- **Acts 6 and 7's release check** (Krazoa Shrines): they read CC's and WG's released bits in place of 0x366. Under
  the ROM's route (part 2) 0x366 is set for WG's release only, and sequence 0x12 clears it, so it couldn't keep a
  second visit from giving CC's or WG's spirit again; acts 2-5 each read their own released bit.
- **Who sets 0x366** (Krazoa Shrines): WG's release, as it takes the mask back in act 7. 0x366: no ROM setter; dll607
  reads it in acts 6 and 7, dll611 clears it. 30055 loads in act 7 alone of acts 1-11, beside WG's release. Cascade:
  Cape Claw's act 9 (0xEE), 0x367, 0x368. The ROM offers: CC's release, the other act that reads 0x366, or GP's and
  NW's, ours, in acts 8 and 9, beside none of which 30055 stands.
- **Entry 77** (Krazoa Shrines): Cape Claw's setuppoint 7 (@08F0), (1457, 156, 16940), by its west entrance, which
  act 9 loads. Act 9 adds no placement of its own: it loads act 8's less 28 of its curves, and setuppoint e7b, 68 to
  the west, loads only in acts 12, 13, 15 and 16. Cascade: no trigger act 9 loads sets an act (TriggerSetups 30dfb and
  3001c, to act 1, and TriggerPlane 30ec4, to act 2, are acts 12 and up's), so Cape Claw stays in act 9, which holds
  none of the tablet room's receptacles and set tablets (acts 2 and 3) or its lights (CCkrazoabright, 1 to 3), as acts
  4 to 8 hold none: Cape Claw after the tablet room (Open). The ROM offers: the tablet room, Krazoa Palace or Drakor,
  none tied to act 9.
- **Warlock Mountain's bits each visit** (Krazoa Shrines): act 5 sets 0x142, EC's release trigger (no ROM setter),
  act 7 sets 0x1D1, WG's. The ROM offers nothing else.
- **"Already won"** (Krazoa Shrines): Golden Plains' as act 8 or later, Willow Grove's as act 7 or later.
- **The fallen column** (Krazoa Shrines): Warlock Mountain's WMfallencol (uid 11, @1A44) is left out of acts 5, 7 and
  9, where it lies across the only way north from the pad: EC's release (act 5) and WG's column (@1AB0, acts 7 and 9)
  are beyond it, and the ROM places act 5's whole course there (the lasers, checkpoint 57, the guards, seqpoints 296b,
  2973 and 297b). It reads and sets no bit: dll331 only turns it as it is made. Cascade: none. The ROM offers: act 7's
  TriggerPlane 2e6f at its face (0x36C), on which WL_LevelControl (dll607 0x1788) starts a rockfall (0x36D), as if it
  fell then (?); nothing else that moves or opens it, or a way round (docs/story-points.md).
- **The third tablet's place** (Krazoa Tablets): a CCKrazoaTabletP with 0x237 at (3574, -257, 16514), where the sand
  worm's arena has CCnewseqobject 42BDF, which nothing triggers (its trigger 0x96 is never), placed once the worm is
  dead (0x3FB, dll461). 0x237: no ROM setter; dll458 reads it. Cascade: 0x23B, 0x23C (the settlement room's door),
  0x238-0x23A, 0x3A9. The ROM offers: 42BDF's sequence 8, untriggered (?).
- **The tablet room's end** (Krazoa Tablets): the last lights set 0x52B, a saved bit with no ROM setter, getter or
  placement, which shows a Transporter in the room, own entry 72, sending to 94. Cascade: Krazoa Palace's fire door
  opens on 0x52B, so the palace needs Krazoa Tablets. The ROM offers: Warlock Mountain's WM_WarpPoint 2ac6 sends to 72
  once 0x31E (CC's release) is set, so the ROM sends a player from Warlock Mountain to whatever 72 holds, now the
  tablet room. Entry 77 (above) arrives in Cape Claw too, by its west entrance, so it doesn't stand in for the room's
  Transporter, which the palace route goes on from.
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
  The ROM offers: WM_BossDrakor and WMKrazoa actors hint at a Warlock Mountain scene (?); entry 77 is Cape
  Claw's (above).
- **The fourth SpellStone** (Kamerian Dragon): the dragon dies at 0 health and leaves a CRSpellstone (0x7A8, then 0x5F4
  once placed), and it and the Nurse stay gone. 0x7A8: no ROM setter; read by DFP SpellPlace 40323, seqpoint 4031e,
  target block 401bb and FXEmits. Cascade: 0x5F4 drives SwapStone Circle's meter blocks 32fcd and 32ff7, and
  `after-dragon-rock` carries it. The ROM offers: a CRSpellstone (35335) in Golden Plains' group 3 with no bits (?);
  four KD_Sequences nothing triggers (?); DFP_TargetBlock 401bb's chain, half unreachable, whose end is 0x7A8.
- **The dragon's room's door** (Kamerian Dragon): the arena's north wall, under the room's own entrance plane
  (0x41AF3), warps into the room by entry 89 once the arena is won (0x609), and the stone, 3 s after it is picked up,
  back by entry 84; both empty, no ROM sender. At the wall, not at the win, so the Warrior can still be freed after
  it (part 2). The ROM offers: entry 83, empty too; the entrance plane faces cells with no blocks.
- **The ice spell** (Kamerian Dragon): A reaches 24 in place of 16, and the scroll's aim casts the ice blast as it
  casts the projectile.
- **The arena's magic** (Kamerian Dragon): filled while the fight is on once it is below a fifth, as the dragon's fight
  fills it. The ROM offers: nothing; the two Crates by the arena (40c16, 40c17) hold meat, for the Warrior (part 2).
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
  command in maps 4 or 13. **Clash, a bug of ours, fixed (PR 52):** the Queen Earthwalker sets the volcano to act 11
  before her two cutscene warps there (dll507_04F8, after 0x486 and 0x487), and `acts.cpp` made it act 1 again at the
  map's load, so neither cutscene played. It now leaves act 11 while a visit is to play (0x486 without 0x484, 0x487
  without 0x485), and once both are spent moves it on as at any load: neither visit loads VFP_LevelControl, the ROM's
  only way back to act 1, so on Original warp 81 after them finds act 11, with no podium or SpellPlace.
- **Walled City's act 2** (Third SpellStone): from 0x317, Moon Mountain Pass's spirit (dll611 sets it; dll519, dll607
  read it). Cascade: act 2 loads WCSunStone, the sockets (0x25A, 0x25B, 0x202, 0x243) and the temples (0x812, 0x813),
  and drops act 1's teeth. The ROM offers: nothing sets Walled City's act.
- **The moonseeds** (Third SpellStone): four given as the third stone is placed (0x878). Cascade: the four receptacles,
  then the plants, which want Kyte's flame (0x46E) at night. The ROM offers: task 90, "COLLECTED MOONSEEDS FOR KRYSTAL",
  is Sabre's, with the clue "SEARCH IN DARK, WET PLACES", a talker says "Moonseeds grow in dark damp places.", the
  pickup MMP_moonseed has a def and is placed nowhere, and WM_MoonSeedMoun ("MoonSeed Mound") is Warlock Mountain's.
  So the ROM has them found, not given, and doesn't say where.
- **The meteor's trigger** (Third SpellStone): the three rocks in vents 3, 4 and 6 (0x89B, dll445_0628) set 0x803
  too, the trigger of the meteor's MMP_seqobject 412AF (sequence 5, done bit 0x804). 0x803: no code, placement or
  event of ROM file 13 names it. 0x89B: dll445 sets it, HitAnimator 4267A (map animation 7) reads it. Cascade: the
  way to the test of fear's entrance. The ROM offers nothing else (PR 54).
- **Tricky's flame on the flame switch** (Third SpellStone): a flameblast (object 0x4F0) within 40 on x and z of the
  risen DB_FlameSwitch counts as its hit of type 0x1A. Tricky breathes from curve 418C9 at his own height, about 140
  below the risen switch's hit sphere. Cascade: 0x3D9, the bridge (DBbridgeanim 415CA) and the beams to the egg
  (0x86D, 0x39E). The ROM offers: no code call names 0x3D9 (the switch holds it in its setup), and no other
  flame reaches the switch's height (?) (PR 54).
- **The moon symbol's reach** (Third SpellStone): 210 in place of its setup's 200 (WCApertureSymbo, dll787_00FC); the
  lift's top stands 200.25 from it. Cascade: 0x829, the moon in the ring and 0x244, then the Moon Stone (0x264). The
  ROM offers: no code call names 0x829 (the symbol holds it in its setup), and the lift's top is the only floor of
  type 0x21 by it (?) (PR 54).
- **EC's Transporter by the sockets** (Krazoa Shrines, Third SpellStone): placed on 0x202 and shown on 0x243, the
  sockets' done bits (dll785_0634), the task list's "used Sun and Moon Stones to open shrine"; on Third SpellStone's
  Original, where the sockets never load, 0x264 as before. Cascade: the shrine (map 33, entry 40) and its return to
  68, the pad's own (part 2). The ROM offers: what shows the pad in map 36 is not read (?) (PR 54).
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

- **Cape Claw's act 9 before the tablet room** (Krazoa Shrines, Krazoa Tablets): sequence 0x12 sets Cape Claw's act
  9 at WG's release, and Cape Claw stays in it (Entry 77), so a player who comes to WG's release with the tablet room
  unsolved finds no receptacles there, and 0x52B, the palace's fire door, can't be set (?). The ROM's acts put the
  room first (acts 4 to 9 are Cape Claw after it), and nothing makes a player solve it before WG's shrine. Nothing
  built.
- **CloudRunner Fortress's gold and Bonehead**: 0x8D1 is set from curve 429d9's +0x32 as Kyte lands at the gold perch
  (read by 429d6 and dll400); 0x4C1 is set by CFseqobject 32dda on 0x300. 0x4B7 (dll369 reads and clears it,
  TriggerPlane 30e8c clears it) and 0x4B8 (dll519, dll400 read it) have no setter. 0x4DC is set and cleared by
  TriggerPlane 31a68 at (-9917, 1445, 18272), and dll400 turns it into 0x4DD, whose sequence sets 0x4DE.
- **The babies left**: 29FA's ledge, 29B3, map 30's and map 15's from the master chest.
- **The logs** (tasks after 0x22B and task 84): no ThornTail sells one, SHriver is a placement with no code, and the
  hollow has no dock.

## The warp entries the restorations take

Every one was empty in ROM file 30 but 68, which holds where EC's pad stood in a map the world no longer places.

- 32: Golden Plains' pad sends there, to Golden Plains' shrine (Krazoa Shrines).
- 44: BOSS Drakor's setuppoint, from the red Krazoa (Krazoa Palace).
- 45, 46: Krazoa Palace's shaft, the bottom pad and the west Transporter (Krazoa Palace).
- 65, 67, 68, 70, 71: where the NW, WG, EC, GP and CC shrines send the player back (Krazoa Shrines). 68 is the
  ROM's EC pad's own entry, filled where that pad stands now (part 2).
- 72: the tablet room's Transporter's own (Krazoa Tablets), which Warlock Mountain's WM_WarpPoint 2ac6 also sends to.
- 77: Cape Claw's setuppoint 7 by its west entrance, which act 9 loads, from Warlock Mountain's WM_WarpPoint 3008c
  after sequence 0x12 (Krazoa Shrines).
- 84: back from the Kamerian Dragon's room to the arena; 89: into the room (Kamerian Dragon).
- 93, 94: Krazoa Palace's west and east Transporters (Krazoa Tablets).

The ROM's own entries the restorations use as they are: 90 and 91 (BOSS TRex in and out), 81 (the volcano), 73 (Moon
Mountain Pass, the MM shrine's pad). Still empty: 1, 3, 4, 7-11, 13, 16-24, 27, 28, 33, 47, 62, 69, 74, 75, 79
and 83.
