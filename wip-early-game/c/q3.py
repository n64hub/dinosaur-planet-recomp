exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
import collections
S=shapes(7)
c=collections.defaultdict(collections.Counter)
for s in S:
    for t in s['tris']:
        ys={t[0][1],t[1][1],t[2][1]}
        if len(ys)==1:
            area=abs((t[1][0]-t[0][0])*(t[2][2]-t[0][2])-(t[2][0]-t[0][0])*(t[1][2]-t[0][2]))/2
            c[s['cell']][t[0][1]]+=int(area)
for k in sorted(c): print(k,c[k].most_common(5))
