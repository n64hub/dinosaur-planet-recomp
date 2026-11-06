exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import sys
want=set(int(x) for x in sys.argv[1:])
for i in range(len(offs)-1):
    if offs[i+1]==0xffffffff or offs[i+1]<=offs[i]: continue
    n,d,_=defn(i)
    if dllnum(d) in want: print(dllnum(d),n,end='; ')
print()
for m in range(98):
    try: o=mapobjs(m)
    except: continue
    c=collections.Counter(objname(x[0])[1] for x in o)
    h=[(k,c[k]) for k in want if c[k]]
    if h: print(m,mapname(m),h)
