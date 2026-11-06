# DLL 502 (SHbluemushroom): state machine, white mushroom pickup, softlock candidates

Offsets are DLL 502 text offsets as `d.py 502` prints them (label base 0x901FEF8C: `L_9xxxxxxx - 0x901FEF8C`).
Player = DLL 210 (label base 0x900E7E00). Helper scripts and dumps are in this folder (`jt.py`, `tb.py`, `mp.py`,
`mushlist.txt`, `d74.txt`, `p1ddc.txt`, `p4910.txt`, `p16220.txt`).

## Layout

- State struct = obj+0xB8. **State byte = +0x136 (u8)**, values 0..9 (jump table at text 0x18C8, 10 entries, 0xDF4).
- Flags byte +0x137: 0x01 animation cycle finished (set or cleared each frame from 80024108's result, 0x16AC-0x16CC);
  0x02 follows a curve (types 4/5, set in setup 0x2A8); 0x04 stuck on surface 0xD (type 4 only, set in 03A8 0x820,
  **never cleared**); 0x08 "moving, snap to ground" (cleared at 0xDB8 each frame, set in states 1/5); 0x10 "was hit"
  (set in 03A8 0x6D0, cleared only when the daze ends, 0x1458).
- +0x108 distance to the nearer of player and sidekick (03A8 0x50C-0x5B4), +0x10C last frame's distance;
  "closing speed" f16 = (0x10C-0x108)/dt (0xDF0-0xDFC). +0x110 = p1D/255, +0x114 = p1C/255*0.2, +0x118/+0x11C run
  distances, +0x124 daze timer, +0x128 star-effect timer, +0x12C looping sound handle, +0x130 heading, +0x134 counter
  bit (0x66D white, 0xC1 blue, 0x358-0x374).
- Map params (from +0x18): +0x18 type (1..5), +0x19 flee radius, +0x1A collected bit, +0x1C/+0x1D speeds, +0x1F
  alert radius, +0x20 model (white = 1; clamped to 0 if >= the def's model count at def+0x5D, 0xBC-0xD0; the
  SHbluemushroom def has 2 models, so white is valid).
- Setup 0018: obj+0xB0 |= 0x6000 (0x5C-0x68, not 0x40); message queue of **capacity 1** (80031620(obj,1), 0x27C);
  if the collected bit is already set: state 8, hidden (obj+6 |= 0x4000), collision bit cleared (0x80-0xA8).
- Every type runs the same state machine: 03A8's switch on type (table at 0x18B0) sends types 1-5 and the default
  all to 0x70C, which calls 0D74. Types 4/5 steer along a curve (0C70, DLL 26) instead of 0910.

## Q1. States and transitions (0D74, called from 03A8 0x71C when state != 8)

Before the switch, every frame: if flags & 4, state := 6 (0xDC0-0xDCC); then **obj+0xAF |= 0x08** (0xDE4-0xDE8),
which keeps the object out of the camera's interaction targeting. Only states 6 and 9 clear that bit again.

| state | anim (table 0x1904) | code | does | goes to |
|---|---|---|---|---|
| 0 idle | 0 | 0xE1C | | hit (0x10) -> 9; dist < p19 -> 1 (heading from 0910/0C70, sound 0x53C, 0xE40-0xF10); dist < p1F -> 3 (0xF14-0xF4C) |
| 1 run away | 1 | 0xF50 | moves (speed = anim delta * 0x110, 0x16D0-0x16F8); flags \|= 8 | hit -> 9; anim end -> 0 |
| 2 picked up | 6 | 0x1354 | untargetable | anim end -> 8, hide (obj+6 \|= 0x4000), 800267A4 (0x1368-0x1384) |
| 3 notices | 2 | 0xF80 | | hit -> 9; anim end -> 4; until then runs state 4's code (0xF9C -> 0xFBC) |
| 4 watches | 3 | 0xFBC | turns to face the player (0x1010) | hit -> 9; dist > p1F+10 -> 7; dist < p19: closing speed >= 0.54 -> 1, else -> 5 (sneaks off, distance 0x11C) (0x109C-0x1204) |
| 5 sneaks off | 4 | 0x1208 | root-motion move; flags \|= 8 | hit and anim end -> 9; dist > p19+10 and anim end -> 4; closing speed >= 0.54 -> 1 |
| 6 stuck | 0 | 0x1520 | **targetable** (0xAF &= ~8, 0x1538) | hit -> 9 (then forced back to 6 next frame) |
| 7 calms down | 5 | 0xF80 | | hit -> 9; anim end -> 0; until then runs state 4's code |
| 8 collected | (6) | 0x1544 / 03A8 0x400 | hidden; 0D74 never runs; 03A8 drains the queue | none |
| 9 dazed | 7 | 0x1390 | **targetable** (0x1510-0x151C); sound 0x745; stars 0x51D every 20 ticks; timer rand(240..300) (0x13E0-0x1404) | timer <= 0 -> 0, clears 0x10, removes the effects (0x1430-0x1488) |

After the switch, in any state 0D74 runs (0x1544): if obj+0xAF & 1 (A pressed on it), it is picked up (below), and
the state becomes 2.

- **Flee: yes.** Inside the flee radius p19 (80 for most white ones) it runs (state 1). Between p19 and p1F it watches,
  and when you approach it runs or sneaks off. 0910 picks a heading away from the nearer of the player and the
  sidekick, turning 20 degrees (0xE38) at a time, up to 8 tries, round obstacles that 80059C40 reports.
- **Stun: yes, required** (except type 4's state 6). States 0-5 and 7 are never targetable. 03A8 0x610-0x6D4: when
  80025F40 reports a hit by anything whose obj+0x46 is not 0x416, it sets flags 0x10, so the next 0D74 enters
  state 9. A hit by 0x416 instead sends it straight to state 8 without setting its bit, and calls the hitter's
  vtable+0x54(hitter,1,mushroom). 0x416 is a raw object id that maps to DR_EarthWarrior, one of the mounts the player
  looks for in its 4910 (0x22, 0x72, 0x38C, 0x416, 0x419, 0x18F). No message stuns it: the state machine reads no
  messages outside state 8.
- **Targetable:** only through the 0xAF bit 0x08 (set at 0xDE8 every frame, cleared in states 6 and 9). No code
  touches obj+0xB0 & 0x40 or +0x74/+0x78, and nothing touches 0xAF bit 0x20.
- Within p1F of the sidekick it also calls the sidekick's export 14 (03A8 0x5B8-0x604), which probably lets the
  sidekick sense it.

## Q2. Picking up a white mushroom

1. Get it dazed: hit it (staff or anything that registers through 80025F40) while it is in any state but 8. It
   has to be close, but it runs from you at about 80 units.
2. While it is dazed (state 9, roughly 240-300 ticks), stand in interaction range facing it and press A. The engine
   sets obj+0xAF bit 0.
3. 0x1544-0x163C: (blue only: tutorial 0x169 and bit 0x911 the first time) -> `8003168C(player, 0x7000A, obj, 0x119)`
   -> **BIT_SET(param bit, 1)** (0x15EC) -> 0xAF |= 8, state 2, its sound stopped. When the animation ends: state 8,
   hidden.
4. In state 8 (03A8 0x414-0x508), every 0x7000B it receives: white -> func_80014930(0x66D) (0x4D0). Blue -> if the
   sidekick's vtable+0x20 says so, it feeds the sidekick (vtable+0x30) instead of counting; otherwise it counts 0xC1.

White differs from blue only in this: no 0x169/0x911 tutorial (0x1554-0x1558), counter 0x66D, never feeds the
sidekick (0x444-0x448), and model 1. The movement, the daze and the pickup are the same.

All 10 white mushrooms are in SwapStone Hollow (map 7: 7 of them; map 8: 3), listed in mushlist.txt. Map 7's are all
type 1 (flee 80, alert 120, except uid 307d5 with alert 0). Map 8 has uid 30844 (type 1), uid 307d4 (type 3, which
behaves like type 1) and **uid 30832, type 4, bit 0x463**. The bits 0x460-0x464, 0x13, 0xF4, 0xF5, 0x15C and 0x177 are
consecutive slots in bank 1 (0x23-0x2C).

## Q3. Who answers 0x7000A, and where a pickup can be lost or soft-lock

Senders of 0x7000B: dll210_1DDC 0x22F0 and dll210_4910 0x5FE0 (the player), plus other DLLs that answer their own
messages (316, 452, 454, 523, 600, 272, 280, 291, 297, 300). Only the player answers the mushroom.

**Player side** (DLL 210). 1DDC is the player's message loop, called from its update (0934 at 0xAD0). Its queue holds
20 messages (0x4C8).
- 0x22C8: on 0x7000A with arg 0x119 > 0, it calls BIT_GET(0x119), a generic "has picked up a mushroom before" flag:
  - **Set:** it answers 0x7000B at once (0x22E8-0x22FC) and enters player state 0x2B (dll210_164DC, a quick pickup
    animation that hides the mushroom and, at the end of its progress, clears 0x708 and returns 2, 0x165B4).
  - **Clear (the first mushroom ever):** BIT_SET(0x119) and player state 0x2A (dll210_16220), with **no answer**.
    16220 plays animation 0xC7, hides the mushroom and fades. When state+0x33A is set, it starts the player's
    sequence 0 through DLL 3 (+0x48, 0x16408-0x16424). The answer only comes from the player's sequence callback 4910
    (obj+0xBC, set at 0x4E4), 0x5F18-0x5FFC: while 0x708 != 0 and the sequence's event byte +0x8D == 1, it sends 0x7000B
    to 0x708 and clears it. Event 0x1A shows the def's text 0x257 through DLL 1.
  - Both paths store the sender in player+0x708 (0x23CC). The state table is at DLL 210 data 0x1E2E0, with
    0x2A -> 0x16220 and 0x2B -> 0x164DC (`st.py`).
- **Ways it never answers the mushroom:**
  - (a) First pickup: the sequence never reaches event 1, because it is skipped or aborted.
  - (b) player+0x708 is cleared before the event: the player's hit reaction dll210_2534 at 0x2BE8-0x2BEC clears
    0x708 (and 0x868) when a hit changes the player's state.
  - (c) 0x708 is overwritten by another 0x7000A sender (0x23CC) or by export 0x1D210.
  - (d) The player's 20-message queue overflows: 8003168C silently drops a message to a full queue, and the mushroom
    never checks.
  - (e) The mushroom is unloaded before the answer, leaving 0x708 dangling: the message is written into freed memory.

**Mushroom side:**
1. **Type 4 can never count (strongest).** Once a type-4 mushroom's sweep hits surface 0xD (03A8 0x7F4-0x828),
   flags 0x04 is set and never cleared. 0xDC0 then forces state 6 every frame, so after a pickup state 2 becomes 6
   the next frame and never reaches 8. Only state 8 reads messages, so the 0x7000B is never processed, while the
   collected bit was already set at 0x15EC. The player's code hides it anyway. Map 8's white uid 30832 (bit 0x463) is
   type 4: caught that way, it is gone for good and 0x66D never rises. Only 9 of the 10 can then be counted.
2. **The bit is set before the count.** BIT_SET(param bit) happens at pickup (0x15EC); the count only on 0x7000B in
   state 8. So any lost answer (player side a-e, item 3) loses the mushroom permanently: setup puts it in state 8,
   hidden, on reload. For the first pickup the window covers the whole cutscene.
3. **One-message queue, never drained before state 8.** If anything messages the mushroom before it is collected,
   that message sits in the only slot, and the later 0x7000B is dropped (func_8003168C checks count < capacity).
   Example: the player broadcasts 0xE0000 to every object (func_80031700 with flags 3, which skips only the sender) at
   dll210 0x6280-0x6294 (618C, health reaching zero) and 0x9FF0-0x9FFC (9F1C). That is harmless if a death reloads the
   objects. Many DLLs import 80031700/80031864 (DLL 3, 71, 216, 222, 228-240, 252, 253, 279, 325, 352, 367, 372,
   383, 688, 708, 710). I did not trace which of them reach the mushrooms with flag 2 or by radius.
4. The counter is not the problem. func_80014930 stops at 2^len-1 (0x66D: 4 bits, 15), and the queen (DLL 507 00A0)
   banks 0x66D into her own tally / bit 0xC2 and resets 0x66D (0x2F0-0x2FC).
5. **Bit collision, not confirmed:** bit 0x177 (white uid 308f5, map 7) is also written by DLL 81 export 0x224:
   BIT_SET(0x177,1) at 0x2B8 and BIT_SET(0x177,0) at 0x3AC, next to music calls. If that runs before the mushroom is
   collected, the mushroom loads as already collected and cannot be got. If it runs after, it comes back. I did not
   find who calls DLL 81.
6. 0x2B path: 164DC clears 0x708 at the end, so there is normally no double answer. A double answer would be dropped
   by the full slot anyway, unless the mushroom had already reached state 8.
7. A pickup cannot happen before the daze unless the engine sets 0xAF bit 0 on an untargetable object.

No state waits forever except state 8 (waiting for 0x7000B, by design) and type 4's state 6 (item 1). None of them
holds the player: the player's 0x2A/0x2B states end on their own animations and sequence. A softlock in white
mushroom collection is most likely a count that never reaches what the queen wants: item 1 (uid 30832 caught while
stuck), or a lost first-pickup answer (player side a/b, item 2).
