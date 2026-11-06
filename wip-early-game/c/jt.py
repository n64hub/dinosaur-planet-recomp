# jt.py <dll> <gotword-offset-of-rodata e.g. 2E14> <table-off> <n>: prints the jump table; values are GOT-relative
import sys,struct
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
ds={d.number:d for d in g.dlls(rom)}
n=int(sys.argv[1]);d=ds[n]
base=int(sys.argv[2],16);off=int(sys.argv[3],16);cnt=int(sys.argv[4])
# rodata is after text? got entry offsets are relative to text start
print('text',hex(d.text),'data',hex(d.data),'rel',hex(d.rel),'size',hex(d.size),'textsize',hex(d.text_size))
got_off=d.rel-d.text
p=d.text_rom+base+off
for i in range(cnt):
    v=struct.unpack('>i',rom[p+i*4:p+i*4+4])[0]
    print(hex(i+1),hex(v+got_off) if v else 0, hex(v&0xffffffff))
