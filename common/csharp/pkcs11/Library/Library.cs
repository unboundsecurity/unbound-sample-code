using System;
using System.Runtime.InteropServices;
using System.Text;

namespace unbound.cryptoki {

public static partial class Library
{
  [DllImport("kernel32")]                            private extern static IntPtr LoadLibrary(string lpLibFileName);
  [DllImport("kernel32", CharSet = CharSet.Ansi)]    private extern static IntPtr GetProcAddress(IntPtr module, string lpProcName);   


  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GetFunctionList(out IntPtr funcList);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_Initialize(IntPtr dummy);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_Finalize(IntPtr dummy);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GetInfo(out CK_INFO.Native info);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GetSlotList(byte present,
    [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] uint[] listPtr, ref int count);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GetSlotInfo(uint slot, out CK_SLOT_INFO.Native info);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GetTokenInfo(uint slot, out CK_TOKEN_INFO.Native info);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GetMechanismList(uint slot,
    [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] uint[] listPtr, ref int count);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GetMechanismInfo(uint slot, uint mechanism, out CK_MECHANISM_INFO info);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_OpenSession(uint slot, uint flags,
    IntPtr app, IntPtr notify, out uint session);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GetSessionInfo(uint session, out CK_SESSION_INFO info);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_CloseSession(uint session);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_CloseAllSessions(uint slot);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_Login(uint session, uint type,
    [MarshalAs(UnmanagedType.LPArray)] byte[] pin, uint pinLen);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_Logout(uint session);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_DestroyObject(uint session, uint obj);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_CreateObject(uint session,
    [MarshalAs(UnmanagedType.LPArray)] CK_ATTRIBUTE.Native[] t, int tLen, out uint obj);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_SetAttributeValue(uint session, uint obj,
    [MarshalAs(UnmanagedType.LPArray)] CK_ATTRIBUTE.Native[] t, int tLen);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GetAttributeValue(uint session, uint obj,
    [In][Out][MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] CK_ATTRIBUTE.Native[] t, int tLen);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_FindObjectsInit(uint session,
    [MarshalAs(UnmanagedType.LPArray)] CK_ATTRIBUTE.Native[] t, int tLen);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_FindObjects(uint session,
    [MarshalAs(UnmanagedType.LPArray)] out uint[] listPtr, int maxCount, out int count);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_FindObjectsFinal(uint session);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_SeedRandom(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] ptr, uint size);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GenerateRandom(uint session,
    [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] byte[] ptr, uint size);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GenerateKeyPair(uint session, CK_MECHANISM.Native mech,
    [MarshalAs(UnmanagedType.LPArray)] CK_ATTRIBUTE.Native[] tPub, int tPubLen,
    [MarshalAs(UnmanagedType.LPArray)] CK_ATTRIBUTE.Native[] tPrv, int tPrvLen, out uint pubKey, out uint prvKey);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_WrapKey(uint session, CK_MECHANISM.Native mech, uint wrappingKey, uint key,
    [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_UnwrapKey(uint session, CK_MECHANISM.Native mech, uint unwrappingKey,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize,
    [MarshalAs(UnmanagedType.LPArray)] CK_ATTRIBUTE.Native[] t, int tLen, out uint key);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_DeriveKey(uint session, CK_MECHANISM.Native mech, uint baseKey,
    [MarshalAs(UnmanagedType.LPArray)] CK_ATTRIBUTE.Native[] t, int tLen, out uint key);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_GenerateKey(uint session, CK_MECHANISM.Native mech,
    [MarshalAs(UnmanagedType.LPArray)] CK_ATTRIBUTE.Native[] t, int tLen, out uint key);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_EncryptInit(uint session, CK_MECHANISM.Native mech, uint key);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_EncryptUpdate(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize, [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_EncryptFinal(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_Encrypt(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize, [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_DecryptInit(uint session, CK_MECHANISM.Native mech, uint key);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_DecryptUpdate(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize, [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_DecryptFinal(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_Decrypt(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize, [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_SignInit(uint session, CK_MECHANISM.Native mech, uint key);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_SignUpdate(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_SignFinal(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_Sign(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize, [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_VerifyInit(uint session, CK_MECHANISM.Native mech, uint key);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_VerifyUpdate(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_VerifyFinal(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] signature, int signatureSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_Verify(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize, [MarshalAs(UnmanagedType.LPArray)] byte[] signature, int signatureSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_DigestInit(uint session, CK_MECHANISM.Native mech);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_DigestUpdate(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_DigestKey(uint session, uint key);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_DigestFinal(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);
  [UnmanagedFunctionPointer(CallingConvention.Cdecl)] private delegate uint type_C_Digest(uint session,
    [MarshalAs(UnmanagedType.LPArray)] byte[] src, int srcSize, [MarshalAs(UnmanagedType.LPArray)] byte[] dst, ref int dstSize);


  [StructLayout(LayoutKind.Sequential, Pack = 1)] private class CK_FUNCTION_LIST
  {
    public CK_VERSION version;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_Initialize C_Initialize;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_Finalize C_Finalize;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GetInfo C_GetInfo;
    IntPtr C_GetFunctionList;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GetSlotList C_GetSlotList;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GetSlotInfo C_GetSlotInfo;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GetTokenInfo C_GetTokenInfo;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GetMechanismList C_GetMechanismList;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GetMechanismInfo C_GetMechanismInfo;
    IntPtr C_InitToken;
    IntPtr C_InitPIN;
    IntPtr C_SetPIN;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_OpenSession C_OpenSession;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_CloseSession C_CloseSession;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_CloseAllSessions C_CloseAllSessions;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GetSessionInfo C_GetSessionInfo;
    IntPtr C_GetOperationState;
    IntPtr C_SetOperationState;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_Login C_Login;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_Logout C_Logout;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_CreateObject C_CreateObject;
    IntPtr C_CopyObject;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_DestroyObject C_DestroyObject;
    IntPtr C_GetObjectSize;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GetAttributeValue C_GetAttributeValue;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_SetAttributeValue C_SetAttributeValue;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_FindObjectsInit C_FindObjectsInit;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_FindObjects C_FindObjects;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_FindObjectsFinal C_FindObjectsFinal;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_EncryptInit C_EncryptInit;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_Encrypt C_Encrypt;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_EncryptUpdate C_EncryptUpdate;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_EncryptFinal C_EncryptFinal;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_DecryptInit C_DecryptInit;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_Decrypt C_Decrypt;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_DecryptUpdate C_DecryptUpdate;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_DecryptFinal C_DecryptFinal;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_DigestInit C_DigestInit;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_Digest C_Digest;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_DigestUpdate C_DigestUpdate;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_DigestKey C_DigestKey;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_DigestFinal C_DigestFinal;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_SignInit C_SignInit;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_Sign C_Sign;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_SignUpdate C_SignUpdate;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_SignFinal C_SignFinal;
    IntPtr C_SignRecoverInit;
    IntPtr C_SignRecover;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_VerifyInit C_VerifyInit;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_Verify C_Verify;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_VerifyUpdate C_VerifyUpdate;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_VerifyFinal C_VerifyFinal;
    IntPtr C_VerifyRecoverInit;
    IntPtr C_VerifyRecover;
    IntPtr C_DigestEncryptUpdate;
    IntPtr C_DecryptDigestUpdate;
    IntPtr C_SignEncryptUpdate;
    IntPtr C_DecryptVerifyUpdate;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GenerateKey C_GenerateKey;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GenerateKeyPair C_GenerateKeyPair;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_WrapKey C_WrapKey;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_UnwrapKey C_UnwrapKey;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_DeriveKey C_DeriveKey;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_SeedRandom C_SeedRandom;
    [MarshalAs(UnmanagedType.FunctionPtr)]      public type_C_GenerateRandom C_GenerateRandom;
    IntPtr C_GetFunctionStatus;
    IntPtr C_CancelFunction;
    IntPtr C_WaitForSlotEvent;
  }

  private static CK_FUNCTION_LIST fl = new CK_FUNCTION_LIST();

  static Library()
  {
    IntPtr module = LoadLibrary("ekmpkcs11.dll");
    if (module == IntPtr.Zero) throw new CKR_Exception(CK.CKR_FUNCTION_FAILED);
    IntPtr ptr_C_GetFunctionList = GetProcAddress(module, "C_GetFunctionList");

    if (ptr_C_GetFunctionList == IntPtr.Zero) throw new CKR_Exception(CK.CKR_FUNCTION_FAILED);

    type_C_GetFunctionList C_GetFunctionList = (type_C_GetFunctionList)Marshal.GetDelegateForFunctionPointer(ptr_C_GetFunctionList, typeof(type_C_GetFunctionList));

    IntPtr funcListPtr;
    CKR_Exception.check(C_GetFunctionList(out funcListPtr));

    Marshal.PtrToStructure(funcListPtr, fl);
  }

  public static void C_Initialize()
  {
    CKR_Exception.check(fl.C_Initialize(IntPtr.Zero), "C_Initialize");
  }
  public static void C_Finalize()
  {
    CKR_Exception.check(fl.C_Finalize(IntPtr.Zero), "C_Finalize");
  }

  internal static string Utf8ToString(byte[] utf8)
  {
    int utf8_size = utf8.Length;
    Decoder d = Encoding.UTF8.GetDecoder();

    int charCount = d.GetCharCount(utf8, 0, utf8_size);
    char[] buffer = new char[charCount];
    d.GetChars(utf8, 0, utf8_size, buffer, 0);
    while (charCount > 0 && (buffer[charCount - 1] == ' ' || buffer[charCount - 1] == '\0')) charCount--;
    return new string(buffer, 0, charCount);
  }

  public static CK_INFO C_GetInfo()
  {
    CK_INFO.Native native = new CK_INFO.Native();
    CKR_Exception.check(fl.C_GetInfo(out native), "C_GetInfo");
    return new CK_INFO(native);
  }

  public static CK_SLOT_ID[] C_GetSlotList(bool present)
  {
    byte tokenPresent = (byte)(present ? 1 : 0);
    int count = 0;
    CKR_Exception.check(fl.C_GetSlotList(tokenPresent, null, ref count), "C_GetSlotList");
    if (count == 0) return new CK_SLOT_ID[0];

    uint[] list = new uint[count];
    if (count > 0) CKR_Exception.check(fl.C_GetSlotList(tokenPresent, list, ref count), "C_GetSlotList");
    
    CK_SLOT_ID[] slots = new CK_SLOT_ID[count];
    for (int i=0; i<count; i++) slots[i] = new CK_SLOT_ID(list[i]);
    return slots;
  }

  public static uint[] C_GetMechanismList(CK_SLOT_ID slot)
  {
    int count = 0;
    CKR_Exception.check(fl.C_GetMechanismList(slot.Id, null, ref count), "C_GetMechanismList");
    if (count == 0) return new uint[0];

    uint[] list = new uint[count];
    if (count > 0) CKR_Exception.check(fl.C_GetMechanismList(slot.Id, list, ref count), "C_GetMechanismList");
    return list;
  }

  public static CK_SLOT_INFO C_GetSlotInfo(CK_SLOT_ID slot)
  {
    CK_SLOT_INFO.Native native = new CK_SLOT_INFO.Native();
    CKR_Exception.check(fl.C_GetSlotInfo(slot.Id, out native), "C_GetSlotInfo");
    return new CK_SLOT_INFO(native);
  }

  public static CK_TOKEN_INFO C_GetTokenInfo(CK_SLOT_ID slot)
  {
    CK_TOKEN_INFO.Native native = new CK_TOKEN_INFO.Native();
    CKR_Exception.check(fl.C_GetTokenInfo(slot.Id, out native), "C_GetTokenInfo");
    return new CK_TOKEN_INFO(native);
  }
  public static CK_MECHANISM_INFO C_GetMechanismInfo(CK_SLOT_ID slot, uint type)
  {
    CK_MECHANISM_INFO info = new CK_MECHANISM_INFO();
    CKR_Exception.check(fl.C_GetMechanismInfo(slot.Id, type, out info), "C_GetMechanismInfo (" + String.Format("{0:X}", type) + ")");
    return info;
  }

  public static CK_SESSION_HANDLE C_OpenSession(CK_SLOT_ID slotID, uint flags = CK.CKF_SERIAL_SESSION|CK.CKF_RW_SESSION)
  {
    uint handle = 0;
    CKR_Exception.check(fl.C_OpenSession(slotID.Id, flags, IntPtr.Zero, IntPtr.Zero, out handle), "C_OpenSession (" + slotID + ")");
    return new CK_SESSION_HANDLE(handle);
  }

  public static void C_CloseSession(CK_SESSION_HANDLE session)
  {
    CKR_Exception.check(fl.C_CloseSession(session.Handle), "C_OpenSession");
  }

  public static void C_CloseAllSessions(CK_SLOT_ID slot)
  {
    CKR_Exception.check(fl.C_CloseAllSessions(slot.Id), "C_CloseAllSessions");
  }

  public static CK_SESSION_INFO C_GetSessionInfo(CK_SESSION_HANDLE session)
  {
    CK_SESSION_INFO info = new CK_SESSION_INFO();
    CKR_Exception.check(fl.C_GetSessionInfo(session.Handle, out info), "C_GetSessionInfo");
    return info;
  }

  public static void C_Login(CK_SESSION_HANDLE session, uint userType, string pin)
  {
    byte[] bytes = Encoding.UTF8.GetBytes(pin);
    CKR_Exception.check(fl.C_Login(session.Handle, userType, bytes, (uint)bytes.Length), "C_Login");
  }

  public static void C_Logout(CK_SESSION_HANDLE session)
  {
    CKR_Exception.check(fl.C_Logout(session.Handle), "C_Logout");
  }
  public static void C_DestroyObject(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE obj)
  {
    CKR_Exception.check(fl.C_DestroyObject(session.Handle, obj.Handle), "C_DestroyObject");
  }

  public static void C_SeedRandom(CK_SESSION_HANDLE session, byte[] data)
  {
    CKR_Exception.check(fl.C_SeedRandom(session.Handle, data, (uint)data.Length));
  }

  public static byte[] C_GenerateRandom(CK_SESSION_HANDLE session, uint len)
  {
    byte[] dst = new byte[len];
    CKR_Exception.check(fl.C_GenerateRandom(session.Handle, dst, len));
    return dst;
  }

    
  public static CK_OBJECT_HANDLE C_CreateObject(CK_SESSION_HANDLE session, CK_ATTRIBUTE[] pTemplate)
  {
    CK_ATTRIBUTE.Native[] t = CK_ATTRIBUTE.ToNative(pTemplate);
    uint objHandle = 0;
    try { CKR_Exception.check(fl.C_CreateObject(session.Handle, t, t.Length, out objHandle), "C_CreateObject"); }
    finally { CK_ATTRIBUTE.Free(t); }
    return new CK_OBJECT_HANDLE(objHandle);
  }

  public static void C_SetAttributeValue(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE obj, CK_ATTRIBUTE[] pTemplate)
  {
    CK_ATTRIBUTE.Native[] t = CK_ATTRIBUTE.ToNative(pTemplate);
    try { CKR_Exception.check(fl.C_SetAttributeValue(session.Handle, obj.Handle, t, t.Length), "C_SetAttributeValue"); }
    finally { CK_ATTRIBUTE.Free(t); }
  }

  public static void C_FindObjectsInit(CK_SESSION_HANDLE session, CK_ATTRIBUTE[] pTemplate)
  {
    CK_ATTRIBUTE.Native[] t = CK_ATTRIBUTE.ToNative(pTemplate);
    try { CKR_Exception.check(fl.C_FindObjectsInit(session.Handle, t, t.Length), "C_FindObjectsInit"); }
    finally { CK_ATTRIBUTE.Free(t); }
  }

  public static CK_OBJECT_HANDLE[] C_FindObjects(CK_SESSION_HANDLE session, int ulMaxObjectCount)
  {
    uint[] buf = new uint[ulMaxObjectCount];
    int count = 0;
    CKR_Exception.check(fl.C_FindObjects(session.Handle, out buf, ulMaxObjectCount, out count), "C_FindObjectsInit");

    CK_OBJECT_HANDLE[] res = new CK_OBJECT_HANDLE[count];
    for (int i=0; i<count; i++) res[i] = new CK_OBJECT_HANDLE(buf[i]); 
    return res;
  }

  public static void C_FindObjectsFinal(CK_SESSION_HANDLE session)
  {
    CKR_Exception.check(fl.C_FindObjectsFinal(session.Handle), "C_FindObjectsFinal");
  }

  public static void C_GetAttributeValue(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE obj, CK_ATTRIBUTE[] pTemplate)
  {
    CK_ATTRIBUTE.Native[] t = null;
    IntPtr buf = IntPtr.Zero;
    try
    {
      bool isKnownSize;
      t = CK_ATTRIBUTE.ToNativeReadSize(pTemplate, out isKnownSize);

      if (!isKnownSize)
      {
        CKR_Exception.check(fl.C_GetAttributeValue(session.Handle, obj.Handle, t, t.Length), "C_GetAttributeValue(null)");
      }

      CK_ATTRIBUTE.ToNativeRead(t);
      CKR_Exception.check(fl.C_GetAttributeValue(session.Handle, obj.Handle, t, t.Length), "C_GetAttributeValue");
      CK_ATTRIBUTE.FromNative(pTemplate, t);
    }
    finally
    {
      CK_ATTRIBUTE.Free(t);
    }
  }

  public static void C_GenerateKeyPair(CK_SESSION_HANDLE session, CK_MECHANISM mech,
      CK_ATTRIBUTE[] tPub,
      CK_ATTRIBUTE[] tPrv,
      out CK_OBJECT_HANDLE pubKey,
      out CK_OBJECT_HANDLE prvKey)
  {
    CK_ATTRIBUTE.Native[] tPubRaw = CK_ATTRIBUTE.ToNative(tPub);
    CK_ATTRIBUTE.Native[] tPrvRaw = CK_ATTRIBUTE.ToNative(tPrv);
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    uint pubKeyHandle = 0; 
    uint prvKeyHandle = 0;
    try { CKR_Exception.check(fl.C_GenerateKeyPair(session.Handle, m, tPubRaw, tPub.Length, tPrvRaw, tPrv.Length, out pubKeyHandle, out prvKeyHandle), "C_GenerateKeyPair"); }
    finally { CK_ATTRIBUTE.Free(tPubRaw); CK_ATTRIBUTE.Free(tPrvRaw); m.Free();  }
    pubKey = new CK_OBJECT_HANDLE(pubKeyHandle);
    prvKey = new CK_OBJECT_HANDLE(prvKeyHandle);

  }

  public static void C_EncryptInit(CK_SESSION_HANDLE session, CK_MECHANISM mech, CK_OBJECT_HANDLE key)
  {
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    try { CKR_Exception.check(fl.C_EncryptInit(session.Handle, m, key.Handle), "C_EncryptInit"); }
    finally { m.Free(); }
  }

  public static void C_DecryptInit(CK_SESSION_HANDLE session, CK_MECHANISM mech, CK_OBJECT_HANDLE key)
  {
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    try { CKR_Exception.check(fl.C_DecryptInit(session.Handle, m, key.Handle), "C_DecryptInit"); }
    finally { m.Free(); }
  }

  public static void C_SignInit(CK_SESSION_HANDLE session, CK_MECHANISM mech, CK_OBJECT_HANDLE key)
  {
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    try { CKR_Exception.check(fl.C_SignInit(session.Handle, m, key.Handle), "C_SignInit"); }
    finally { m.Free(); }
  }

  public static void C_VerifyInit(CK_SESSION_HANDLE session, CK_MECHANISM mech, CK_OBJECT_HANDLE key)
  {
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    try { CKR_Exception.check(fl.C_VerifyInit(session.Handle, m, key.Handle), "C_VerifyInit"); }
    finally { m.Free(); }
  }

  public static void C_DigestInit(CK_SESSION_HANDLE session, CK_MECHANISM mech)
  {
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    try { CKR_Exception.check(fl.C_DigestInit(session.Handle, m), "C_DigestInit"); }
    finally { m.Free(); }
  }

  public static int C_EncryptUpdate(CK_SESSION_HANDLE session, byte[] input,  byte[] output)
  {
    int inSize = input == null ? 0 : input.Length;
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_EncryptUpdate(session.Handle, input, inSize, output, ref outSize), "C_EncryptUpdate");
    return outSize;
  }

  public static int C_EncryptFinal(CK_SESSION_HANDLE session, byte[] output)
  {
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_EncryptFinal(session.Handle, output, ref outSize), "C_EncryptFinal");
    return outSize;
  }

  public static int C_Encrypt(CK_SESSION_HANDLE session, byte[] input, byte[] output)
  {
    int inSize = input == null ? 0 : input.Length;
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_Encrypt(session.Handle, input, inSize, output, ref outSize), "C_Encrypt");
    return outSize;
  }

  private static byte[] SubArray(byte[] src, int length)
  {
    if (src.Length==length) return src;
    byte[] result = new byte[length];
    Array.Copy(src, 0, result, 0, length);
    return result;
  }

  public static byte[] C_Encrypt(CK_SESSION_HANDLE session, byte[] input)
  {
    int outSize = C_Encrypt(session, input, null);
    byte[] output = new byte[outSize];
    outSize = C_Encrypt(session, input, output);
    return SubArray(output, outSize);
  }

  public static int C_DecryptUpdate(CK_SESSION_HANDLE session, byte[] input, byte[] output)
  {
    int inSize = input == null ? 0 : input.Length;
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_DecryptUpdate(session.Handle, input, inSize, output, ref outSize), "C_DecryptUpdate");
    return outSize;
  }

  public static int C_DecryptFinal(CK_SESSION_HANDLE session, byte[] output)
  {
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_DecryptFinal(session.Handle, output, ref outSize), "C_DecryptFinal");
    return outSize;
  }
  public static int C_Decrypt(CK_SESSION_HANDLE session, byte[] input, byte[] output)
  {
    int inSize = input == null ? 0 : input.Length;
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_Decrypt(session.Handle, input, inSize, output, ref outSize), "C_Decrypt");
    return outSize;
  }

  public static byte[] C_Decrypt(CK_SESSION_HANDLE session, byte[] input)
  {
    int outSize = C_Decrypt(session, input, null);
    byte[] output = new byte[outSize];
    outSize = C_Decrypt(session, input, output);
    return SubArray(output, outSize);
  }

  public static void C_SignUpdate(CK_SESSION_HANDLE session, byte[] input)
  {
    int inSize = input == null ? 0 : input.Length;
    CKR_Exception.check(fl.C_SignUpdate(session.Handle, input, inSize), "C_SignUpdate");
  }

  public static int C_SignFinal(CK_SESSION_HANDLE session, byte[] output)
  {
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_SignFinal(session.Handle, output, ref outSize), "C_SignFinal");
    return outSize;
  }

  public static int C_Sign(CK_SESSION_HANDLE session, byte[] input, byte[] output)
  {
    int inSize = input == null ? 0 : input.Length;
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_Sign(session.Handle, input, inSize, output, ref outSize), "C_Sign");
    return outSize;
  }

  public static byte[] C_Sign(CK_SESSION_HANDLE session, byte[] input)
  {
    int outSize = C_Sign(session, input, null);
    byte[] output = new byte[outSize];
    outSize = C_Sign(session, input, output);
    return SubArray(output, outSize);
  }

  public static void C_VerifyUpdate(CK_SESSION_HANDLE session, byte[] input)
  {
    int inSize = input == null ? 0 : input.Length;
    CKR_Exception.check(fl.C_VerifyUpdate(session.Handle, input, inSize), "C_VerifyUpdate");
  }

  public static void C_VerifyFinal(CK_SESSION_HANDLE session, byte[] signature)
  {
    int sigSize = signature == null ? 0 : signature.Length;
    CKR_Exception.check(fl.C_VerifyFinal(session.Handle, signature, sigSize), "C_VerifyFinal");
  }

  public static void C_Verify(CK_SESSION_HANDLE session, byte[] input, byte[] signature)
  {
    int inSize = input == null ? 0 : input.Length;
    int sigSize = signature == null ? 0 : signature.Length;
    CKR_Exception.check(fl.C_Verify(session.Handle, input, inSize, signature, sigSize), "C_Verify");
  }

  public static void C_DigestUpdate(CK_SESSION_HANDLE session, byte[] input)
  {
    int inSize = input == null ? 0 : input.Length;
    CKR_Exception.check(fl.C_DigestUpdate(session.Handle, input, inSize), "C_DigestUpdate");
  }

  public static int C_DigestFinal(CK_SESSION_HANDLE session, byte[] output)
  {
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_DigestFinal(session.Handle, output, ref outSize), "C_DigestFinal");
    return outSize;
  }

  public static int C_Digest(CK_SESSION_HANDLE session, byte[] input, byte[] output)
  {
    int inSize = input == null ? 0 : input.Length;
    int outSize = output == null ? 0 : output.Length;
    CKR_Exception.check(fl.C_Digest(session.Handle, input, inSize, output, ref outSize), "C_Digest");
    return outSize;
  }

  public static byte[] C_Digest(CK_SESSION_HANDLE session, byte[] input)
  {
    int outSize = C_Digest(session, input, null);
    byte[] output = new byte[outSize];
    outSize = C_Digest(session, input, output);
    return SubArray(output, outSize);
  }

  public static void C_DigestKey(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE key)
  {
    CKR_Exception.check(fl.C_DigestKey(session.Handle, key.Handle), "C_DigestKey");
  }

  public static int C_WrapKey(CK_SESSION_HANDLE session, CK_MECHANISM mech, CK_OBJECT_HANDLE key, CK_OBJECT_HANDLE wrappedKey, byte[] output)
  {
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    int outSize = output == null ? 0 : output.Length;
    try { CKR_Exception.check(fl.C_WrapKey(session.Handle, m, key.Handle, wrappedKey.Handle, output, ref outSize), "C_WrapKey"); }
    finally { m.Free(); }
    return outSize;
  }

  public static byte[] C_WrapKey(CK_SESSION_HANDLE session, CK_MECHANISM mech, CK_OBJECT_HANDLE key, CK_OBJECT_HANDLE wrappedKey)
  {
    int outSize = C_WrapKey(session, mech, key, wrappedKey, null);
    byte[] output = new byte[outSize];
    outSize = C_WrapKey(session, mech, key, wrappedKey, output);
    return SubArray(output, outSize);
  }

  public static CK_OBJECT_HANDLE C_UnwrapKey(CK_SESSION_HANDLE session, CK_MECHANISM mech, CK_OBJECT_HANDLE key, byte[] input, CK_ATTRIBUTE[] pTemplate)
  {
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    CK_ATTRIBUTE.Native[] t = CK_ATTRIBUTE.ToNative(pTemplate);
    int inSize = input == null ? 0 : input.Length;
    uint unwrappedKeyHandle = 0;
    try { CKR_Exception.check(fl.C_UnwrapKey(session.Handle, m, key.Handle, input, inSize, t, t.Length, out unwrappedKeyHandle), "C_UnwrapKey"); }
    finally { m.Free(); CK_ATTRIBUTE.Free(t); }
    return new CK_OBJECT_HANDLE(unwrappedKeyHandle);
  }

  public static CK_OBJECT_HANDLE C_DeriveKey(CK_SESSION_HANDLE session, CK_MECHANISM mech, CK_OBJECT_HANDLE key, CK_ATTRIBUTE[] pTemplate)
  {
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    CK_ATTRIBUTE.Native[] t = CK_ATTRIBUTE.ToNative(pTemplate);
    uint derivedKeyHandle = 0;

    try { CKR_Exception.check(fl.C_DeriveKey(session.Handle, m, key.Handle, t, t.Length, out derivedKeyHandle), "C_DeriveKey"); }
    finally { m.Free(); CK_ATTRIBUTE.Free(t); }
    return new CK_OBJECT_HANDLE(derivedKeyHandle);
  }

  public static CK_OBJECT_HANDLE C_GenerateKey(CK_SESSION_HANDLE session, CK_MECHANISM mech, CK_ATTRIBUTE[] pTemplate)
  {
    CK_MECHANISM.Native m = new CK_MECHANISM.Native(mech);
    CK_ATTRIBUTE.Native[] t = CK_ATTRIBUTE.ToNative(pTemplate);
    uint keyHandle = 0;

    try { CKR_Exception.check(fl.C_GenerateKey(session.Handle, m, t, t.Length, out keyHandle), "C_GenerateKey"); }
    finally { m.Free(); CK_ATTRIBUTE.Free(t); }
    return new CK_OBJECT_HANDLE(keyHandle);
  }
}

} // namespace unbound.cryptoki
