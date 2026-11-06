import sys,struct
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
ds={d.number:d for d in g.dlls(rom)}
d=ds[325]
print(hex(d.text),hex(d.data),hex(d.rel),d.got[:4])
base=d.rom_addr+d.text
gpv=d.rel-d.text
print('gp',hex(gpv))
for n,o,c in (('cmd',0x2E14+0x6D0,0x26),('sub0',0x2E14+0x768,0xb),('sub1',0x2E14+0x794,8)):
    t=struct.unpack('>%di'%c,rom[base+o:base+o+4*c])
    print(n,[hex((x+gpv)&0xffff) if 0 else hex(x) for x in t])
