exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import zlib
a36,s36=rfile(36);a37,s37=rfile(37);a38,s38=rfile(38);a34,s34=rfile(34)
t37=[w(a37+i*4) for i in range(s37//4)]
def block(i):
    o=t37[i]&0xffffff; e=t37[i+1]&0xffffff
    return zlib.decompressobj(-15).decompress(rom[a36+o+5:a36+e])
m=7
t=[w(a32+(m*7+i)*4) for i in range(8)]
W,H=struct.unpack('>HH',rom[a31+t[0]:a31+t[0]+4])
print(W,H)
g=[]
for z in range(H):
    r=[]
    for x in range(W):
        v=w(a31+t[1]+(z*W+x)*4)
        r.append(v>>17)
    g.append(r);print(r)
b34=[struct.unpack('>H',rom[a34+i*2:a34+i*2+2])[0] for i in range(s34//2)]
b38=[struct.unpack('>H',rom[a38+i*2:a38+i*2+2])[0] for i in range(s38//2)]
for v in sorted({x for r in g for x in r if x<4000}):
    mod=v>>6;sub=v&63
    print(v,mod,sub,'b34',b34[mod] if mod<len(b34) else None)
    idx=None
    if mod<len(b34) and b34[mod]+sub<len(b38): idx=b38[b34[mod]+sub]
    print(' b38',idx)
