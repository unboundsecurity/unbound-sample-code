import com.dyadicsec.cryptoki.*;

import java.util.Arrays;

public class AESGCMEncDec
{
  public static void run() throws Exception
  {
    final byte[] plainData = "PLAIN DATA".getBytes("UTF-8");
    int slotId = 0;
    Library.C_Initialize();

    // Open PKCS#11 session
    CK_SESSION_HANDLE session = Library.C_OpenSession(slotId, CK.CKF_RW_SESSION | CK.CKF_SERIAL_SESSION);
    Library.C_Login(session, CK.CKU_USER, Main.password); // optional

    // Generate AES key
    int aesKey = Library.C_GenerateKey(session, new CK_MECHANISM(CK.CKM_AES_KEY_GEN),
      new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, false),
        new CK_ATTRIBUTE(CK.CKA_CLASS, CK.CKO_SECRET_KEY),
        new CK_ATTRIBUTE(CK.CKA_KEY_TYPE, CK.CKK_AES),
        new CK_ATTRIBUTE(CK.CKA_VALUE_LEN, 16), // AES 128 bit
      });

    // Generate random IV
    byte[] iv = Library.C_GenerateRandom(session, 16);

    // Encrypt data
    CK_GCM_PARAMS gcmParams = new CK_GCM_PARAMS();
    gcmParams.pIv = iv;
    gcmParams.ulTagBits = 128;
    CK_MECHANISM gcmMech = new CK_MECHANISM(CK.CKM_AES_GCM, gcmParams);
    Library.C_EncryptInit(session, gcmMech, aesKey);
    byte[] encrypted = Library.C_Encrypt(session, plainData);

    // Decrypt data
    Library.C_DecryptInit(session, gcmMech, aesKey);
    byte[] decrypted = Library.C_Decrypt(session, encrypted);

    assert Arrays.equals(plainData, decrypted);

    // Close PKCS#11 session
    Library.C_CloseSession(session);
    Library.C_Finalize();
  }
}