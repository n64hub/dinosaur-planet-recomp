exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/rf.py').read())
import re
a,s=rfile(19)
d=rom[a:a+s]
strs=[(m.start(),m.group().decode('latin1')) for m in re.finditer(rb'[\x20-\x7e]{3,}',d[0x77a0:0x9438])]
a,s=rfile(55)
tb={}
for i in range(s//4):
    st=struct.unpack('>H',rom[a+i*4:a+i*4+2])[0];fl=rom[a+i*4+2];t=rom[a+i*4+3]
    if fl&0x20: tb.setdefault(t,[]).append(i)
print({k:[hex(x) for x in v] for k,v in sorted(tb.items()) if k<=70})
for o,t in strs[:150]: print(t)
