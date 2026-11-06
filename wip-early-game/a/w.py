exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
a,s=rfile(30)
for i in (15,82,14,85,11,66,64,81,80,86,2,26):
    x,y,z=struct.unpack('>fff',rom[a+i*16:a+i*16+12]); print(i,round(x),round(y),round(z),struct.unpack('>i',rom[a+i*16+12:a+i*16+16]), 'local7',round(x+8960),round(z+3200))
for m in range(98):
    try: L=mapobjs(m)
    except: continue
    for oid,x,y,z,uid,raw in L:
        n,d=objname(oid)
        if d==275: print(m,mapname(m),n,'%.0f %.0f %.0f uid %x +3=%02x'%(x,y,z,uid,raw[3]),raw[0x18:].hex())
