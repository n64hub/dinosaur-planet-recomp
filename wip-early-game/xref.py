# xref.py: every constant BIT_SET/BIT_GET in every DLL -> bits.txt
import re,sys
sys.argv=['d.py','0','none']
src=open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/d.py').read().split("n=int(sys.argv[1])")[0]
exec(src)
out=open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/bits.txt','w')
cache={}
files=collections.defaultdict(list)
for k,(f,i) in idx.items(): files[f].append((i,k))
for f,l in files.items():
    L=open(f,errors='replace').readlines(); l.sort()
    for j,(i,k) in enumerate(l):
        mm=re.match(r'dll(\d+)_([0-9A-F]+)',k)
        if not mm: continue
        n=int(mm.group(1)); d=ds.get(n)
        if not d: continue
        end=l[j+1][0] if j+1<len(l) else len(L)
        regs={};seen=set()
        for ln in L[i:end]:
            m=re.match(r'\s*// 0x([0-9A-F]{8}): (.*)',ln)
            if not m: continue
            a=int(m.group(1),16)
            if a in seen: continue
            seen.add(a); t=re.sub(r'\s+',' ',m.group(2))
            m=re.match(r'lw \$(\w+), (-?0x[0-9A-F]+|\d+)\(\$gp\)',t)
            if m: regs[m.group(1)]=got(d,int(m.group(2),16))
            m=re.match(r'(addiu|ori) \$(\w+), \$zero, (-?0x[0-9A-F]+|\d+)',t)
            if m: regs[m.group(2)]=int(m.group(3),16)
            if t.startswith('jalr $t9'):
                g_=regs.get('t9')
                if g_ in('BIT_SET','BIT_GET'):
                    # delay slot might set a0: peek handled by next comment; keep simple
                    out.write('%s %s dll%d_%s %04X a1=%s\n'%(hex(regs['a0']) if isinstance(regs.get('a0'),int) else 'var',g_[4:],n,mm.group(2),a-d.vram,regs.get('a1')))
out.close()
