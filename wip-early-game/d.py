# d.py <dll> [offset-hex ...] : annotated disassembly of a DLL from RecompiledFuncs' comments; no offsets = summary of bits and calls per function
import sys,re,bisect,glob,os,pickle,collections
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
imports=g.dll_imports(rom)
ds={d.number:d for d in g.dlls(rom)}
IDX='/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/idx.pickle'
if os.path.exists(IDX): idx=pickle.load(open(IDX,'rb'))
else:
    idx={}
    for f in sorted(glob.glob(B+'/RecompiledFuncs/funcs_*.c')):
        for i,l in enumerate(open(f,errors='replace')):
            if l.startswith('RECOMP_FUNC void'):
                idx[l.split()[2].split('(')[0]]=(f,i)
    pickle.dump(idx,open(IDX,'wb'))
NAMES={0x80014670:'BIT_SET',0x800147EC:'BIT_GET',0x80023914:'player',0x8004D1D4:'warp'}
def lines(fn):
    f,i=idx[fn]; out=[]; seen=set()
    with open(f,errors='replace') as h:
        L=h.readlines()
    for l in L[i+1:]:
        if l.startswith('RECOMP_FUNC'): break
        m=re.match(r'\s*// 0x([0-9A-F]{8}): (.*)',l)
        if m:
            a=int(m.group(1),16)
            if a not in seen: seen.add(a); out.append((a,re.sub(r'\s+',' ',m.group(2))))
    return out
def got(d,off):
    i=off//4
    if i>=len(d.got): return None
    e=d.got[i]
    if e&0x80000000:
        t=imports[(e&0x7fffffff)-1]; return NAMES.get(t,'%X'%t)
    return 'own+%X'%e
def anno(d,fn):
    res=[];regs={}
    for a,t in lines(fn):
        c=''
        m=re.match(r'lw \$(\w+), (-?0x[0-9A-F]+|\d+)\(\$gp\)',t)
        if m:
            off=int(m.group(2),16); r=got(d,off); c=' ; GOT '+str(r); regs[m.group(1)]=r
        m=re.match(r'(addiu|ori) \$(\w+), \$zero, (-?0x[0-9A-F]+|\d+)',t)
        if m: regs[m.group(2)]=int(m.group(3),16)
        m=re.match(r'jalr \$t9',t)
        if m:
            c=' ; CALL %s a0=%s a1=%s'%(regs.get('t9'),regs.get('a0') if not isinstance(regs.get('a0'),int) else hex(regs['a0']),regs.get('a1') if not isinstance(regs.get('a1'),int) else hex(regs['a1']))
        m2=re.match(r"lw $(w+), (-?0x[0-9A-F]+|d+)($(w+))",t)
        if m2 and m2.group(3)!="gp" and m2.group(3)!="sp":
            b=regs.get(m2.group(3)); regs[m2.group(1)]=("%s>%s"%(b,m2.group(2))) if isinstance(b,str) else None
        elif not m and re.match(r"(or|addu|move) $(w+), $(w+), $zero",t):
            m3=re.match(r"(or|addu|move) $(w+), $(w+), $zero",t); regs[m3.group(2)]=regs.get(m3.group(3))
        res.append((a,t+c))
    return res
n=int(sys.argv[1]);d=ds[n]
fns=sorted([k for k in idx if k.startswith('dll%d_'%n)],key=lambda s:int(s.split('_')[1],16))
if len(sys.argv)>2:
    want=sys.argv[2:]
    for fn in fns:
        o=fn.split('_')[1]
        if want==['all'] or o in want or o.lstrip('0') in want:
            print('==',fn)
            base=d.vram
            for a,t in anno(d,fn): print('%04X %s'%(a-base,t))
else:
    print('dll',n,'exports',[hex(x) for x in d.exports],'ctors',[hex(x) for x in d.ctors],'text',hex(d.text_size))
    for fn in fns:
        calls=[]
        an=anno(d,fn)
        for a,t in an:
            m=re.search(r'; CALL (\S+) a0=(\S+) a1=(\S+)',t)
            if m:
                if m.group(1) in('BIT_SET','BIT_GET'): calls.append('%s(%s%s)'%(m.group(1)[4:],m.group(2),','+m.group(3) if m.group(1)=='BIT_SET' else ''))
                else: calls.append(m.group(1))
        print(fn.split('_')[1],len(an),' '.join(calls))
