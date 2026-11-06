exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import zlib
a36,s36=rfile(36);a37,s37=rfile(37);a38,s38=rfile(38);a34,s34=rfile(34)
t37=[w(a37+i*4) for i in range(s37//4)]
b34=[struct.unpack('>H',rom[a34+i*2:a34+i*2+2])[0] for i in range(s34//2)]
b38=[struct.unpack('>H',rom[a38+i*2:a38+i*2+2])[0] for i in range(s38//2)]
def block(i):
    o=t37[i]&0xffffff; e=t37[i+1]&0xffffff
    return zlib.decompressobj(-15).decompress(rom[a36+o+5:a36+e])
def grid(m):
    t=[w(a32+(m*7+i)*4) for i in range(8)]
    W,H,ox,oz=struct.unpack('>HHhh',rom[a31+t[0]:a31+t[0]+8])
    out={}
    for z in range(H):
        for x in range(W):
            v=w(a31+t[1]+(z*W+x)*4)>>17
            if v<4000:
                mod=v>>6;sub=v&63
                out[(x-ox,z-oz)]=b38[b34[mod]+sub]
    return out
a46,s46=rfile(46);a47,s47=rfile(47)
t46=[w(a46+i*4) for i in range(s46//4)]
def blk(i):
    o=t46[i];e=t46[i+1]
    d=rom[a47+o:a47+e]
    return zlib.decompressobj(-15).decompress(d[13:]),d[:13]
a41,s41=rfile(41);a42,s42=rfile(42);a44,s44=rfile(44);a45,s45=rfile(45)
t42=[w(a42+i*4) for i in range(s42//4)]
t45=[w(a45+i*4) for i in range(s45//4)]
def grid2(m):
    t=[w(a32+(m*7+i)*4) for i in range(8)]
    W,H,ox,oz=struct.unpack('>HHhh',rom[a31+t[0]:a31+t[0]+8])
    out={}
    for z in range(H):
        for x in range(W):
            v=w(a31+t[1]+(z*W+x)*4)>>17
            if v<4000: out[(x-ox,z-oz)]=v
    return out
def blk41(i):
    d=rom[a41+t42[i]:a41+t42[i+1]]
    return zlib.decompressobj(-15).decompress(d[9:]),d[:9]
def hits(i): return rom[a44+t45[i]:a44+t45[i+1]]
def shapes(m):
    out=[]
    for (gx,gz),v in grid(m).items():
        d,h=blk41(v)
        P=lambda o:struct.unpack('>I',d[o:o+4])[0]
        vo,fo,so=P(4),P(8),P(0xC)
        nv,nf,ns=struct.unpack('>HHH',d[0x32:0x38])
        vs=[struct.unpack('>hhh',d[vo+i*16:vo+i*16+6]) for i in range(nv)]
        vs=[(a+gx*640,b,c+gz*640) for a,b,c in vs]
        rec=[d[so+i*0x18:so+i*0x18+0x18] for i in range(ns)]
        for i,r in enumerate(rec):
            fl=struct.unpack('>I',r[:4])[0]
            v0,f0=struct.unpack('>HH',r[4:8])
            f1=struct.unpack('>H',rec[i+1][6:8])[0] if i+1<ns else nf
            v1=struct.unpack('>H',rec[i+1][4:6])[0] if i+1<ns else nv
            tris=[]
            for f in range(f0,f1):
                q=d[fo+f*8:fo+f*8+8]
                tris.append((vs[v0+q[1]],vs[v0+q[2]],vs[v0+q[3]],q[0]))
            out.append(dict(cell=(gx,gz),block=v,i=i,fl=fl,sid=r[0x14],tex=r[0x12],rec=r,tris=tris,verts=vs[v0:v1]))
    return out
a43,s43=rfile(43)
b43=[struct.unpack('>H',rom[a43+i*2:a43+i*2+2])[0] for i in range(s43//2)]
def grid(m):
    t=[w(a32+(m*7+i)*4) for i in range(8)]
    W,H,ox,oz=struct.unpack('>HHhh',rom[a31+t[0]:a31+t[0]+8])
    out={}
    for z in range(H):
        for x in range(W):
            v=w(a31+t[1]+(z*W+x)*4)>>17
            if v<4000:
                out[(x-ox,z-oz)]=b43[v>>6]+(v&63)
    return out
