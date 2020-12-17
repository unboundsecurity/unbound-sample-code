using System.Runtime.InteropServices;

namespace unbound.cryptoki {

  [StructLayout(LayoutKind.Sequential, Pack = 1)]  
  public struct CK_SESSION_INFO
  {
    public uint slotID;
    public uint state;
    public uint flags;
    public uint ulDeviceError;
  }

} // namespace unbound.cryptoki
