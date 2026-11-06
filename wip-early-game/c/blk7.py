exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
g=grid2(7); print(g)
v=g[(3,2)]
d,h=blk41(v); print(v,h.hex(),len(d)); print(d[:0x60].hex())
for i in range(0,0x48,4): print(hex(i),struct.unpack('>I',d[i:i+4])[0],struct.unpack('>hh',d[i:i+4]))
H=hits(v); print('hits',len(H),H[:60].hex())
for vv in sorted(set(g.values())): print(vv,len(hits(vv)),end='; ')
