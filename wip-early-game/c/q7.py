exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
S=shapes(7)
for s in S:
    if s['sid']:
        vs=s['verts']
        print(s['cell'],s['i'],hex(s['fl']),'sid',hex(s['sid']),'n',len(s['tris']),'x',min(p[0] for p in vs),max(p[0] for p in vs),'y',min(p[1] for p in vs),max(p[1] for p in vs),'z',min(p[2] for p in vs),max(p[2] for p in vs))
print('hit lines per block')
for c,v in sorted(grid(7).items()):
    H=hits(v)
    print(c,v,len(H),H[:40].hex())
