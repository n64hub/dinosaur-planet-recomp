exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
g=grid2(7);v=g[(3,2)];d,h=blk41(v)
P=lambda o:struct.unpack('>I',d[o:o+4])[0]
vo,fo,so=P(4),P(8),P(0xC)
nv=struct.unpack('>H',d[0x32:0x34])[0];nf=struct.unpack('>H',d[0x34:0x36])[0];ns=struct.unpack('>H',d[0x36:0x38])[0]
print(nv,nf,ns)
for i in range(4): print(d[vo+i*16:vo+i*16+16].hex(),struct.unpack('>hhh',d[vo+i*16:vo+i*16+6]))
vs=[struct.unpack('>hhh',d[vo+i*16:vo+i*16+6]) for i in range(nv)]
print(min(a[0] for a in vs),max(a[0] for a in vs),min(a[1] for a in vs),max(a[1] for a in vs),min(a[2] for a in vs),max(a[2] for a in vs))
for i in range(6): print(d[fo+i*8:fo+i*8+8].hex())
for i in range(ns): print(i,d[so+i*0x18:so+i*0x18+0x18].hex())
print(d[0x38:0x50].hex())
print(hex(P(0)),d[P(0):P(0)+32].hex(),hex(P(0x10)),d[P(0x10):P(0x10)+32].hex())
