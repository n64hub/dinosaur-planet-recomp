import sys,re
c={};w=set()
for f in sys.argv[2:]:
    for l in open(f,errors='replace'):
        m=re.match(r'cell (-?\d+) (-?\d+) (-?\d+)',l)
        if m: c[(int(m[1]),int(m[2]))]=int(m[3])
        m=re.match(r'wall (-?\d+) (-?\d+)',l)
        if m: w.add((int(m[1]),int(m[2])))
cs=float(sys.argv[1])
print(len(c),'cells',len(w),'walls')
xs=[k[0] for k in c]+[k[0] for k in w]; zs=[k[1] for k in c]+[k[1] for k in w]
ch=lambda y:'0123456789abcdefghijklmnopqrstuvwxyz'[max(0,min(35,int((y+2000)/25)))]
for z in range(min(zs),max(zs)+1):
    print('%6d '%(z*cs)+''.join(ch(c[(x,z)]) if (x,z) in c else ('#' if (x,z) in w else ' ') for x in range(min(xs),max(xs)+1)))
print('x from',min(xs)*cs,'to',max(xs)*cs,' y code: char*25-2000')
