exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/lib.py').read())
a21,s21=rfile(21)
grid={}
for i in range(s21//12):
    x,z,l,m=struct.unpack('>iihh',rom[a21+i*12:a21+i*12+12]) if False else struct.unpack('>hhhh',rom[a21+i*12:a21+i*12+8])+()
for i in range(s21//12):
    r=rom[a21+i*12:a21+i*12+12]
    grid.setdefault(struct.unpack('>h',r[8:10])[0] if False else None,0)
print(rom[a21:a21+48].hex())
for m in (7,8):
    for oid,x,y,z,uid,raw in mapobjs(m):
        n,d=objname(oid)
        if n in('SHbluemushroom','SHqueenearthwal','SHtricky','SHseqobject','WarpPoint','setuppoint'):
            print(m,n,'%.0f %.0f %.0f'%(x,y,z),'uid %x'%uid,'hdr',raw[2:8].hex(),'p20=%d'%raw[0x20] if len(raw)>0x20 else '',raw[0x18:].hex())
