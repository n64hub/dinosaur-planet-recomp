exec(open('C:/Users/Alek/AppData/Local/Temp/hs-scratch/17939592916570000/t/lib.py').read())
import sys,re
pat=re.compile(sys.argv[1],re.I)
for i in range(len(offs)-1):
    try: n,d,raw=defn(i)
    except Exception: continue
    if pat.search(n): print(i,n,dllnum(d),'models(+5D)=%d'%raw[0x5d],'len',len(raw),raw[:0x60].hex())
