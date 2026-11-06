p='/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/top.py'
s=open(p,'rb').read()
i=s.index(b"raw=b''.join(")
j=s.index(b"import struct as st")
s=s[:i]+b"raw=b''.join(bytes([0])+bytes(v for p in r for v in p) for r in px)\n"+s[j:]
i=s.index(b"open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/'+out")
j=s.index(b"print('ok'",i)
s=s[:i]+b"open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/'+out,'wb').write(bytes([0x89])+b'PNG'+bytes([13,10,26,10])+ch(b'IHDR',st.pack('>IIBBBBB',W,H,8,2,0,0,0))+ch(b'IDAT',_z.compress(raw))+ch(b'IEND',b''))\n"+s[j:]
open(p,'wb').write(s)
