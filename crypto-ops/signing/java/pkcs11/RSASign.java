package com.unboundtech.crypto_ops.signing.pkcs11;

import com.dyadicsec.cryptoki.*;

public class RSASign
{
  public static void main(String[] args) throws Exception
  {
    char[] pwd = null;
    if (args.length > 0) pwd = args[0].toCharArray();

    final byte[] dataToSign = "DATA TO SIGN".getBytes("UTF-8");
    int slotId = 0;
    Library.C_Initialize();

    // Open PKCS#11 session
    System.out.println("Open Session and login with default user");
    CK_SESSION_HANDLE session = Library.C_OpenSession(slotId, CK.CKF_RW_SESSION | CK.CKF_SERIAL_SESSION);
    Library.C_Login(session, CK.CKU_USER, pwd); // Optional if password is null

    // Generate key pair
    System.out.println("Generate key pair");
    int[] keyHandles = Library.C_GenerateKeyPair(session,
      new CK_MECHANISM(CK.CKM_RSA_PKCS_KEY_PAIR_GEN), // RSA generation mechanism
      new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, false),
        new CK_ATTRIBUTE(CK.CKA_MODULUS_BITS, 2048), // RSA key size
      },
      new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
      });
    int pubKey = keyHandles[0];
    int prvKey = keyHandles[1];

    System.out.println("Sign Data");
    Library.C_SignInit(session, new CK_MECHANISM(CK.CKM_SHA256_RSA_PKCS), prvKey);
    byte[] signature = Library.C_Sign(session, dataToSign);

    Library.C_VerifyInit(session, new CK_MECHANISM(CK.CKM_SHA256_RSA_PKCS), pubKey);
    Library.C_Verify(session, dataToSign, signature); // throws if verification failed
    System.out.println("Verify Signature - success");

    // Close PKCS#11 session
    System.out.println("Close PKCS#11 session");
    Library.C_CloseSession(session);
    Library.C_Finalize();
  }
}
