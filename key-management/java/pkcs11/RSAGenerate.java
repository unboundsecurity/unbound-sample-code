import com.dyadicsec.cryptoki.*;

public class RSAGenerate
{
  public static void run() throws Exception
  {
    int slotId = 0;
    byte[] keyName = "TEST RSA KEY".getBytes("UTF-8");
    Library.C_Initialize();

    // Open PKCS#11 session
    CK_SESSION_HANDLE session = Library.C_OpenSession(slotId, CK.CKF_RW_SESSION | CK.CKF_SERIAL_SESSION);
    Library.C_Login(session, CK.CKU_USER, Main.password); // optional

    // Generate key pair
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
        new CK_ATTRIBUTE(CK.CKA_ID, keyName),
      });
    assert keyHandles.length==2;

    // Find the RSA private key
    Library.C_FindObjectsInit(session, new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
        new CK_ATTRIBUTE(CK.CKA_CLASS, CK.CKO_PRIVATE_KEY),
        new CK_ATTRIBUTE(CK.CKA_KEY_TYPE, CK.CKK_RSA),
        new CK_ATTRIBUTE(CK.CKA_ID, keyName),
      });
    int[] foundKeyHandles = Library.C_FindObjects(session, 1);
    Library.C_FindObjectsFinal(session);
    assert foundKeyHandles.length==1;

    // Close PKCS#11 session
    Library.C_CloseSession(session);
    Library.C_Finalize();
  }
}
