exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
S=shapes(7)
for s in S:
    if s['cell'] in ((3,2),(2,3),(1,2),(4,1),(4,2),(1,3)):
        vs=s['verts']
        if not vs: continue
        if min(a[1] for a in vs)<20:
            print(s['cell'],s['i'],hex(s['fl']),'x',min(a[0] for a in vs),max(a[0] for a in vs),'y',min(a[1] for a in vs),max(a[1] for a in vs),'z',min(a[2] for a in vs),max(a[2] for a in vs),len(s['tris']),s['rec'][0x10:].hex())
