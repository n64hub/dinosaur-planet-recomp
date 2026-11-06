import sys,struct
sys.path.insert(0,'/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000/tools')
import importlib.util
ROMP='/mnt/c/Users/Benla/source/bot-worktrees/dinoplanet-early-17939592916570000/roms/dinoplanet.z64'
rom=open(ROMP,'rb').read()
FT=0xA4970;FB=0xA4AA0
def w(o): return struct.unpack('>I',rom[o:o+4])[0]
n=w(FT)
def rfile(i):
    s=w(FT+4+i*4);e=w(FT+8+i*4)
    return FB+s,e-s
