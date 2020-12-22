import com.dyadicsec.cryptoki.*;

import java.util.Arrays;

public class RSAEncDec
{
  public static void run() throws Exception
  {
    final byte[] plainData = "PLAIN DATA".getBytes("UTF-8");
    int slotId = 0;
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
      });
    int pubKey = keyHandles[0];
    int prvKey = keyHandles[1];

    CK_RSA_PKCS_OAEP_PARAMS oaepParams = new CK_RSA_PKCS_OAEP_PARAMS();
    oaepParams.hashAlg = CK.CKM_SHA256;
    oaepParams.mgf = CK.CKG_MGF1_SHA256;
    CK_MECHANISM mech = new CK_MECHANISM(CK.CKM_RSA_PKCS_OAEP, oaepParams);

    Library.C_EncryptInit(session, mech, pubKey);
    byte[] encrypted = Library.C_Encrypt(session, plainData);

    Library.C_DecryptInit(session, mech, prvKey);
    byte[] decrypted = Library.C_Decrypt(session, encrypted);

    assert Arrays.equals(plainData, decrypted);

    // Close PKCS#11 session
    Library.C_CloseSession(session);
    Library.C_Finalize();
  }
}
