exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/lib.py').read())
import sys
for m in map(int,sys.argv[1:]):
  for oid,x,y,z,uid,raw in mapobjs(m):
    n,d=objname(oid)
    if d==325:
        acts=[raw[i:i+4].hex() for i in range(0x18,len(raw),4)]
        print(m,n,hex(uid),round(x),round(y),round(z),'h',raw[3:8].hex(),' '.join(acts))
