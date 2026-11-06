exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
m=7
t=[w(a32+(m*7+i)*4) for i in range(8)]
print([hex(x) for x in t])
h=rom[a31+t[0]:a31+t[0]+0x40];print(h.hex())
for i in range(1,7):
    print(i,hex(t[i]),rom[a31+t[i]:a31+t[i]+64].hex())
a38,s38=rfile(38);print(s38,rom[a38:a38+64].hex())
a34,s34=rfile(34);print(rom[a34:a34+64].hex())
print('38 @7',struct.unpack('>H',rom[a38+14:a38+16]))
