import com.dyadicsec.cryptoki.*;

public class ExamplePKCS11_AESGCM
{
  public static void main(String [] args) throws Throwable
  {
    Library.C_Initialize();
    CK_SESSION_HANDLE hSession = Library.C_OpenSession(0, CK.CKF_RW_SESSION | CK.CKF_SERIAL_SESSION);

    int aesKeyHandle = Library.C_GenerateKey(hSession, new CK_MECHANISM(CK.CKM_AES_KEY_GEN),
      new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
        new CK_ATTRIBUTE(CK.CKA_CLASS, CK.CKO_SECRET_KEY),
        new CK_ATTRIBUTE(CK.CKA_KEY_TYPE, CK.CKK_AES),
        new CK_ATTRIBUTE(CK.CKA_VALUE_LEN, 32),
      });

    String plainData = "TEST PLAIN DATA";
    byte[] iv = Library.C_GenerateRandom(hSession, 12);

    CK_GCM_PARAMS gcmParams = new CK_GCM_PARAMS();
    gcmParams.pIv = iv;
    gcmParams.ulTagBits = 96;

    Library.C_EncryptInit(hSession, new CK_MECHANISM(CK.CKM_AES_GCM,gcmParams), aesKeyHandle);
    byte[] encrypted = Library.C_Encrypt(hSession, plainData.getBytes());

    Library.C_DecryptInit(hSession, new CK_MECHANISM(CK.CKM_AES_GCM,gcmParams), aesKeyHandle);

    byte[] decrypted;
    try
    {
      decrypted = Library.C_Decrypt(hSession, encrypted);
    }
    catch (CKR_Exception e)
    {
      throw new SecurityException("GCM decryption failure", e);
    }

    String test = new String(decrypted);

    if (!test.equals(plainData)) throw new AssertionError("Decrypted sata mismatch");
  }
}
