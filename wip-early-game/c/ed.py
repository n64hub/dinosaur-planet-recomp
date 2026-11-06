p='/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/top.py'
s=open(p).read()
s=s.replace("""try:
    from PIL import Image,ImageDraw
except Exception as e:
    print('nopil',e); sys.exit()
""","import zlib as _z\n")
i=s.index("img=Image.new")
s=s[:i]+'''px=[[(0,0,0)]*W for _ in range(H)]
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
raw=b''.join(b'\0'+bytes(v for p in r for v in p) for r in px)
import struct as st
def ch(t,d): return st.pack('>I',len(d))+t+d+st.pack('>I',_z.crc32(t+d))
open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/'+out,'wb').write(b'\x89PNG\r\n\x1a\n'+ch(b'IHDR',st.pack('>IIBBBBB',W,H,8,2,0,0,0))+ch(b'IDAT',_z.compress(raw))+ch(b'IEND',b''))
print('ok',W,H)
'''
open(p,'w').write(s)
