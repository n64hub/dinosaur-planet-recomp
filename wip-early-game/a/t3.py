import sys,struct
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
ds={d.number:d for d in g.dlls(rom)}
d=ds[3]
base=d.rom_addr+d.text
gpv=d.rel-d.text
print(hex(gpv),hex(d.got[1]))
o=d.got[1]+0x2b0
t=struct.unpack('>36i',rom[base+o:base+o+144])
print([(i+2,hex(x+gpv)) for i,x in enumerate(t)])
o=d.got[1]+0x340
t=struct.unpack('>40i',rom[base+o:base+o+160])
print([(i,hex(x+gpv)) for i,x in enumerate(t)])
