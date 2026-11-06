exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/lib.py').read())
import sys,struct
for m in map(int,sys.argv[1:]):
  for oid,x,y,z,uid,raw in mapobjs(m):
    n,d=objname(oid)
    if d==309:
        done,trig=struct.unpack('>hh',raw[0x18:0x1C])
        print(m,hex(uid),round(x),round(y),round(z),'h',raw[3:8].hex(),'done',hex(done),'trig',hex(trig),'flags',hex(raw[0x1D]),'seq',raw[0x1E],raw[0x18:].hex())
