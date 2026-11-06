exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import sys
for m in map(int,sys.argv[1:]):
    o=mapobjs(m);print('MAP',m,mapname(m),len(o))
    c=collections.Counter()
    for oid,x,y,z,uid,raw in o:
        n,d=objname(oid);c[(n,d)]+=1
    for (n,d),k in sorted(c.items(),key=lambda t:t[0][1]): print('  ',k,n,d)
