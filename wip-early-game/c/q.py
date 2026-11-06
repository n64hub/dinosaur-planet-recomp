exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
import sys
S=shapes(7)
def hit(x,z):
    r=[]
    for s in S:
        for t in s['tris']:
            (x1,y1,z1),(x2,y2,z2),(x3,y3,z3)=t[0],t[1],t[2]
            den=(z2-z3)*(x1-x3)+(x3-x2)*(z1-z3)
            if den==0: continue
            a=((z2-z3)*(x-x3)+(x3-x2)*(z-z3))/den
            b=((z3-z1)*(x-x3)+(x1-x3)*(z-z3))/den
            c=1-a-b
            if a>=-0.001 and b>=-0.001 and c>=-0.001:
                nx=(y2-y1)*(z3-z1)-(z2-z1)*(y3-y1)
                ny=(z2-z1)*(x3-x1)-(x2-x1)*(z3-z1)
                nz=(x2-x1)*(y3-y1)-(y2-y1)*(x3-x1)
                r.append((round(a*y1+b*y2+c*y3,1),s['cell'],s['i'],hex(s['fl']),'up' if ny>0 else 'dn'))
    return sorted(r)
for p in sys.argv[1:]:
    x,z=map(float,p.split(','))
    print(p,hit(x,z))
