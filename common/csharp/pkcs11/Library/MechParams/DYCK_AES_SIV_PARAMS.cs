using System;
using System.Runtime.InteropServices;

namespace unbound.cryptoki {

public class DYCK_AES_SIV_PARAMS : CK_MECHANISM_PARAM
{
  public byte[][] pAuthData;

  public IntPtr ToBin(out int size)
  {
    int count = pAuthData==null ? 0 : pAuthData.Length;
    int authDataLen = 0;
    for (int i = 0; i < count; i++) authDataLen += pAuthData[i].Length;
    size = 12;

    IntPtr ptr = Marshal.AllocCoTaskMem(size + count*12 + authDataLen);
    int datasOffset = 12;
    int authOffset = 12 + 12*count;
    IntPtr datas = new IntPtr(ptr.ToInt64() + datasOffset);
    Marshal.WriteInt32(ptr, 0, count);
    Marshal.WriteIntPtr(ptr, 4, datas);

    for (int i = 0; i < count; i++, datasOffset+=12)
    {
      IntPtr pAuth = new IntPtr(ptr.ToInt64() + authOffset);
      Marshal.WriteInt32 (ptr, datasOffset,   pAuthData[i].Length);
      Marshal.WriteIntPtr(ptr, datasOffset+4, pAuth);
      Marshal.Copy(pAuthData[i], 0, pAuth, pAuthData[i].Length);
      authOffset += pAuthData[i].Length;
    }

    return ptr;
  }

}

}
