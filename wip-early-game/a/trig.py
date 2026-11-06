exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/lib.py').read())
import sys
N={0x13:'grp+',0x14:'grp-',0x18:'act(self)',0x1a:'grp+(map)',0x1b:'grp-(map)',0x1e:'act(map)',0x21:'bit?',0x22:'grpToggle',0x0d:'seq?',0x0a:'c0a',0x0f:'c0f',0x12:'c12'}
def cmds(raw):
    o=[]
    for i in range(8):
        c=raw[0x18+i*4:0x1c+i*4]
        if len(c)<4 or c[1]==0: continue
        o.append('%02x:%s(%02x%02x)'%(c[0],N.get(c[1],'c%02x'%c[1]),c[2],c[3]))
    return ' '.join(o)
if sys.argv[1]=='map':
    for m in map(int,sys.argv[2:]):
        for oid,x,y,z,uid,raw in mapobjs(m):
            n,d=objname(oid)
            if d==325: print(m,n,'%.0f %.0f %.0f uid %x +3=%02x'%(x,y,z,uid,raw[3]),cmds(raw),'| tail',raw[0x38:].hex())
else:
    for m in range(0,120):
      try: L=mapobjs(m)
      except Exception as e: continue
      for oid,x,y,z,uid,raw in L:
        n,d=objname(oid)
        if d!=325: continue
        for i in range(8):
            c=raw[0x18+i*4:0x1c+i*4]
            if len(c)==4 and c[1] in(0x1e,0x1a,0x1b) and c[3] in (7,8):
                print(m,mapname(m),n,'%.0f %.0f %.0f uid %x'%(x,y,z,uid),cmds(raw))
            if len(c)==4 and c[1]==0x18: print('ACTSELF',m,mapname(m),n,'%.0f %.0f %.0f uid %x'%(x,y,z,uid),cmds(raw))
