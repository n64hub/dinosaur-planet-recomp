exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/rf.py').read())
import re,sys
a,s=rfile(19)
d=rom[a:a+s]
pat=re.compile(sys.argv[1],re.I)
for m in re.finditer(rb'[\x20-\x7e]{4,}',d):
    t=m.group().decode('latin1')
    if pat.search(t): print(hex(m.start()),t[:150])
