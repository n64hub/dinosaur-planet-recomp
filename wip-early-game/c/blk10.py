exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
m=7
t=[w(a32+(m*7+i)*4) for i in range(8)]
W,H,ox,oz=struct.unpack('>HHhh',rom[a31+t[0]:a31+t[0]+8])
for z in range(H):
    for x in range(W):
        word=w(a31+t[1]+(z*W+x)*4)
        v=word>>17
        if v>=4000: continue
        d,h=blk41(v)
        print((x-ox,z-oz),v,hex(word),h.hex(),d[0x38:0x50].hex())
