import sys,struct
B='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000'
sys.path.insert(0,B+'/tools')
import gen_syms as g
rom=open(B+'/roms/dinoplanet.z64','rb').read()
vals=[int(x,16) for x in sys.argv[1:]]
for d in g.dlls(rom):
    a=d.rom_addr; s=d.size
    start=d.text_rom+d.text_size
    blob=rom[start:a+s]
    for v in vals:
        pat=struct.pack('>H',v); p=0
        while True:
            p=blob.find(pat,p)
            if p<0: break
            if p%2==0:
                print('dll',d.number,'val %x'%v,'dataoff %x'%p,blob[max(0,p-8):p+10].hex())
            p+=1
# main seg
main=rom[0x1000:0x1000+0x9d000]
for v in vals:
    pat=struct.pack('>H',v);p=0x89350
    while True:
        p=main.find(pat,p)
        if p<0:break
        if p%2==0: print('main val %x at %x'%(v,0x80000400+p),main[max(0,p-8):p+10].hex())
        p+=1
