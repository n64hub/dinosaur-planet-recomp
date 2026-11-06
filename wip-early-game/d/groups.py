exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/lib.py').read())
import sys
for m in map(int,sys.argv[1:]):
    g=collections.defaultdict(collections.Counter)
    for oid,x,y,z,uid,raw in mapobjs(m):
        n,d=objname(oid)
        if raw[4]&0x10: g[raw[6]][n]+=1
    for k in sorted(g): print(m,'group',k,dict(g[k]))
