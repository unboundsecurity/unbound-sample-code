using System.Runtime.InteropServices;

namespace unbound.cryptoki {

public class CK_INFO
{
  public CK_VERSION cryptokiVersion;
  public string manufacturerID;
  public uint flags;
  public string libraryDescription;
  public CK_VERSION libraryVersion;

  internal CK_INFO(Native native)
  {
    cryptokiVersion = native.cryptokiVersion;
    flags = native.flags;
    libraryVersion = native.libraryVersion;
    libraryDescription = Library.Utf8ToString(native.libraryDescription);
    manufacturerID = Library.Utf8ToString(native.manufacturerID);      
  }

  [StructLayout(LayoutKind.Sequential, Pack = 1)] internal struct Native
  {
    public CK_VERSION cryptokiVersion;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]      
    public byte[] manufacturerID;
    public uint flags;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]      
    public byte[] libraryDescription;
    public CK_VERSION libraryVersion;
  }

}

} //namespace unbound.cryptoki
