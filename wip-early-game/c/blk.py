exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/rf.py').read())
import zlib
a36,s36=rfile(36);a37,s37=rfile(37)
t=[w(a37+i*4) for i in range(s37//4)]
print(len(t),[hex(x) for x in t[:8]])
for bi in (0,1,2,3):
    o=t[bi]&0xffffff; e=t[bi+1]&0xffffff
    d=rom[a36+o:a36+e]
    print(bi,hex(o),len(d),d[:24].hex())
    for sk in range(0,12):
        try:
            z=zlib.decompressobj(-15); r=z.decompress(d[sk:]); print(' raw deflate at',sk,len(r),r[:32].hex()); break
        except Exception as ex: pass
a=rfile(21)
print(rom[a[0]:a[0]+64].hex())
