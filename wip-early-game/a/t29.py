import sys,struct
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
ds={d.number:d for d in g.dlls(rom)}
d=ds[29]
print(hex(d.text),hex(d.data),hex(d.rel),hex(d.size),[hex(x) for x in d.got[:30]])
base=d.rom_addr+d.text
def tab(o):
    return struct.unpack('>120H',rom[base+o:base+o+240])
a=tab(0x1F90); b=tab(0x1F90+0xF0)
print('act bits',[hex(x) for x in a]); print('grp bits',[hex(x) for x in b])
print('mapremap', rom[base+0x2180:base+0x2180+0].hex())
