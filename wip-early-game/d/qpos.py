exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/lib.py').read())
import sys
m=int(sys.argv[1]); want=sys.argv[2:]
for oid,x,y,z,uid,raw in mapobjs(m):
    n=objname(oid) if 'objname' in globals() else str(oid)
    if any(w.lower() in str(n).lower() for w in want):
        print(n, hex(uid), round(x-8960 if False else x), round(y), round(z), raw[:0x20].hex())
