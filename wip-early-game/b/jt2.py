import sys,struct
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
ds={d.number:d for d in g.dlls(rom)}
d=ds[int(sys.argv[1])]; off=int(sys.argv[2],16); n=int(sys.argv[3])
# off= offset from text start; words are distance from GOT (got_vram)
p=d.text_rom+off
gv=d.got_vram-d.vram if callable(getattr(d,'got_vram',None))==False else d.got_vram()-d.vram
print('gotoff',hex(gv))
for i in range(n):
    w=struct.unpack('>i',rom[p+4*i:p+4*i+4])[0]
    print(i,hex(w),hex(w+gv))
