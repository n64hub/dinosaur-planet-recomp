# DLL 325 triggers (TriggerPoint / Plane / Area / Time / Setup / Bits / ...): actions decoded

Base of DLL 325 in the recomp: 0x9018435C (so L_90184E58 = offset 0xAFC). Offsets below are DLL offsets
(as `./py d.py 325` prints them). Dumps used: agent2/d0018.txt (ctor), d0288.txt (update), drest.txt (helpers),
q325.txt (the interpreter dll325_09B4). Jump tables read from the recompiled C (`switch (jr_addend_...)`).

## Object types handled (param block +0x00 = raw map object id)

ctor 0x0018 / update 0x0288 switch on (id - 0x4B), plus 0xF4 and 0x230:

| raw id | ctor | update | what it is |
|---|---|---|---|
| 0x4B | 19C0 | 1AFC | TriggerPoint: sphere, radius = (+0x3A * 2), squared into state+4 |
| 0x4C | 1EE0 | 23A8 (only while gate bit +0x48 is set, if +0x48 >= 0) | TriggerPlane: bounded plane, yaw = +0x3D<<10, pitch = +0x3E<<8, size from +0x3A.. |
| 0x4D | 260C | 2630 | area (273C inside test): +1 enter, +2 stay in, -1 leave, -2 stay out |
| 0x4E | - | 0x668 | TriggerTime: counts frames into state+0xC, fires (s3=+1) once >= u16 +0x46 |
| 0x50 | - | 0x6D4 | fires (s3=+1) every frame, then destroys itself (80022338) |
| 0x54 | copies +0x48..+0x4E to state+0x5A.. | 0x720 | TriggerBits: fires (s3=+1) when all four bits (those >= 0) are set |
| 0x230 | 1CE8 | 1D18 | (curve type) |
| 0xF4 | 2B10 | 2B20 | (button type) |

Common params: +0x38 s16 = chain id (matched by op 0x0C), +0x43 u8 = who sets it off (0 player,
1 sidekick = first object of list 1 via 8002394C, 2 the camera (DLL 2 export 2), >=3 nearest object of that
category within 200), **+0x44 s16 = "done" bit**, +0x46 u16 = TriggerTime limit, +0x48 s16 = TriggerPlane gate bit
(TriggerBits: +0x48..+0x4E four bits). Actions: 8 records of 4 bytes at +0x18..+0x37.

Direction value s3 (a2 of 09B4) as the shape updates compute it:
- TriggerPlane (23A8): plane value d = n.p + D of previous and current actor position. d_prev >= 0 and d_new < 0 →
  **+1** (crossed from the front / normal side to the back); d_prev < 0 and d_new >= 0 → **-1** (back to front).
  09B4 is called only for ±1, and only when the crossing point is inside the plane's bounding box (state+0x38..0x4C).
- TriggerPoint (1AFC): +1 entered the sphere, +2 still inside, -1 left, -2 still outside (called every frame).
- Area (2630): same +1/+2/-1/-2.
- Time/Setup/Bits and the "done at load" replay: always +1.
- a3 = truncated current distance (plane distance or squared sphere distance), 0 for the others.

## The condition byte (byte 0) — 09B4 offsets 0x0A24-0x0AD8

```
state flags (state = obj->0xB8, byte 0): 1 = has fired with s3>0 before, 2 = has fired with s3<0 before,
                                         4 = done bit was already set at load, 0x40 = first frame
if (flags & 4) and !(cond & 0x20): skip
if cond & 0x10:                               # "level" mode, no once-only
    if cond & 1: fire when s3 >= 0
    elif cond & 2: fire when s3 <= 0
    else: fire always
elif s3 == +1: fire if (cond & 1) and (!(flags & 1) or (cond & 4))
elif s3 == -1: fire if (cond & 2) and (!(flags & 2) or (cond & 8))
else: skip                                    # +2/-2/0 only reach 0x10 records
after the loop: s3 > 0 → flags |= 1 and BIT_SET(+0x44 done bit, 1);  s3 < 0 → flags |= 2
```

| bit | meaning |
|---|---|
| 0x01 | fire on +1 (plane: crossing front→back; point/area: entering) |
| 0x02 | fire on -1 (plane: back→front; point/area: leaving) |
| 0x04 | +1 records fire again on later crossings (without it: first +1 crossing only) |
| 0x08 | -1 records fire again on later crossings (without it: first -1 crossing only) |
| 0x10 | level mode: fire on every call with s3 >= 0 (with 1) / s3 <= 0 (with 2) / always; ignores once-only. For a point/area that is every frame inside / outside |
| 0x20 | when the done bit (+0x44) was already set at load, these records (and only these) replay once on the first frame (restores state such as groups); then flags = fired-forward |

So 0x0F = both directions every time, 0x05 = forward every time, 0x0A = backward every time, 0x01 / 0x02 / 0x03 =
first crossing only (per direction). **0x04, 0x08 and 0x00 alone never fire** (no direction bit, no 0x10): they are
dead records (designer-disabled). Once-only state lives in RAM and is lost when the object reloads, unless the done
bit +0x44 is a real bit (then "fired forward" is remembered and re-armed as flag 1 at load).

Nobody else reads byte 0: the ctor only scans op 0x19 records (to preload DLLs).

## Every op (byte 1), argument = bytes 2-3 (hi, lo); op-1 indexes the jump table at 0xAFC

| op | offset | what it does |
|---|---|---|
| 0x00 | - | empty record, skipped |
| 0x01 | 0x0B04 | sub-switch on hi: 7 → send message 0x80 (s3=+1) / 0x81 (otherwise) to every object of def 0x365 and of def 0x4CA within 6000 units (80031864); 8/9/10 → player export 67 (vtable 0x110) with 9/10/11; 0-6 nothing |
| 0x02 | - | no-op |
| 0x03 | 0x0C64 | music (DLL 5 AMSEQ): s3 >= 0 → export 0 = start music action arg16 (record of MUSICACTIONS.bin, file 0x0A); s3 < 0 → if export 2 says it is active, export 1 = stop it |
| 0x04 | 0x0D30 | sound (DLL 6 AMSFX): s3 >= 0 → export 14 play sound arg16 at the trigger, handle kept per record (state+0x64+4i); s3 < 0 → stop that handle (export 6) |
| 0x05 | 0x0FE4 | no effect (tests state+4, a3, id==0x4C, every path skips) |
| 0x06 | 0x0DC0 | camera (DLL 2 export 8)(hi, lo): hi = kind 0..4; kind 0 switches to camera mode 0x5A with preset lo&0x7F (flag lo&0x80), 0x78-frame blend; kind 1 → mode 0x59, ... |
| 0x07 | - | no-op |
| 0x08 | 0x0DE8 | world switches, sub on hi, value lo (0/1; lo >= 2 is written back as 1): 0 → 80041C6C (flag 0x10 of 0x80092A98), 1 → 80041CA8 (0x20), 2 → 80041CE4 (0x40), 3 → DLL 12 export 6, 4 → DLL 16 export 2, 5 → 8005CA5C (byte 0x800BCC78), 6 → 8001EBD0 (byte 0x80090CB0, read by 8001EBE0, which dll29's checkpoint code reads), 7 → 80041E24 (flag 0x20000) |
| 0x09 | - | no-op |
| 0x0A | 0x1024 | environment effect: 80000860(obj, actor, arg16, a3) loads record arg16 (0x60 bytes) of ENVFXACT.bin (file 0x49) and hands it by its type byte to DLL 9 (types 0-2, 4), DLL 8 (3), DLL 7 (5), ... (sky / fog / clouds / time of day) |
| 0x0B | 0x10A0 | **object sequences** (DLL 3): hi 0 or 3 → export 1(lo, 0): queue "start sequence number lo" (lo 0..44): every sequence object (obj+0x44 == 0x10) whose param byte +0x1F == lo starts its sequence (DLL 3 own+3D0(obj,1)); hi 1 → export 2(lo, 1), hi 2 → export 2(lo, 0): set/clear DLL 3's per-number flag table[lo] (own+A72C+0x138). **No trigger in any map uses op 0x0B.** |
| 0x0C | 0x1140 | chain: own 29C0 runs the action list (same actor, s3, a3) of every trigger object of types 0x4B-0x50, 0x54, 0x230 whose +0x38 == arg16 |
| 0x0D | 0x1058 | lighting: 80000450(obj, actor, arg16, s3, a3) loads record arg16 (0x28 bytes) of LACTIONS.bin (file 0x0C); type 2 → DLL 57, type 5 → DLL 58, else DLL 11 export 0 (light fx) |
| 0x0E | - | no-op |
| 0x0F | 0x11AC | own 1754: empty function, no-op |
| 0x10 | 0x1180 | 80023A18(player, hi): set the player's model/variant index (obj+0xAD), clamped |
| 0x11 | 0x147C | BIT_SET(0x4E2, arg16) (16-bit value) |
| 0x12 | 0x11CC | **generic bit op**, own 1764: bit = arg & 0x3FFF, mode = arg >> 14: 0 clear (BIT_SET(bit,0)), 1 set (value -1 = all ones), 2 toggle (~value), 3 rewrite unchanged |
| 0x13 | 0x1228 | object group arg16 of this map (obj+0xAC) on: DLL 29 export 19(map, group, 1) |
| 0x14 | 0x125C | same, off |
| 0x15 | 0x12E8 | own 1868: list 800213A0(arg16+2) of object ids (to -1): each one not loaded (8003E960) is created via 80012584(0x32, 3, 0, id) |
| 0x16 | 0x1314 | own 1920: same list, each loaded one is removed (8003DA40) |
| 0x17 | - | no-op |
| 0x18 | 0x1340 | set this map's act: DLL 29 export 16(map, arg16) = BIT_SET(act bit of the map, arg) |
| 0x19 | 0x1370 | call export lo of DLL (0x51 + hi), preloaded by the ctor (hi 0..2 → DLL 81..83), with (obj, actor, s3, a3) |
| 0x1A | 0x13C0 | group on in another map: DLL 29 export 19(map = lo, group = hi, 1) |
| 0x1B | 0x13E8 | same, off |
| 0x1C | 0x1410 | BIT_SET(0x46E, arg16) (16-bit value) |
| 0x1D | 0x1434 | BIT_SET(0x488, arg16) (8-bit value) |
| 0x1E | 0x1458 | set another map's act: DLL 29 export 16(map = lo, act = hi) |
| 0x1F | 0x14A0 | checkpoint: DLL 29 export 6(trigger position, trigger yaw, lo, current layer) = where play restarts |
| 0x20 | 0x14EC | layer: hi == 0 → 80047374 (layer byte 0x80092A8C + 1, max 2), else 800473BC (layer - 1, min -2); both set 0x4000 in 0x80092A98 |
| 0x21 | 0x11F8 | **toggle one bit of a bit field**, own 17FC: field = arg & 0x1FFF, n = arg >> 13: BIT_SET(field, value ^ (1 << n)) |
| 0x22 | 0x1290 | toggle object group arg16 of this map (export 18 get, export 19 set !v) |
| 0x23 | 0x1520 | retry snapshot (DLL 29): hi 0 → export 8 (copy the save + position/yaw/layer as the restart copy), 1 → export 10 (drop the copy), 2 → export 9 (restore it) |
| 0x24, 0x25 | 0x16B8 | DLL 6 (AMSFX) export 19(hi) |
| 0x26 | 0x15C0 | sidekick (8002394C, first object of list 1, i.e. Tricky): hi 0 → its export 23; 1 → destroy it (80022338); 2 → find nearest object of category 0x34 (else 0x33) and give it to the sidekick's export 22 |
| > 0x26 | - | skipped |

DLL 29 tables (own+1F90): map act bit per map (map 7: 0xE1, 4-bit) and object-group bit field per map
(own+1F90+0xF0; map 7: 0x452, 32-bit). So groups and acts are game bits too, but never 0xC3.

## Map 7 (SwapStone Hollow) triggers in plain words

"Forward" for a plane = crossing from its front (normal) side to its back. All have +0x43 = 0 (player) and no gate
bit. Done bit is -1 except 307f5/307f6 (bit 0, a dummy). Positions are map-local.

- **18dc** (plane, 604,-787,236): every crossing either way toggles object group 1 (`0f220001`). `080008ff` is dead
  (cond 0x08 alone), `0a000015` is an empty op.
- **18ea** (plane, 1694,-828,2666): crossing back→front, every time, runs lighting actions 0x185, 0x186, 0x188, 0x18A.
  The forward ones (0x191, 0x192, 0x193) have cond 0x04 alone and never run.
- **30595** (TriggerPoint, 2876,-561,1766, radius 0x33*2 = 102): the first time the player leaves the sphere: DLL 81
  export 4 (empty) and export 1 with s3 = -1: environment action 0x60, music action 4 or 1 (by DLL 7 export 8, likely
  time of day), and **clears bit 0x178** (export 1 with s3 = +1 would run env 0x142, music 5 and set 0x178, but cond
  0x02 never fires it on entry). Note SHseqobject 42272 in map 7 has 0x0178 as its first parameter.
- **3079c** (plane, 1720,-828,2680, next to 18ea): crossing back→front, every time: environment actions 0xB4, 0xB5,
  0xB6. The forward ones (0x5F, 0x60, 0x61) are dead (cond 0x04).
- **307f5, 307f6, 307f7, 307f8, 307fa** (planes around 2418-3118, -529..-631, 1514-1890): forward: camera preset 0x2C
  (kind 0), lighting 0x194, group 5 on (not 307f8), lighting 0x196 (307f5/6 only); backward: camera preset 1, lighting
  0x191, group 5 off (not 307f8). Every time. A doorway/zone edge that switches camera, light and group 5.
- **19a5** (plane, 1061,-652,1597): forward: group 4 on, world switch 6 on (8001EBD0(1)), layer down one;
  backward: group 4 off, switch 6 off, layer up one. Every time. (A floor/level change.)
- **308c6** (plane, 2681,-707,667): every crossing toggles group 0. Second record is an empty op.
- **31c1b** (plane, 1673,-828,2654): every crossing toggles group 6 and turns group 8 off. The other records are dead
  (cond 0x08, 0x08, 0x00).
- **31cca** (TriggerPoint, 1138,-797,539, radius 0x87*2 = 270): entering starts sound 0x604, leaving stops it
  (ambient loop). Chain id +0x38 = 1. **31cdd** (2381,-780,892, radius 240): same with sound 0x605.
- **34640** (plane, 1678,-797,519): every crossing toggles groups 6 and 7. **34732** (2806,-666,858): toggles group 8.
- **40b72** (1858,-828,2624), **40b75** (320,-835,-74), **40b79** (2388,-633,1592): the first crossing in each
  direction stores a checkpoint (restart position = the trigger, its yaw, current layer; lo = 0). 40b79 also
  toggles **bit 0x887** on every crossing (op 0x12 mode 2): SHswapstone (DLL 515) only works while 0x887 and 0x15A are
  set, so this is the "player is in the SwapStone's zone" flag (in → on, out → off). **40b7a** (3100,-789,-198):
  checkpoint on every crossing.
- **429da** (plane, 1503,-742,2090): every crossing toggles group 8; backward camera preset 0x3D, forward preset 1.
  **42ba3** (1483,-683,273): forward preset 0x3D, backward preset 1.
- **429ee** (plane, 989,-637,1408): every crossing toggles bit 0 of bit field 0x64B (op 0x21; 0x64B is 8 bits,
  bank 3; no constant reader in bits.txt, so its reader takes it from parameters or a table).

## Bit 0xC3

- 0xC3: bank 0 (not saved), 1 bit. bits.txt: SET by dll501_0018 and read by dll501_00A8 (SHtricky, one in map 7).
  SHseqobject 34841 (309) in map 7 at (1175,-631,1257) holds 0x00C3 in both of its first two parameters.
- Ways a DLL 325 trigger could set it:
  - op 0x12 with arg 0x40C3 (set) or 0x80C3 (toggle). Arg **0x00C3 with op 0x12 clears it** (mode 0).
  - op 0x21 with arg 0x00C3 toggles bit 0 of 0xC3, so it sets it when clear (and clears it when set).
  - the done bit: a trigger with +0x44 = 0x00C3 sets it on its first s3 > 0 call (plane forward crossing, entering a
    point/area, a TriggerTime expiring, a TriggerBits condition met, a 0x50 trigger at once).
  - ops 0x11 / 0x1C / 0x1D write a value into fixed bits (0x4E2, 0x46E, 0x488), never a bit given by the argument.
  - groups/acts (0x13, 0x14, 0x18, 0x1A, 0x1B, 0x1E, 0x22) write map 7's 0x452 / 0xE1 fields, not 0xC3.
  - op 0x0B (start sequences by number) and op 0x19 (DLL 81-83 exports) could reach code that sets it indirectly;
    DLL 81's exports set only 0x351, 0x178, 0x177.
- Scan of every DLL 325 trigger in maps 0-120 (agent2/alltrig.txt, 1250 objects): **none** has op 0x12 with
  bit 0xC3, op 0x21 with field 0xC3, or +0x44 / +0x48 = 0xC3. Op 0x0B is used by no trigger at all (op counts:
  0x12: 504, 0x0D: 688, 0x13: 441, 0x06: 455, 0x0A: 364, 0x1C: 244, 0x03: 200, 0x19: 2, 0x0B: 0). The only DLL 325
  triggers holding 0xC3 anywhere are music actions (`xx0300c3`, op 3 arg 0xC3) in maps 4 and 35. No map 7 trigger
  can set 0xC3.
- Sequence op: op 0x0B, argument lo = sequence number 0..44 matched against a sequence object's param byte +0x1F (all
  five SHseqobjects in map 7 have 0 there); hi selects start (0/3) or set/clear the number's flag (1/2).
