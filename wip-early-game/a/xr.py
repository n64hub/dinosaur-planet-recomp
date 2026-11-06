# find uses of dll29 exports by word offset: in DLLs the handle comes through GOT; decode via d.py machinery
import re,sys,pickle
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
imports=g.dll_imports(rom)
ds={d.number:d for d in g.dlls(rom)}
want=set(int(x,16) for x in sys.argv[2:])
H=int(sys.argv[1],16)
cur=None;dll=None;regs={}
for l in open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/a/all.asm'):
    if l.startswith('== '):
        cur=l.split()[1];regs={}
        m=re.match(r'dll(\d+)_',cur); dll=ds.get(int(m.group(1))) if m else None
        continue
    if dll is None or l.startswith('L_'): continue
    t=l.split(': ',1)
    if len(t)<2: continue
    a,t=t[0],t[1].strip()
    m=re.match(r'lw \$(\w+), (-?0x[0-9A-F]+|\d+)\(\$(\w+)\)',t)
    if m:
        d,o,b=m.group(1),int(m.group(2),16),m.group(3)
        if b=='gp':
            i=o//4
            e=dll.got[i] if 0<=i<len(dll.got) else 0
            regs[d]=('imp',imports[(e&0x7fffffff)-1]) if e&0x80000000 else None
        elif regs.get(b)==('imp',H) and o==0: regs[d]=('h',)
        elif regs.get(b)==('h',) and o==0: regs[d]=('t',)
        elif regs.get(b)==('t',):
            regs[d]=None
            if o in want: print(cur,a,hex(o))
        else: regs[d]=None
