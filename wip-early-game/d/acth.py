exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/lib.py').read())
import sys
for m in map(int,sys.argv[1:]):
    c=collections.Counter()
    for oid,x,y,z,uid,raw in mapobjs(m):
        n,d=objname(oid)
        if n=='curve': continue
        c[(raw[3],raw[5],n)]+=1
    for (a,b,n),k in sorted(c.items()): print(m,'+3=%02x +5=%02x'%(a,b),k,n)
