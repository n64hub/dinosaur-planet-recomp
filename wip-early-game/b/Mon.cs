using System; using System.IO; using System.Collections.Generic; using System.Diagnostics; using System.Runtime.InteropServices; using System.Threading; using System.Text;
public static class Mon {
 [DllImport("kernel32.dll")] static extern IntPtr OpenProcess(uint a,bool b,uint p);
 [DllImport("kernel32.dll")] static extern bool ReadProcessMemory(IntPtr h,IntPtr a,byte[] b,IntPtr n,out IntPtr r);
 [DllImport("kernel32.dll")] static extern bool WriteProcessMemory(IntPtr h,IntPtr a,byte[] b,IntPtr n,out IntPtr r);
 [DllImport("kernel32.dll")] static extern IntPtr VirtualQueryEx(IntPtr h,IntPtr a,out MBI m,IntPtr l);
 [StructLayout(LayoutKind.Sequential)] struct MBI{public IntPtr Base,Alloc;public uint AP;public uint pad0;public IntPtr Size;public uint State,Prot,Type,pad1;}
 static byte[] M=new byte[0x800000]; static IntPtr H; static long RB;
 static uint W(uint a){a&=0x7FFFFF;if(a+4>0x800000)return 0;return BitConverter.ToUInt32(M,(int)(a&~3u));}
 static byte B(uint a){a&=0x7FFFFF;return M[(int)(a^3)];}
 static ushort HW(uint a){a&=0x7FFFFF;return BitConverter.ToUInt16(M,(int)(a^2));}
 static float F(uint a){return BitConverter.ToSingle(BitConverter.GetBytes(W(a)),0);}
 static bool Good(uint w){return (w>>24)==0x80;}
 static bool Find(){
  long a=0x10000; MBI m; var sz=(IntPtr)Marshal.SizeOf(typeof(MBI)); byte[] t=new byte[0x1000];IntPtr r;
  while(a<0x7FFFFFFFFFFFL){
   if(VirtualQueryEx(H,(IntPtr)a,out m,sz)==IntPtr.Zero)break;
   long s=m.Base.ToInt64(),e=s+m.Size.ToInt64();
   if(m.State==0x1000&&m.Size.ToInt64()>=0x400000){
    for(long b=s;b+0xB0000<=e;b+=0x10000){
     if(ReadProcessMemory(H,(IntPtr)(b+0x8C000),t,(IntPtr)0x1000,out r)){
      uint w1=BitConverter.ToUInt32(t,0x994),w2=BitConverter.ToUInt32(t,0x9F4),w3=BitConverter.ToUInt32(t,0x97C);
      if(Good(w1)&&Good(w2)&&Good(w3)&&w1!=w2&&w2!=w3){RB=b;return true;}
     }
    }
   }
   a=e;
  }
  return false;
 }
 static uint Bank(uint save,int bank){return save+(bank==0?0x17ACu:bank==1?0x13D4u:bank==2?0x12D4u:0x15D4u);}
 static int GetBit(uint tab,uint save,int i){
  uint e=tab+(uint)i*4; ushort st=HW(e); byte fl=B(e+2); uint bs=Bank(save,fl>>6); int len=(fl&31)+1; int v=0;
  for(int k=0;k<len;k++){int p=st+k; if((B(bs+(uint)(p>>3))&(1<<(p&7)))!=0)v|=1<<k;}
  return v;
 }
 static void PokeB(uint a,byte val){a&=0x7FFFFF;var b=new byte[]{val};IntPtr r;M[a^3]=val;WriteProcessMemory(H,(IntPtr)(RB+(a^3)),b,(IntPtr)1,out r);}
 static void PokeW(uint a,uint val){a&=0x7FFFFF;var b=BitConverter.GetBytes(val);IntPtr r;WriteProcessMemory(H,(IntPtr)(RB+(a&~3u)),b,(IntPtr)4,out r);}
 static void SetBit(uint tab,uint save,int i,int val){
  uint e=tab+(uint)i*4; ushort st=HW(e); byte fl=B(e+2); uint bs=Bank(save,fl>>6); int len=(fl&31)+1;
  for(int k=0;k<len;k++){int p=st+k; uint ad=bs+(uint)(p>>3); byte c=B(ad); if((val&(1<<k))!=0)c|=(byte)(1<<(p&7)); else c&=(byte)~(1<<(p&7)); PokeB(ad,c);}
 }
 // a poke file: "<seconds> bit <hex> <value>", "<seconds> w <hex address> <hex value>", "<seconds> b <hex address> <hex value>", "<seconds> dump <hex address> <hex length>"
 public static void Go(string pidf,string uids,string log,string pokes,int ms){
  var o=new StreamWriter(log,false); o.AutoFlush=true;
  Stopwatch w=Stopwatch.StartNew(); int pid=0;
  while(w.Elapsed.TotalSeconds<900){ if(File.Exists(pidf)){try{pid=int.Parse(File.ReadAllText(pidf).Trim());break;}catch{}} Thread.Sleep(100);}
  if(pid==0){o.WriteLine("no pid");return;}
  w.Restart(); Process pr=Process.GetProcessById(pid); H=OpenProcess(0x38|0x400,false,(uint)pid); o.WriteLine("pid "+pid+" h "+H);
  var ul=new HashSet<uint>(); foreach(var s in uids.Split(','))if(s.Length>0)ul.Add(Convert.ToUInt32(s,16));
  var pk=new List<string[]>(); if(pokes!=""&&File.Exists(pokes))foreach(var l in File.ReadAllLines(pokes)){var p=l.Trim().Split(' ');if(p.Length>=3)pk.Add(p);}
  double hb=0; int[] prev=null; var last=new Dictionary<uint,string>(); IntPtr rd; string lastp="";
  while(!pr.HasExited){
   if(RB==0){ if(!Find()){Thread.Sleep(300);continue;} o.WriteLine("rdram at "+RB.ToString("X")+" t="+w.Elapsed.TotalSeconds.ToString("F1")); }
   { byte[] c=new byte[0x10000]; int okc=0; for(int off=0;off<0x800000;off+=0x10000){ if(ReadProcessMemory(H,(IntPtr)(RB+off),c,(IntPtr)0x10000,out rd)){Buffer.BlockCopy(c,0,M,off,0x10000);okc++;} } rd=(IntPtr)okc; }
   double t=w.Elapsed.TotalSeconds; string T=t.ToString("F1"); if(t>=hb){hb=t+5;o.WriteLine(T+" hb save "+W(0x800AE2A0).ToString("X")+" tab "+W(0x800AE2A4).ToString("X")+" nb "+HW(0x800AE2A8)+" read "+rd+" c0 "+((short)HW(0x800B28B0))+" c1 "+((short)HW(0x800B28B2))+" c2 "+((short)HW(0x800B28B4))+" p0 "+W(0x800B2938).ToString("X"));}
   uint save=W(0x800AE2A0),tab=W(0x800AE2A4); int nb=2336;
   if(Good(save)&&Good(tab)&&nb>0){
    var cur=new int[nb]; for(int i=0;i<nb;i++)cur[i]=GetBit(tab,save,i);
    if(prev!=null&&prev.Length==nb){var sb=new StringBuilder();for(int i=0;i<nb;i++)if(cur[i]!=prev[i])sb.Append(" "+i.ToString("x")+":"+prev[i]+">"+cur[i]); if(sb.Length>0&&sb.Length<1500)o.WriteLine(T+" bits"+sb); else if(sb.Length>=1500)o.WriteLine(T+" bits many "+sb.Length);}
    prev=cur;
    for(int i=pk.Count-1;i>=0;i--){var p=pk[i]; if(t>=double.Parse(p[0])){
      if(p[1]=="bit")SetBit(tab,save,Convert.ToInt32(p[2],16),int.Parse(p[3]));
      else if(p[1]=="w")PokeW(Convert.ToUInt32(p[2],16),Convert.ToUInt32(p[3],16));
      else if(p[1]=="b")PokeB(Convert.ToUInt32(p[2],16),Convert.ToByte(p[3],16));
      else if(p[1]=="save")File.WriteAllBytes(p[2],M);
      else if(p[1]=="dump"){uint ad=Convert.ToUInt32(p[2],16);uint n=Convert.ToUInt32(p[3],16);var sb=new StringBuilder();for(uint k=0;k<n;k+=4)sb.Append(W(ad+k).ToString("x8")+" ");o.WriteLine(T+" dump "+p[2]+" "+sb);}
      o.WriteLine(T+" poke "+string.Join(" ",p)); pk.RemoveAt(i);}}
   }
   short s0=(short)HW(0x800B28B0),s1=(short)HW(0x800B28B2),s2=(short)HW(0x800B28B4);
   uint pl=(s1>s0)?W(0x800B2938+(uint)s0*4):0, sk=(s2>s1)?W(0x800B2938+(uint)s1*4):0;
   string ps="player "+pl.ToString("X")+" side "+sk.ToString("X")+(Good(sk)?" def "+HW(sk+0x46).ToString("x"):"");
   if(ps!=lastp){lastp=ps;o.WriteLine(T+" "+ps+" rec "+(Good(pl)?W(pl+0x4C).ToString("X"):""));}
   if(ul.Count>0){
    var rec=new Dictionary<uint,uint>();
    for(uint a=0x100000;a+0x20<0x7FF000;a+=4){uint u=BitConverter.ToUInt32(M,(int)a); if(u>=0x100&&u<0x60000&&ul.Contains(u)&&a>=0x14){uint ra=a-0x14; uint sz=(uint)B(ra+2); uint xb=W(ra+8); if(sz>=6&&sz<=0x30&&(xb>>24)>=0x40&&(xb>>24)<0xC8)rec[0x80000000|ra]=u;}}
    var byr=rec;
    var seen=new HashSet<uint>();
    for(uint a=0x100000;a<0x7FFFF0;a+=4){uint v=BitConverter.ToUInt32(M,(int)a); if((v>>24)==0x80&&byr.ContainsKey(v)&&a>=0x4C){uint ob=0x80000000|(a-0x4C); uint d=W(ob+0x50),st=W(ob+0xB8),vt=W(ob+0x68); if(Good(d)&&Good(vt)){uint u=byr[v];seen.Add(u);
       var sb=new StringBuilder(); if(Good(st))for(uint k=0;k<4;k++)sb.Append(B(st+k).ToString("x2"));
       string s=ob.ToString("X")+" st "+sb+" af "+B(ob+0xAF).ToString("x2")+" ad "+B(ob+0xAD)+" seq "+((short)HW(ob+0xB4))+" f6 "+HW(ob+6).ToString("x")+" fb0 "+HW(ob+0xB0).ToString("x")+" a0 "+HW(ob+0xA0).ToString("x")+" s136 "+(Good(st)?B(st+0x136).ToString("x"):"")+" msg "+(Good(W(ob+0xCC))?W(W(ob+0xCC)).ToString():"");
       string pos=" @"+F(ob+0x18).ToString("F0")+","+F(ob+0x1C).ToString("F0")+","+F(ob+0x20).ToString("F0");
       string old; if(!last.TryGetValue(u,out old)||old!=s){last[u]=s;o.WriteLine(T+" obj "+u.ToString("x")+" "+s+pos);}
    }}}
    foreach(var u in new List<uint>(last.Keys))if(!seen.Contains(u)){o.WriteLine(T+" obj "+u.ToString("x")+" gone");last.Remove(u);}
   }
   Thread.Sleep(ms);
  }
  o.WriteLine("exit "+w.Elapsed.TotalSeconds.ToString("F1")); o.Close();
 }
}
