exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
import collections
res=[]
for i in range(len(t42)-1):
    try: d,h=blk41(i)
    except Exception as e: continue
    if len(d)<0x50: continue
    nv=struct.unpack('>H',d[0x32:0x34])[0]; vo=struct.unpack('>I',d[4:8])[0]
    if vo!=0x50 or nv==0 or vo+nv*16>len(d): continue
    ys=collections.Counter(struct.unpack('>h',d[vo+k*16+2:vo+k*16+4])[0] for k in range(nv))
    sc=sum(ys[y] for y in (-635,-636,-680,-681,-759,-835,-716,-707))
    if sc>=4: res.append((sc,i,struct.unpack('>hh',d[0x40:0x44]),ys.most_common(4)))
for r in sorted(res,reverse=True)[:25]: print(r)
