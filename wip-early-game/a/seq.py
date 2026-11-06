exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/rf.py').read())
import collections
a13,s13=rfile(13);a14,s14=rfile(14);a15,s15=rfile(15)
tab=struct.unpack('>%dH'%(s15//2),rom[a15:a15+s15//2*2])
n=s14//8
print('seqs',n,'tab',len(tab),tab[:8])
def ev(i):
    w0,w1=struct.unpack('>II',rom[a14+i*8:a14+i*8+8])
    size=w0>>16;cnt=w0&0xffff
    if size==0 or w1>=s13: return []
    return [struct.unpack('>I',rom[a13+w1+j*4:a13+w1+j*4+4])[0] for j in range(min(cnt,size//4))]
kinds=collections.Counter()
hits=[]
for i in range(n):
    for e in ev(i):
        k=e&0x3f;t=(e>>6)&0x3ff;p=(e>>16)&0xffff
        kinds[k]+=1
        if k==6 and t in (0x1b,0x1c,0x1d): hits.append((i,hex(t),hex(p)))
print(kinds)
print(len(hits));
import bisect
def owner(i):
    j=bisect.bisect_right(tab,i)-1
    return j,i-tab[j]
for h in hits:
    print(h,owner(h[0]))
