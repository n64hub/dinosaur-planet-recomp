exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import sys,re
m=int(sys.argv[1]);pat=re.compile(sys.argv[2],re.I)
for oid,x,y,z,uid,raw in mapobjs(m):
    n,d=objname(oid)
    if pat.search(n): print(n,d,'%.0f %.0f %.0f'%(x,y,z),'uid %x'%uid,'hdr',raw[2:8].hex(),raw[0x18:].hex())
