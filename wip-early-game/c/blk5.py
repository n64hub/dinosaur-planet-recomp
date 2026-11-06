exec(open('/mnt/c/Users/Benla/AppData/Local/Temp/hs-scratch/17939592916570000/c/blklib.py').read())
for T,D in ((42,41),(46,47),(53,52),(40,39),(45,44)):
    aT,sT=rfile(T);aD,sD=rfile(D)
    t=[w(aT+i*4) for i in range(sT//4)]
    print(T,D,len(t),[hex(x) for x in t[:4]])
    for bi in (167,):
        o=t[bi]&0xffffff;e=t[bi+1]&0xffffff
        d=rom[aD+o:aD+e]
        print(' raw',len(d),d[:32].hex())
        for sk in range(0,16):
            try:
                r=zlib.decompressobj(-15).decompress(d[sk:]); 
                if len(r)>32:
                    print(' deflate at',sk,len(r));print(r[:96].hex());break
            except Exception as ex: pass
