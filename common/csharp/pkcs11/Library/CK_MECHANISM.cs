using System;
using System.Runtime.InteropServices;

namespace unbound.cryptoki {

public interface CK_MECHANISM_PARAM
{
  IntPtr ToBin(out int size);  
}

internal class CK_MECHANISM_PARAM_Buffer
{
  internal CK_MECHANISM_PARAM_Buffer(int size, int ext)
  {
    Size = size;
    Ptr = Marshal.AllocCoTaskMem(size+ext);
    BasePtr = Ptr.ToInt64();
    Offset = 0;
    ExtOffset = Offset + size;
  }  
  
  internal IntPtr Ptr { get; }
  internal int Size { get; }
  private long BasePtr;
  private int Offset;
  private int ExtOffset;

  internal void Pack(int value)
  {
    Marshal.WriteInt32 (Ptr, Offset, value); 
    Offset += 4;
  }

  internal void Pack(bool value)
  {
    Marshal.WriteByte (Ptr, Offset, (byte)(value ? 1 : 0)); 
    Offset += 1;
  }

  internal void Pack(uint value)
  {
    Pack((int)value);
  }

  internal void Pack(byte[] a)
  {
    int aLen = a==null ? 0 : a.Length;
    long ptrValue = aLen==0 ? 0 : BasePtr + ExtOffset;
    Marshal.WriteInt64(Ptr, Offset, ptrValue); 
    Offset += 8;

    if (aLen>0)
    {
      Marshal.Copy(a,  0, new IntPtr(BasePtr+ExtOffset),  aLen);
      ExtOffset += aLen;
    }
  }

  internal void Pack(char[] a)
  {
    int aLen = a==null ? 0 : a.Length;
    long ptrValue = aLen==0 ? 0 : BasePtr + ExtOffset;
    Marshal.WriteInt64(Ptr, Offset, ptrValue); 
    Offset += 8;

    if (aLen>0)
    {
      for (int i = 0; i < aLen; i++) Marshal.WriteByte(Ptr, ExtOffset+i, (byte)a[i]);
      Marshal.WriteByte(Ptr, ExtOffset+aLen, 0); // zero terminated
      ExtOffset += aLen+1;
    }
  }
}

public class CK_MECHANISM
{
  public uint mechanism;
  public CK_MECHANISM_PARAM parameter;

  public CK_MECHANISM(uint mechanism, CK_MECHANISM_PARAM parameter=null)
  {
    this.mechanism = mechanism;
    this.parameter = parameter;
  }

  [StructLayout(LayoutKind.Sequential, Pack = 1)]
  internal struct Native
  {
    public uint mechanism;
    public IntPtr pParameter;
    public int ulParameterLen;

    internal Native(CK_MECHANISM mech)
    {
      mechanism = mech.mechanism;
      ulParameterLen = 0;
      pParameter = IntPtr.Zero;
      if (mech.parameter!=null) pParameter = mech.parameter.ToBin(out ulParameterLen);
    }

    internal void Free()
    {
      if (pParameter!=IntPtr.Zero) Marshal.FreeCoTaskMem(pParameter);
      pParameter = IntPtr.Zero;
    }
  }

}

} //unbound.cryptoki
