using System;

namespace unbound.cryptoki {

public class CK_RSA_PKCS_PSS_PARAMS : CK_MECHANISM_PARAM
{
  public uint hashAlg;
  public uint mgf;
  public int sLen;

  public IntPtr ToBin(out int size)
  {
    CK_MECHANISM_PARAM_Buffer buf = new CK_MECHANISM_PARAM_Buffer(12, 0);
    buf.Pack(hashAlg);
    buf.Pack(mgf);
    buf.Pack(sLen);
    size = buf.Size;
    return buf.Ptr;
  }
}

}