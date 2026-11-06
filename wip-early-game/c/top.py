exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
import sys
import zlib as _z
S=shapes(7)
X0,Z0,X1,Z1=[int(a) for a in sys.argv[1:5]]; sc=float(sys.argv[5]); out=sys.argv[6]; mode=sys.argv[7]
W=int((X1-X0)*sc);H=int((Z1-Z0)*sc)
import math
hb=[[None]*W for _ in range(H)]
def rast(t):
    (x1,y1,z1),(x2,y2,z2),(x3,y3,z3)=t[:3]
    ny=(z2-z1)*(x3-x1)-(x2-x1)*(z3-z1)
    if ny<=0: return
    den=(z2-z3)*(x1-x3)+(x3-x2)*(z1-z3)
    if den==0: return
    # slope filter
    ax,ay,az=x2-x1,y2-y1,z2-z1; bx,by,bz=x3-x1,y3-y1,z3-z1
    nx=ay*bz-az*by; nyy=az*bx-ax*bz; nz=ax*by-ay*bx
    L=math.sqrt(nx*nx+nyy*nyy+nz*nz) or 1
    if abs(nyy)/L<0.3: return
    xa=int((min(x1,x2,x3)-X0)*sc);xb=int((max(x1,x2,x3)-X0)*sc)+1
    za=int((min(z1,z2,z3)-Z0)*sc);zb=int((max(z1,z2,z3)-Z0)*sc)+1
    for pz in range(max(0,za),min(H,zb)):
        z=Z0+(pz+.5)/sc
        for px in range(max(0,xa),min(W,xb)):
            x=X0+(px+.5)/sc
            a=((z2-z3)*(x-x3)+(x3-x2)*(z-z3))/den
            b=((z3-z1)*(x-x3)+(x1-x3)*(z-z3))/den
            c=1-a-b
            if a>=0 and b>=0 and c>=0:
                y=a*y1+b*y2+c*y3
                o=hb[pz][px]
                if o is None or (mode=='hi' and y>o) or (mode=='lo' and y<o): hb[pz][px]=y
for s in S:
    if s['fl']&0x200: continue
    if (s['fl']&0xe) ==0xe : continue
    for t in s['tris']: rast(t)
px=[[(0,0,0)]*W for _ in range(H)]
ymin,ymax=-860,-480
for j in range(H):
    for i in range(W):
        y=hb[j][i]
        if y is None: continue
        f=max(0,min(1,(y-ymin)/(ymax-ymin)))
        px[j][i]=(int(255*f),int(255*(1-abs(2*f-1))),int(255*(1-f)))
step=int(sys.argv[8]) if len(sys.argv)>8 else 500
for gx in range((X0//step+1)*step,X1,step):
    i=int((gx-X0)*sc)
    for j in range(0,H,2): px[j][i]=(255,255,255)
for gz in range((Z0//step+1)*step,Z1,step):
    j=int((gz-Z0)*sc)
    for i in range(0,W,2): px[j][i]=(255,255,255)
def dot(x,z,c,r=2):
    i=int((x-X0)*sc);j=int((z-Z0)*sc)
    for a in range(-r,r+1):
        for b in range(-r,r+1):
            if 0<=i+a<W and 0<=j+b<H: px[j+b][i+a]=c
for oid,x,y,z,uid,raw in mapobjs(7):
    n,dl=objname(oid)
    if dl in(324,347,348,354): continue
    dot(x,z,(255,255,0) if dl==325 else (255,0,255))
raw=b''.join(bytes([0])+bytes(v for p in r for v in p) for r in px)
import struct as st
def ch(t,d): return st.pack('>I',len(d))+t+d+st.pack('>I',_z.crc32(t+d))
open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/'+out,'wb').write(bytes([0x89])+b'PNG'+bytes([13,10,26,10])+ch(b'IHDR',st.pack('>IIBBBBB',W,H,8,2,0,0,0))+ch(b'IDAT',_z.compress(raw))+ch(b'IEND',b''))
print('ok',W,H)
