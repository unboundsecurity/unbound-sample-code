import com.dyadicsec.cryptoki.*;

public class ECDSASign
{
  static final byte[] P256_OID = new byte[] {0x06, 0x08, 0x2a, (byte)0x86, 0x48, (byte)0xce, 0x3d, 0x03, 0x01, 0x07};

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
      });
    int pubKey = keyHandles[0];
    int prvKey = keyHandles[1];

    // Sign data
    System.out.println("Sign Data");
    Library.C_SignInit(session, new CK_MECHANISM(CK.CKM_ECDSA_SHA256), prvKey);
    byte[] signature = Library.C_Sign(session, dataToSign);

    // Verify signature
    Library.C_VerifyInit(session, new CK_MECHANISM(CK.CKM_ECDSA_SHA256), pubKey);
    Library.C_Verify(session, dataToSign, signature); // throws if verification failed
    System.out.println("Verify Signature - success");

    // Close PKCS#11 session
    System.out.println("Close PKCS#11 session");
    Library.C_CloseSession(session);
    Library.C_Finalize();
  }
}
