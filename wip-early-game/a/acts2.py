exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import sys
for m in [int(x) for x in sys.argv[1:]]:
    print('MAP',m,mapname(m))
    c=collections.Counter((o[5][3],o[5][5]) for o in mapobjs(m))
    print({('%02x'%a,'%02x'%b):v for (a,b),v in sorted(c.items())})
    for oid,x,y,z,uid,raw in mapobjs(m):
        n,d=objname(oid)
        if n=='curve': continue
        if raw[3] not in (0,) or raw[5] not in (4,0xe4,0x84):
            print(' %-16s %3d %6.0f %6.0f %6.0f uid %-6x +3=%02x +4=%02x +5=%02x +6=%02x +7=%02x %s'%(n,d,x,y,z,uid,raw[3],raw[4],raw[5],raw[6],raw[7],raw[0x18:0x38].hex()))
