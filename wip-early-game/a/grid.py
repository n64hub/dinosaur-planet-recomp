exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
a,s=rfile(21)
for i in range(s//12):
    v=[struct.unpack('>h',rom[a+i*12+2*j:a+i*12+2*j+2])[0] for j in range(6)]
    m=v[3]
    try: h=rom[a31+w(a32+(m*7)*4):a31+w(a32+(m*7)*4)+8]; d=struct.unpack('>hhhh',h)
    except: d=None
    if m in (5,6,7,8,14,29,35,4,10,11,15,16,17,12,13,9) or v[4] in(7,8,35) or v[5] in (7,8,35):
        print(i,v,mapname(m) if m>=0 else '',d)
