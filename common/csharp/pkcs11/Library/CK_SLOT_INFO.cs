using System.Runtime.InteropServices;

namespace unbound.cryptoki 
{

public class CK_SLOT_INFO
{
  public string slotDescription;
  public string manufacturerID;
  public uint flags;
  public CK_VERSION hardwareVersion;
  public CK_VERSION firmwareVersion;

  internal CK_SLOT_INFO(Native native)
  {
    flags = native.flags;
    hardwareVersion = native.hardwareVersion;
    firmwareVersion = native.firmwareVersion;
    slotDescription = Library.Utf8ToString(native.slotDescription);
    manufacturerID = Library.Utf8ToString(native.manufacturerID);
  }

  [StructLayout(LayoutKind.Sequential, Pack = 1)] internal struct Native
  {
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]      
    public byte[] slotDescription;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]      
    public byte[] manufacturerID;

    public uint flags;
    public CK_VERSION hardwareVersion;
    public CK_VERSION firmwareVersion;
  }
}

} // namespace unbound.cryptoki
