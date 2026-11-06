exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
S=shapes(7)
print(len(S),sum(len(s['tris']) for s in S))
import collections
print(collections.Counter(hex(s['fl']) for s in S).most_common(30))
for s in S:
    if s['sid'] or s['rec'][0x13]:
        vs=s['verts']
        if not vs: continue
        print(s['cell'],s['i'],hex(s['fl']),'id',hex(s['sid']),'x',min(a[0] for a in vs),max(a[0] for a in vs),'y',min(a[1] for a in vs),max(a[1] for a in vs),'z',min(a[2] for a in vs),max(a[2] for a in vs),len(s['tris']))
