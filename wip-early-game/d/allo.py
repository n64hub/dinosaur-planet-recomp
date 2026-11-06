exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/lib.py').read())
import sys,collections
m=int(sys.argv[1]); c=collections.Counter()
for oid,x,y,z,uid,raw in mapobjs(m):
    n,d=objname(oid); c[(n,d,raw[3],raw[4]&0x10 and raw[6] or -1)]+=1
for k,v in sorted(c.items(), key=lambda kv: kv[0][0]): print(v,*k)
