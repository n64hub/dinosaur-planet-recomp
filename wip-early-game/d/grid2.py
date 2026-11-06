exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/rf.py').read())
a,s=rfile(21)
for i in range(s//12):
    v=[struct.unpack('>h',rom[a+i*12+2*j:a+i*12+2*j+2])[0] for j in range(6)]
    if v[3] in (7,8) or v[4] in (7,8) or v[5] in (7,8): print(i,v)
