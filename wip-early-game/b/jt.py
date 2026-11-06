import sys
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
ds={d.number:d for d in g.dlls(rom)}
d=ds[int(sys.argv[1])]
print([a for a in dir(d) if not a.startswith('_')])
print(hex(d.vram), d.got[:8] if hasattr(d,'got') else None)
