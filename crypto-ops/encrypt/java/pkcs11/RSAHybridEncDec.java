import com.dyadicsec.cryptoki.*;

import java.util.Arrays;

public class RSAHybridEncDec
{
  /***************
   *
   * @param args - if default user password is non-empty, provide it as first arg
   * @throws Exception
   */
  public static void main(String[] args) throws Exception
  {
    char[] pwd = null;
    if (args.length > 0) pwd = args[0].toCharArray();

    final byte[] plainData = "PLAIN DATA".getBytes("UTF-8");
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

    // Generate temporary AES key
    System.out.println("Generate temporary AES key");
    int tempAesKey = Library.C_GenerateKey(session, new CK_MECHANISM(CK.CKM_AES_KEY_GEN),
      new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, false),
        new CK_ATTRIBUTE(CK.CKA_CLASS, CK.CKO_SECRET_KEY),
        new CK_ATTRIBUTE(CK.CKA_KEY_TYPE, CK.CKK_AES),
        new CK_ATTRIBUTE(CK.CKA_VALUE_LEN, 16), // AES 128 bit
      });

    // Wrap AES key
    System.out.println("Wrap AES key");
    CK_RSA_PKCS_OAEP_PARAMS oaepParams = new CK_RSA_PKCS_OAEP_PARAMS();
    oaepParams.hashAlg = CK.CKM_SHA256;
    oaepParams.mgf = CK.CKG_MGF1_SHA256;
    CK_MECHANISM oaepMech = new CK_MECHANISM(CK.CKM_RSA_PKCS_OAEP, oaepParams);
    byte[] wrappedKey = Library.C_WrapKey(session, oaepMech, pubKey, tempAesKey);

    // Generate random IV
    System.out.println("Generate random IV");
    byte[] iv = Library.C_GenerateRandom(session, 16);

    // Encrypt data
    System.out.println("Encrypt data");
    CK_GCM_PARAMS gcmParams = new CK_GCM_PARAMS();
    gcmParams.pIv = iv;
    gcmParams.ulTagBits = 128;
    CK_MECHANISM gcmMech = new CK_MECHANISM(CK.CKM_AES_GCM, gcmParams);
    Library.C_EncryptInit(session, gcmMech, tempAesKey);
    byte[] encrypted = Library.C_Encrypt(session, plainData);

    // Destroy temporary AES key
    System.out.println("Destroy temporary AES key");
    Library.C_DestroyObject(session, tempAesKey);

    // Unwrap AES key
    System.out.println("Unwrap AES key");
    int unwrappedAesKey = Library.C_UnwrapKey(session, oaepMech, prvKey, wrappedKey,
      new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, false),
        new CK_ATTRIBUTE(CK.CKA_CLASS, CK.CKO_SECRET_KEY),
        new CK_ATTRIBUTE(CK.CKA_KEY_TYPE, CK.CKK_AES),
      });

    // Decrypt data
    System.out.println("Decrypt data");
    Library.C_DecryptInit(session, gcmMech, unwrappedAesKey);
    byte[] decrypted = Library.C_Decrypt(session, encrypted);

    assert Arrays.equals(plainData, decrypted);
    System.out.println("Test plain and decrypted data is identical - success");

    // Close PKCS#11 session
    System.out.println("Close PKCS#11 session");
    Library.C_CloseSession(session);
    Library.C_Finalize();
  }
}
