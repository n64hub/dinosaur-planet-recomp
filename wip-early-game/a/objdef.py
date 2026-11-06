exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import sys
for i in range(len(offs)-1):
    n,dl,d=defn(i)
    if sys.argv[1].lower() in n.lower():
        print(i,n,dl,len(d)); print(d[:0x90].hex())
