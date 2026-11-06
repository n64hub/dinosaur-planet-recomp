import struct,sys
d=open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/b/ram1.bin','rb').read()
w=struct.unpack('<%dI'%(len(d)//4),d)
print(len(w))
def W(a): return w[(a&0x7fffff)>>2]
for uid in (0x120e,0x34950,0x34841):
    hits=[i*4 for i,v in enumerate(w) if v==uid]
    print(hex(uid),[hex(h) for h in hits][:20])
    for h in hits[:6]:
        print(' ',[hex(W(h-0x14+k)) for k in range(0,0x20,4)])
