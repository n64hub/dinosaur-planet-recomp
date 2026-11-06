# WIP: the early game's tools (remove before this branch lands)

Scripts for reading the game's DLLs and maps and for playing it unattended, kept here only so that the card's next
session has them. `README.txt` says how each is used. Paths in them point at the scratch folder and worktree of the
session that wrote them: change `SC` and `W`. Nothing here is from the ROM: `bits.txt` and `idx.pickle` are made by
`xref.py` and `d.py` from a build's `RecompiledFuncs/`.

`ngreal.txt` is the `--input` script of a real New Game through the title and name screens. `--warp` only works from
real play, after about 115 s of it.

`a/`, `b/` and `c/` are the scripts of the three read-only analyses below. Their outputs stay out of git (disassembly,
a RAM save, shots): make them again. This branch is cut from 95c07f2, and main has moved on since (interpolation).

# Where the card got to

No fix exists yet, no bug's cause is proven, and nothing is landed. Everything is read from the ROM unless it says
played. Each "next" is where that bug's work should go.

## Tools' traps

- `bits.txt` reads $a0 before a delay slot, so some entries are false: "0x15a SET dll515_0018" is really 0x886.
- `--bits <hex>=<value>` takes a decimal value: `452=0x83` parses as 0, use `452=131`.
- `--warp` needs real play: use `ngreal.txt`, then `--warp <n>@115`. `--story` then a warp crashes at the ice rooms.
- Played: a real New Game plus `--warp 26@115` ran 130 s with no crash, reported as map 23. Its shots aren't read.

## The act filter

`func_8004B4A0(setup, map)` filters map objects by the map's act, dll29 export 15 (table offset 0x40): act 0 loads
everything, acts 1-8 skip an object with bit act-1 of its byte +3 set, acts of 9 or more test byte +5. Callers are
`func_8004AF4C`, `func_8004B190` and `func_8004B548`. Act of maps 7 and 8: bit 0xE1. Group word: bit 0x452. dll29
export 16 sets the act. (func_800493B4 and func_800418E8 are not the filter.)

## The river (not confirmed by a play)

- Played: `--story new-game,15` shows the river full (run sh0). Bit 0x64B changes nothing there (run sh1): it toggles
  tunnel and upper-area shapes.
- Read: act writers are new game (act 1), dll515 at 0x0644 (act 7) and func_80048054 by way of func_800141A4 from
  dlls 60, 62 and 70. No sequence or TriggerPlane sets act 7 or 8.
- Unproven: SHriver (uid 30934, the only object with +3 = 0x01) can't load in act 1, so it may be the refilled river
  and never load. But the water seen at the start is block geometry, so this may not explain the symptom at all.
- Unread and possibly the dam: HitAnimators and ExplodeAnimators on 0x658-0x65A, GroundAnimators 34c33-34c35.
- Next: find other callers of export 16's table offset 0x44. Play `--bits 452=131,e1=2` and `e1=0` near world
  -6610,-700,-2594, checking the act is set before the map loads. In run a-r2 (`e1=2`) Thorntail 307f2 still
  appeared, unexplained. Read dlls 353, 351, 357.

## The log (not confirmed by a play)

DFlog (object 495, dll 417) is placed only in map 5 (uid 40b2e) and map 29 (uid 2cd8), not in 7, 8 or 35, and neither
are DFriverflow or DFdockpoint. Tasks 41-43 have no task bit in file 55. Likely it was never placed, but creating it
at run time isn't ruled out. Next: dll 417, dll 508 from 0x1264, dll 309.

## Tricky's crash (not reproduced)

Runs b1 and b2 stood near the Queen with no crash and never reached the sequence. `dll501_00A8` state 0 waits for
0xC3, then plays sequence 3 with the sidekick; `dll3_8900` reads 0x50(a1) with no null check seen, a guess at the
crash, though a sidekick existed in those runs. Nothing visible sets 0xC3 to 1: perhaps a sequence script. Next: walk
in from the entrance through trigger 429ee at (989,-637,1408), as a player does, and decode sequences 1-4.

## The white mushrooms (cause not established)

SHwhitemushroom isn't placed. The ten are SHbluemushroom (dll 502) with param +0x20 = 1, so bit 0x66D in place of
0xC1: map 7 uids 307d5, 308f3, 429e9, 308f2, 308f4, 308f5, 308f6 and map 8 uids 30832, 30844, 307d4. On A, `dll502_0D74`
sends 0x7000A, and 0x66D rises only on 0x7000B in state 8 (`dll502_03A8`): its sender isn't found. The Queen's export
0x20 (`dll1_0DF4`) asks whether 0x66D is the selected item, not how many. No compare with 10 is in dll 507, perhaps
in sequence 2. Candidates, all unproven: 0x7000B never arrives, the item can't be selected, the def's +0x5D makes all
ten count as 0xC1, bits shared with other objects hide some. Next: find the sender in dll 210, read def +0x5D.

## The well and the breaks (nothing proven)

- No game text says "well". SPWell is only in map 51, the Swap Store (uids 340b6, 42881), so what the report meant is
  unknown: ask whoever filed it if this stays unclear.
- The square hole by warp 15 is a real ramp pit under SHswapstone 189e, not missing geometry. Played: c-p2 walked to
  its deep end and entered a climb state, repeating. Whether a player can leave isn't known. Next: place at world
  -6379,-740,-2023 and hold the stick south.
- Played: c-w1 (warp 15, stick 15,70, no bits) walked to (2879,-702,765), inside the 0x658 plug's bounds with 0x658
  clear. Whether that passes the plug or goes round it is not settled: its frames s025-s035 aren't read, and scratch
  is gone, so play it again.
- Unplayed hazards: plane 40b79 toggles 0x887 rather than setting it, and planes 18dc, 308c6, 31c1b, 34640, 34732,
  429da and 429ee toggle too, so skipping one may invert a state. SHswaplift 41f64 sits below every floor there.
- Not found: what sets 0x15A, probably a sequence script.
