import re,glob
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000/RecompiledFuncs/'
res={}
for f in glob.glob(B+'funcs_*.c'):
    L=[l for l in open(f,errors='replace') if '// 0x8' in l or l.startswith('RECOMP_FUNC')]
    cur=None
    for i,l in enumerate(L):
        if l.startswith('RECOMP_FUNC'): cur=l.split()[2].split('(')[0]; continue
        if 'jal' in l and '0x800126E0' in l:
            v=None
            for k in L[max(0,i-12):i+2]:
                m=re.search(r'addiu\s+\$a1, \$zero, 0x([0-9A-F]+)',k)
                if m: v=int(m.group(1),16)
            res.setdefault(v,set()).add(cur)
for k in sorted(res,key=lambda x:x or 0): print(k,sorted(res[k])[:6])
