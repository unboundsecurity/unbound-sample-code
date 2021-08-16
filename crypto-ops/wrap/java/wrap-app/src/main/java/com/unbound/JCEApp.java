package com.unbound;

import com.dyadicsec.cryptoki.CKR_Exception;
import com.unbound.provider.KeyGenSpec;
import com.unbound.provider.KeyParameters;
import com.unbound.provider.UBCryptoProvider;
import com.unbound.provider.UBKeyStoreEntry;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import java.io.IOException;
import java.security.*;
import java.security.spec.AlgorithmParameterSpec;
import java.util.Arrays;

import static javax.crypto.Cipher.SECRET_KEY;

public class JCEApp implements App {

    private static final String UB_PROVIDER_NAME = "UNBOUND";
    private static final String UB_PROVIDER_TYPE = "PKCS11";
    private static final String prefix = "JCE";

    private static final String RSA_ECB_OAEPPADDING = "RSA/ECB/OAEPPadding";
    private static final String AES_ECB_NO_PADDING = "AES/ECB/NoPadding";
    private static KeyStore ubKS;

    @Override
    public String getPrefix() {
        return prefix;
    }

    @Override
    public void load(String user, String password,String token) throws GeneralSecurityException, IOException {
        System.out.println("Loading Unbound KeyStore");

        Provider provider = new UBCryptoProvider();
        Security.addProvider(provider);
        ubKS = KeyStore.getInstance(UB_PROVIDER_TYPE, UB_PROVIDER_NAME);
        if (token != null) {
            String auth = String.format("{\"token\":\"%s\"}", token);
            System.out.println(auth);
            ubKS.load(null, auth.toCharArray());
        } else if (user != null && password != null) {
            String auth = String.format("{\"username\":\"%s\", \"password\":\"%s\"}", user, password);
            ubKS.load(null, auth.toCharArray());
        } else {
            ubKS.load(null);
        }

        UBCryptoProvider.allowPrivateKeyWithoutCertificate(true);
    }

    @Override
    public void generateWrappingKey(String wrappingKeyName) throws GeneralSecurityException {
        System.out.println("Generate RSA Private Key");
        KeyParameters privateKeyParams = new KeyParameters();
        privateKeyParams.setName(prv(wrappingKeyName));
        KeyPairGenerator gen = KeyPairGenerator.getInstance("RSA", UB_PROVIDER_NAME);
        gen.initialize(new KeyGenSpec(2_048, privateKeyParams));
        KeyPair keyPair = gen.generateKeyPair();

        System.out.println("Importing RSA public Key");
        KeyParameters publicKeyParams = new KeyParameters();
        publicKeyParams.setTrusted(true);
        UBKeyStoreEntry keyEntry = new UBKeyStoreEntry(keyPair.getPublic(), publicKeyParams);
        ubKS.setEntry(pub(wrappingKeyName), keyEntry, null);
    }

    @Override
    public void generateWrappedKey(String wrappedKeyName) throws GeneralSecurityException {
        System.out.println("Generate Key For Wrapping");
        KeyGenerator generator = KeyGenerator.getInstance("AES", UB_PROVIDER_NAME);
        KeyParameters keyParams = new KeyParameters();
        keyParams.setExportProtection(KeyParameters.EXPORT_WRAP_WITH_TRUSTED);
        keyParams.setName(wrapped(wrappedKeyName));
        generator.init(new KeyGenSpec(256, keyParams));
        generator.generateKey();
    }

    @Override
    public void wrap(String wrappingKeyName, String wrappedKeyName) throws GeneralSecurityException {
        Key wrappingKey = ubKS.getKey(pub(wrappingKeyName), null);
        Key unWrappingKey = ubKS.getKey(prv(wrappingKeyName), null);
        Key wrappedKey = ubKS.getKey(wrapped(wrappedKeyName), null);

        if(wrappingKey == null) throw new InvalidKeyException("Wrapping Key not found");
        if(unWrappingKey == null) throw new InvalidKeyException("UnWrapping Key not found");
        if(wrappedKey == null) throw new InvalidKeyException("Wrapped Key not found");

        System.out.println("Wrapping");
        Cipher wrappingCipher = Cipher.getInstance(RSA_ECB_OAEPPADDING, UB_PROVIDER_NAME);
        wrappingCipher.init(Cipher.WRAP_MODE, wrappingKey, (AlgorithmParameterSpec) null, SecureRandom.getInstanceStrong());
        byte[] encrypted = wrappingCipher.wrap(wrappedKey);

        System.out.println("UnWrapping");
        Cipher unWrappingCipher = Cipher.getInstance(RSA_ECB_OAEPPADDING, UB_PROVIDER_NAME);
        unWrappingCipher.init(Cipher.UNWRAP_MODE, unWrappingKey, (AlgorithmParameterSpec) null, SecureRandom.getInstanceStrong());
        Key tempKey = unWrappingCipher.unwrap(encrypted, "AES", SECRET_KEY);

        ubKS.setKeyEntry(TEMP_KEY_NAME, tempKey, null, null);
        tempKey = ubKS.getKey(TEMP_KEY_NAME, null);

        Cipher encryptionCipher1 = Cipher.getInstance(AES_ECB_NO_PADDING, UB_PROVIDER_NAME);
        encryptionCipher1.init(Cipher.ENCRYPT_MODE, wrappedKey);
        byte[] e1 = encryptionCipher1.update(new byte[32]);

        Cipher encryptionCipher2 = Cipher.getInstance(AES_ECB_NO_PADDING, UB_PROVIDER_NAME);
        encryptionCipher2.init(Cipher.ENCRYPT_MODE, tempKey);
        byte[] e2 = encryptionCipher2.doFinal(new byte[32]);

        boolean ok = Arrays.equals(e1, e2);
        System.out.println(ok ? "Succeeded" : "Failed");

        ubKS.deleteEntry(TEMP_KEY_NAME);
    }

    @Override
    public void importKey(String importedKeyName, String importedKeyData) throws GeneralSecurityException, CKR_Exception {
        System.out.println("Import key");

        PrivateKey privateKey = pemToPrivateKey(importedKeyData);
        UBKeyStoreEntry keyEntry = new UBKeyStoreEntry(privateKey);
        ubKS.setEntry(imported(importedKeyName),keyEntry,null);
    }
}
