import re,sys
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
imports=g.dll_imports(rom)
ds={d.number:d for d in g.dlls(rom)}
T=int(sys.argv[1],16)
cur=None;dll=None;regs={}
pend=None
for l in open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/a/all.asm'):
    if l.startswith('== '):
        cur=l.split()[1];regs={}
        m=re.match(r'dll(\d+)_',cur); dll=ds.get(int(m.group(1))) if m else None
        continue
    if dll is None or l.startswith('L_'): continue
    t=l.split(': ',1)
    if len(t)<2: continue
    a,t=t[0],t[1].strip()
    m=re.match(r'lw \$(\w+), (-?0x[0-9A-F]+|\d+)\(\$gp\)',t)
    if m:
        i=int(m.group(2),16)//4
        e=dll.got[i] if 0<=i<len(dll.got) else 0
        regs[m.group(1)]=('imp',imports[(e&0x7fffffff)-1]) if e&0x80000000 else None
    m=re.match(r'addiu \$(\w+), \$zero, (-?0x[0-9A-F]+|\d+)',t)
    if m: regs[m.group(1)]=int(m.group(2),16)
    m=re.match(r'or \$(\w+), \$(\w+), \$zero',t)
    if m: regs[m.group(1)]=regs.get(m.group(2))
    if pend:
        print(cur,a,'a0=',regs.get('a0'),'a1=',regs.get('a1'),'a2=',regs.get('a2')); pend=None
    if t.startswith('jalr $t9') and regs.get('t9')==('imp',T): pend=1
