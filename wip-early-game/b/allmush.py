exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
for m in range(98):
    try: o=mapobjs(m)
    except: continue
    for oid,x,y,z,uid,raw in o:
        n,d=objname(oid)
        if d in (502,503,504,506,300) or 'ushroom' in n:
            print(m,mapname(m),n,d,'%.0f %.0f %.0f'%(x,y,z),'uid %x'%uid,raw[2:8].hex(),raw[0x18:].hex())
