exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import sys,re
A={1:'sound?',3:'seq?',4:'obj4',5:'5',6:'6',8:'cfg8',0xa:'A',0xb:'B',0xc:'C',0xd:'D',0xf:'F',0x10:'10',0x11:'11',0x12:'BIT',0x13:'GRP_ON',0x14:'GRP_OFF',0x15:'15',0x16:'16',0x18:'18',0x19:'19',0x1f:'SAVEPT?',0x20:'20',0x21:'BITXOR',0x22:'GRP_TOG',0x23:'23'}
m=int(sys.argv[1])
for oid,x,y,z,uid,raw in mapobjs(m):
    n,d=objname(oid)
    if d!=325: continue
    p=raw[0x18:]
    acts=[]
    for i in range(8):
        f,a,p1,p2=p[i*4:i*4+4]
        if a==0: continue
        v=(p1<<8)|p2
        s=A.get(a,hex(a))
        if a==0x12: s+=' %x=%s'%(v&0x3fff,['0','1','tog','?'][v>>14])
        elif a==0x21: s+=' %x^=%d'%(v&0x1fff,1<<(v>>13))
        elif a in(0x13,0x14,0x22): s+=' %d'%v
        else: s+=' %04x'%v
        acts.append('[%02x]%s'%(f,s))
    print(n,'%.0f %.0f %.0f'%(x,y,z),'uid %x'%uid,'grp',raw[6],'tail',p[32:].hex(),' '.join(acts))
