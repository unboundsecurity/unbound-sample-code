package com.unboundtech.crypto_ops.encryption.provider;

import com.unbound.provider.UBCryptoProvider;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import java.security.KeyStore;
import java.security.Provider;
import java.security.SecureRandom;
import java.security.Security;
import java.util.Arrays;

public class AESGCMEncDec
{
  /******
   *
   * @param args - optional. args[0] : username, args[1] : password
   *
   * @throws Exception
   */
  public static void main(String[] args) throws Exception
  {
    String user = "user"; //optional. default is "user"
    if (args.length > 0) user = args[0];
    String pwd = null; //optional. default is blank
    if (args.length > 1) pwd = args[1];


    // Add Provider to Java Security
    System.out.println("Add Provider to Java Security");
    Provider provider = new UBCryptoProvider();
    Security.addProvider(provider);

    // Login
    System.out.println("Login");
    KeyStore keyStore = KeyStore.getInstance("PKCS11", "UNBOUND");
    String auth = String.format("{\"username\":\"%s\", \"password\":\"%s\"}", user, pwd);
    keyStore.load(null, pwd.toCharArray());

    // Generate AES key
    System.out.println("Generate AES key");
    KeyGenerator gen = KeyGenerator.getInstance("AES", "UNBOUND");
    gen.init(128); // AES key size

    SecretKey aesKey = gen.generateKey();

    // Generate IV
    System.out.println("Generate IV");
    SecureRandom secureRandom = new SecureRandom();
    byte[] iv = new byte[12];
    secureRandom.nextBytes(iv);

    final byte[] plainData = "PLAIN DATA".getBytes("UTF-8");

    // Encrypt data
    System.out.println("Encrypt data");
    GCMParameterSpec parameterSpec = new GCMParameterSpec(16, iv); // TAG length = 16
    Cipher encryptCipher = Cipher.getInstance("AES/GCM/NoPadding","DYADIC");
    encryptCipher.init(Cipher.ENCRYPT_MODE, aesKey, parameterSpec);
    byte[] encrypted = encryptCipher.doFinal(plainData);

    // Decrypt data
    System.out.println("Decrypt data");
    Cipher decryptCipher = Cipher.getInstance("AES/GCM/NoPadding","DYADIC");
    decryptCipher.init(Cipher.DECRYPT_MODE, aesKey, parameterSpec);
    byte[] decrypted = decryptCipher.doFinal(encrypted);

    // check decryption
    assert Arrays.equals(plainData, decrypted);
    System.out.println("Check plain data is identical to decrypted - success");
  }
}
