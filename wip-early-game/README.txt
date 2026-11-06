Tools for reading Dinosaur Planet (all in this folder, from Git Bash; `SC=<this folder>`; cd $SC first):
 ./py d.py <dll>                  summary: each function of a DLL, its calls and constant bit GET/SET
 ./py d.py <dll> <off> [<off>..]  annotated disassembly of those functions (GOT resolved; "all" = every function). Pipe through cut/sed: it is long.
 ./dis func_8001234                MIPS of a main-segment function
 bits.txt                          every constant bit use: "<bit> GET|SET dll<n>_<func> <offset> a1=.." (grep it). "var" = bit not constant
 ./py mapdump.py <map>             a map's objects by name and DLL
 ./py mapo.py <map> '<regex>'      those objects with position, uid, header bytes, parameters (raw hex from +0x18)
 ./py findbit.py <hex>...          objects in any map whose parameters hold that bit number
 ./py objs.py '<regex>'            object definitions by name -> DLL
 ./py bydll.py <dll>...            which objects use a DLL, which maps hold them
 ./py bitinfo.py <hex>...          a bit's bank, length and task (table in ROM file 55)
 ./py tasks3.py                    the task list in order; ./py tasks2.py keyword search in the game text (edit its regex)
Facts: object DLL value 0x8000|n means DLL n+209. Maps: file 31 data, file 32 table (7 words a map; section 4 = its objects), file 33 names (32 bytes each), file 21 grid (12-byte records: x, z, layer, map; world = local + 640*grid). Map 7 SwapStone Hollow is at grid (-14,-5), so world = local + (-8960, 0, -3200). Object entry: u16 id (through file 67), u8 size in words, then +3.. header bytes, x y z at +8, uid at +0x14, params from +0x18.
A dll's export n is word n+1 of the table its handle points at. Handles live in main-segment words 0x8008C9xx. BIT_SET = func_80014670(bit,value), BIT_GET = func_800147EC(bit).
Repo for reading (do not edit): C:\Users\Benla\source\bot-worktrees\dinoplanet-early-17939592916570000 (CLAUDE.md, docs/story-points.md say how the game is laid out; RecompiledFuncs/ holds every function as C with MIPS comments).

Playing (each play is one GPU-queue turn; start it, it blocks until the turn is granted and ends by itself):
  cat > $SC/<you>/t.sh <<X
  cd $SC; FROM=20 EVERY=5 bash run.sh <unique-name> 40 --story new-game,15 --bits 64b
  X
  "/c/Users/Benla/AppData/Local/HoloCareBot/node/node.exe" /c/Users/Benla/AppData/Local/HoloCareBot/code/gpu.mjs --run --shared --max 3 -- bash $SC/<you>/t.sh
Results: runs/<name>/sNNN.png (read them), stderr.log has --report's line a second (map, position, health) and a stack on a crash. INPUT=<file> before bash run.sh plays a script, lines "<seconds> <buttons hex> [x y]" from the game's first input read, last line held (A=8000 B=4000 Z=2000 START=1000 C-up=8 C-down=4 C-left=2 C-right=1; x y stick -80..80).
Options: --story <point>[,<warp>], --place x,y,z[,layer] (world coordinates), --bits <hex>[=<value>],... docs/story-points.md says more. The game takes ~8 s to reach the start of a --story run.
