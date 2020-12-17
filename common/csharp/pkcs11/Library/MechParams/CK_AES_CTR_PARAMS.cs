using System;
using System.Runtime.InteropServices;

namespace unbound.cryptoki {


  public class CK_AES_CTR_PARAMS : CK_MECHANISM_PARAM
  {
    public int ulCounterBits;
    public byte[] cb;

    public IntPtr ToBin(out int size)
    {
      if (cb.Length!=16) throw new CKR_Exception(CK.CKR_MECHANISM_PARAM_INVALID);
      CK_MECHANISM_PARAM_Buffer buf = new CK_MECHANISM_PARAM_Buffer(0, 20);
      buf.Pack(ulCounterBits);
      buf.Pack(cb);
      size = buf.Size;
      return buf.Ptr;
    }
  }




} //namespace unbound.cryptoki