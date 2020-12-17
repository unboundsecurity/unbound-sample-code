using System.Runtime.InteropServices;


namespace unbound.cryptoki {

public class CK_TOKEN_INFO
{
  public string label;
  public string manufacturerID;
  public string model;
  public string serialNumber;
  public uint flags;
  public int ulMaxSessionCount;
  public int ulSessionCount;
  public int ulMaxRwSessionCount;
  public int ulRwSessionCount;
  public int ulMaxPinLen;
  public int ulMinPinLen;
  public int ulTotalPublicMemory;
  public int ulFreePublicMemory;
  public int ulTotalPrivateMemory;
  public int ulFreePrivateMemory;
  public CK_VERSION hardwareVersion;
  public CK_VERSION firmwareVersion;
  public string utcTime;

  internal CK_TOKEN_INFO(Native native)
  {
    firmwareVersion = native.firmwareVersion;
    flags = native.flags;
    hardwareVersion = native.hardwareVersion;
    label = Library.Utf8ToString(native.label);
    manufacturerID = Library.Utf8ToString(native.manufacturerID);
    model = Library.Utf8ToString(native.model);
    serialNumber = Library.Utf8ToString(native.serialNumber);
    ulFreePrivateMemory = native.ulFreePrivateMemory;
    ulFreePublicMemory = native.ulFreePublicMemory;
    ulMaxPinLen = native.ulMaxPinLen;
    ulMaxRwSessionCount = native.ulMaxRwSessionCount;
    ulMaxSessionCount = native.ulMaxSessionCount;
    ulMinPinLen = native.ulMinPinLen;
    ulRwSessionCount = native.ulRwSessionCount;
    ulSessionCount = native.ulSessionCount;
    ulTotalPrivateMemory = native.ulTotalPrivateMemory;
    ulTotalPublicMemory = native.ulTotalPublicMemory;
    utcTime = Library.Utf8ToString(native.utcTime);
  }

  [StructLayout(LayoutKind.Sequential, Pack = 1)] internal struct Native
  {
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]      
    public byte[] label;
    
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]      
    public byte[] manufacturerID;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]         
    public byte[] model;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]          
    public byte[] serialNumber;

    public uint flags;
    public int ulMaxSessionCount;
    public int ulSessionCount;
    public int ulMaxRwSessionCount;
    public int ulRwSessionCount;
    public int ulMaxPinLen;
    public int ulMinPinLen;
    public int ulTotalPublicMemory;
    public int ulFreePublicMemory;
    public int ulTotalPrivateMemory;
    public int ulFreePrivateMemory;
    public CK_VERSION hardwareVersion;
    public CK_VERSION firmwareVersion;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]          
    public byte[] utcTime;
  }

}

} // namespace unbound.cryptoki