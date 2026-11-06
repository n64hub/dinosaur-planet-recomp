exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
import sys
S=shapes(7)
X0,X1,Z0,Z1,Y0,Y1=[float(a) for a in sys.argv[1:7]]
for s in S:
    tr=[t for t in s['tris'] if any(X0<=p[0]<=X1 and Z0<=p[2]<=Z1 and Y0<=p[1]<=Y1 for p in t[:3])]
    if not tr: continue
    vs=[p for t in tr for p in t[:3]]
    print(s['cell'],s['i'],hex(s['fl']),'tex',s['rec'][0x12],'sid',hex(s['rec'][0x14]),'n',len(tr),'x',min(p[0] for p in vs),max(p[0] for p in vs),'y',min(p[1] for p in vs),max(p[1] for p in vs),'z',min(p[2] for p in vs),max(p[2] for p in vs))
