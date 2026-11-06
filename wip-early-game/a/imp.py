import sys,re
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
imports=g.dll_imports(rom)
ds={d.number:d for d in g.dlls(rom)}
T=[int(x,16) for x in sys.argv[1:]]
for t in T:
    for n,d in ds.items():
        for e in d.got:
            if e&0x80000000 and imports[(e&0x7fffffff)-1]==t: print(hex(t),'dll',n)
