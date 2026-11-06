exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import sys
bits=[int(x,16) for x in sys.argv[1:]]
for m in range(98):
    try: o=mapobjs(m)
    except Exception as e: continue
    for oid,x,y,z,uid,raw in o:
        for b in bits:
            p=0
            while True:
                p=raw.find(struct.pack('>H',b),p)
                if p<0: break
                if p>=0x18 and p%2==0:
                    n,dl=objname(oid)
                    print(hex(b),'map',m,mapname(m),n,dl,'%.0f %.0f %.0f'%(x,y,z),'uid %x'%uid,'at',hex(p),raw[0x18:].hex())
                p+=2
