import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import java.security.SecureRandom;
import java.util.Arrays;

public class AESGCMEncDec
{
  public static void run() throws Exception
  {
    final byte[] plainData = "PLAIN DATA".getBytes("UTF-8");

    // Generate AES key
    KeyGenerator gen = KeyGenerator.getInstance("AES", "DYADIC");
    gen.init(128); // AES key size
    SecretKey aesKey = gen.generateKey();

    // Generate IV
    SecureRandom secureRandom = new SecureRandom();
    byte[] iv = new byte[12];
    secureRandom.nextBytes(iv);

    // Encrypt data
    GCMParameterSpec parameterSpec = new GCMParameterSpec(16, iv); // TAG length = 16
    Cipher encryptCipher = Cipher.getInstance("AES/GCM/NoPadding","DYADIC");
    encryptCipher.init(Cipher.ENCRYPT_MODE, aesKey, parameterSpec);
    byte[] encrypted = encryptCipher.doFinal(plainData);

    // Decrypt data
    Cipher decryptCipher = Cipher.getInstance("AES/GCM/NoPadding","DYADIC");
    decryptCipher.init(Cipher.DECRYPT_MODE, aesKey, parameterSpec);
    byte[] decrypted = decryptCipher.doFinal(encrypted);

    assert Arrays.equals(plainData, decrypted);
  }
}
