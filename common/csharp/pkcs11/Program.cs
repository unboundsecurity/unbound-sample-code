using System;
using System.Linq;
using System.Text;
using unbound.cryptoki;


namespace sample_pkcs11_csharp
{
  class Program
  {
    
    static void Main(string[] args)
    {
      Library.C_Initialize();
      CK_SLOT_ID[] slots = Library.C_GetSlotList(true);
      CK_SLOT_ID slot = slots[0];
      CK_SESSION_HANDLE session = Library.C_OpenSession(slot);

      CK_OBJECT_HANDLE hKey = Library.C_GenerateKey(session, new CK_MECHANISM(CK.CKM_AES_KEY_GEN), new CK_ATTRIBUTE[] {
        new CK_ATTRIBUTE(CK.CKA_TOKEN,     true),
        new CK_ATTRIBUTE(CK.CKA_CLASS,     CK.CKO_SECRET_KEY),
        new CK_ATTRIBUTE(CK.CKA_KEY_TYPE,  CK.CKK_AES),
        new CK_ATTRIBUTE(CK.CKA_VALUE_LEN, 32),
      });

      CK_ATTRIBUTE[] t = new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_ID),
        new CK_ATTRIBUTE(CK.CKA_CLASS),
        new CK_ATTRIBUTE(CK.CKA_KEY_TYPE),
      };

      Library.C_GetAttributeValue(session, hKey, t);

      byte[] iv = Library.C_GenerateRandom(session, 12);
      byte[] plain = Encoding.UTF8.GetBytes("TEST PLAIN DATA");

      CK_MECHANISM mech = new CK_MECHANISM(CK.CKM_AES_GCM, new CK_GCM_PARAMS(iv, null, 96));
      Library.C_EncryptInit(session, mech, hKey);
      byte[] enc = Library.C_Encrypt(session, plain);

      Library.C_DecryptInit(session, mech, hKey);
      byte[] dec = Library.C_Decrypt(session, enc);

      if (!Enumerable.SequenceEqual(dec, plain)) throw new Exception("ENC/DEC mismatch");
    }
  }

}
