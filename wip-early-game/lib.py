exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/rf.py').read())
import bisect,collections
a65,s65=rfile(65);a66,s66=rfile(66);a67,s67=rfile(67)
offs=[w(a65+i*4) for i in range(s65//4)]
idx=[struct.unpack('>h',rom[a67+2*i:a67+2*i+2])[0] for i in range(s67//2)]
def defn(i):
    d=rom[a66+offs[i]:a66+offs[i+1]]
    name=d[0x5f:0x5f+16].split(b'\0')[0].decode('latin1')
    dll=struct.unpack('>H',d[0x58:0x5a])[0]
    return name,dll,d
def resolve(oid):
    # 0x8000 flag? ids go through file 67 unless flagged
    i=idx[oid] if oid<len(idx) else -1
    return i
def dllnum(v):
    return (v&0x7fff)+209 if v&0x8000 else v
a31,s31=rfile(31);a32,s32=rfile(32);a33,s33=rfile(33)
def mapname(m): return rom[a33+m*32:a33+m*32+28].split(b'\0')[0].decode('latin1')
def mapobjs(m):
    t=[w(a32+(m*7+i)*4) for i in range(8)]
    p=a31+t[4];e=a31+t[5];out=[]
    while p<e:
        oid=struct.unpack('>H',rom[p:p+2])[0];sz=rom[p+2]*4
        if sz==0: break
        x,y,z=struct.unpack('>fff',rom[p+8:p+20]);uid=w(p+20)
        out.append((oid,x,y,z,uid,rom[p:p+sz]));p+=sz
    return out
def objname(oid):
    i=resolve(oid)
    if i<0 or i>=len(offs)-1: return '?%x'%oid,0
    n,d,_=defn(i); return n,dllnum(d)
