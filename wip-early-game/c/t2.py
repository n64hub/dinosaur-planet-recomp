exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/rf.py').read())
import re
a,s=rfile(19)
d=rom[a:a+s]
strs=[(m.start(),m.group().decode('latin1')) for m in re.finditer(rb'[\x20-\x7e]{4,}',d)]
pat=re.compile(r'bWELLb|bHOLE|bPITb|TUNNEL|PLANT|ROCKET|PURPLE|WHITE MUSH|LIFT|CAVE|SWAPSTONE',re.I)
for o,t in strs:
    if pat.search(t): print(hex(o),t[:120])
