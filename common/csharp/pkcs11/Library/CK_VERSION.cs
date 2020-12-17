using System.Runtime.InteropServices;

namespace unbound.cryptoki {

[StructLayout(LayoutKind.Sequential, Pack = 1)]  public struct CK_VERSION
{
  public byte major;
  public byte minor;
}



} //namespace unbound.cryptoki