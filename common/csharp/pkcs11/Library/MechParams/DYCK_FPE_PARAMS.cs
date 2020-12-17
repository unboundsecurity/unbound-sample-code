using System;

namespace unbound.cryptoki {

public class DYCK_FPE_PARAMS : CK_MECHANISM_PARAM
{
  public uint ulMode;
  public char[] pFormat;
  public int ulMaxLen;

  public IntPtr ToBin(out int size)
  {
    int ulFormatLen = pFormat==null ? 0 : pFormat.Length+1;
    CK_MECHANISM_PARAM_Buffer buf = new CK_MECHANISM_PARAM_Buffer(16, ulFormatLen);
    buf.Pack(ulMode);
    buf.Pack(pFormat);
    buf.Pack(ulMaxLen);
    size = buf.Size;
    return buf.Ptr;
  }
}

}