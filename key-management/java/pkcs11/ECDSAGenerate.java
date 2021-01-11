package com.unboundtech.key_management.pkcs11;

import com.dyadicsec.cryptoki.*;

public class ECDSAGenerate
{
  static final byte[] P256_OID = new byte[] {0x06, 0x08, 0x2a, (byte)0x86, 0x48, (byte)0xce, 0x3d, 0x03, 0x01, 0x07};

  /***************
   *
   * @param args - if default user password is non-empty, provide it as first arg
   * @throws Exception
   */
  public static void main(String[] args) throws Exception
  {
    char[] pwd = null;
    if (args.length > 0) pwd = args[0].toCharArray();


    int slotId = 0;
    byte[] keyName = "TEST ECDSA KEY".getBytes("UTF-8");
    Library.C_Initialize();

    // Open PKCS#11 session
    System.out.println("Open Session and login with default user");
    CK_SESSION_HANDLE session = Library.C_OpenSession(slotId, CK.CKF_RW_SESSION | CK.CKF_SERIAL_SESSION);
    Library.C_Login(session, CK.CKU_USER, pwd); // optional

    // Generate key pair
    System.out.println("Generate key pair");
    int[] keyHandles = Library.C_GenerateKeyPair(session,
      new CK_MECHANISM(CK.CKM_EC_KEY_PAIR_GEN), // EC generation mechanism
      new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, false),
  			new CK_ATTRIBUTE(CK.CKA_EC_PARAMS, P256_OID), // Curve P256
      },
      new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
        new CK_ATTRIBUTE(CK.CKA_SIGN, true), // for ECDSA
        new CK_ATTRIBUTE(CK.CKA_ID, keyName),
      });
    assert keyHandles.length==2;

    // Find the ECDSA private key
    System.out.println("Find the ECDSA private key");
    Library.C_FindObjectsInit(session, new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
        new CK_ATTRIBUTE(CK.CKA_CLASS, CK.CKO_PRIVATE_KEY),
        new CK_ATTRIBUTE(CK.CKA_KEY_TYPE, CK.CKK_EC),
        new CK_ATTRIBUTE(CK.CKA_ID, keyName),
      });
    int[] foundKeyHandles = Library.C_FindObjects(session, 1);
    Library.C_FindObjectsFinal(session);
    assert foundKeyHandles.length==1;
    System.out.println("Key found - success");

    // Close PKCS#11 session
    System.out.println("Close PKCS#11 session");
    Library.C_CloseSession(session);
    Library.C_Finalize();
  }
}
