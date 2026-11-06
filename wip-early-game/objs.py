exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import sys,re
pat=re.compile(sys.argv[1],re.I)
seen=set()
for i in range(len(offs)-1):
    if offs[i+1]==0xffffffff or offs[i+1]<=offs[i]: continue
    n,d,_=defn(i)
    if pat.search(n):
        print(i,n,dllnum(d))
