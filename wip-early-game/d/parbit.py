exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/lib.py').read())
import sys,struct
want=[int(x,16) for x in sys.argv[2:]]
for m in map(int,sys.argv[1].split(',')):
  for oid,x,y,z,uid,raw in mapobjs(m):
    for o in range(0x18,len(raw)-1,2):
        v=struct.unpack('>H',raw[o:o+2])[0]
        if v in want: print(m,objname(oid),hex(uid),'off',hex(o),hex(v),round(x),round(y),round(z))
