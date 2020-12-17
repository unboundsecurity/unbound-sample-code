using System.Runtime.InteropServices;

namespace unbound.cryptoki {

[StructLayout(LayoutKind.Sequential, Pack = 1)] public struct CK_MECHANISM_INFO
{
  public int ulMinKeySize;
  public int ulMaxKeySize;
  public uint flags;
};

} //namespace unbound.cryptoki