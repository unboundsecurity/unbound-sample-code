using System;

namespace unbound.cryptoki {
  
public class DYCK_DERIVE_BIP_PARAMS : CK_MECHANISM_PARAM
{
  public bool hardened;
  public uint ulChildNumber;  
  
  public IntPtr ToBin(out int size)
  {
    CK_MECHANISM_PARAM_Buffer buf = new CK_MECHANISM_PARAM_Buffer(5, 0);
    buf.Pack(hardened);
    buf.Pack(ulChildNumber);
    size = buf.Size;
    return buf.Ptr;
  }
  
}

}