import sys,struct
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
ds={d.number:d for d in g.dlls(rom)}
d=ds[int(sys.argv[1])];off=int(sys.argv[2],16);n=int(sys.argv[3],16)
st=d.text_rom+d.text_size
b=rom[st+off:st+off+n]
for i in range(0,len(b),16): print('%04x'%(off+i),b[i:i+16].hex(' ',2))
