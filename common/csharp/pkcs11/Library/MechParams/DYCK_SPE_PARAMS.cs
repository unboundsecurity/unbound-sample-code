using System;

namespace unbound.cryptoki {

public class DYCK_SPE_PARAMS : CK_MECHANISM_PARAM
{
  public int ulBits;

  public IntPtr ToBin(out int size)
  {
    CK_MECHANISM_PARAM_Buffer buf = new CK_MECHANISM_PARAM_Buffer(4, 0);
    buf.Pack(ulBits);
    size = buf.Size;
    return buf.Ptr;
  }
}

}