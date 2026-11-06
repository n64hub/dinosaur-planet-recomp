exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/rf.py').read())
import sys
a,s=rfile(55)
for x in sys.argv[1:]:
    i=int(x,16);st=struct.unpack('>H',rom[a+i*4:a+i*4+2])[0];fl=rom[a+i*4+2];t=rom[a+i*4+3]
    print(x,'start',hex(st),'bank',fl>>6,'task' if fl&0x20 else '', 'len',(fl&31)+1,'t',t)
