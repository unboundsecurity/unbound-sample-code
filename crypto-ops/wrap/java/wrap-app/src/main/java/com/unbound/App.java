package com.unbound;

import com.dyadicsec.cryptoki.CKR_Exception;
import com.unbound.common.Base64;

import java.io.IOException;
import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.interfaces.RSAPrivateCrtKey;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.stream.Collector;
import java.util.stream.Stream;

public interface App {

    String TEMP_KEY_NAME = "temp-key";

    String getPrefix();

    default String prv(String wrappingKeyName) {
        return String.format("%s-%s-prv", getPrefix(), wrappingKeyName);
    }

    default String pub(String wrappingKeyName) {
        return String.format("%s-%s-pub", getPrefix(), wrappingKeyName);
    }

    default String wrapped(String wrappedKeyName) {
        return String.format("%s-%s", getPrefix(), wrappedKeyName);
    }

    default String imported(String importedKeyName) {
        return String.format("%s-%s", getPrefix(), importedKeyName);
    }

    default RSAPrivateCrtKey pemToPrivateKey(String importedKeyData) throws NoSuchAlgorithmException, InvalidKeySpecException {
        importedKeyData = Stream.of(
                importedKeyData.split("\\r?\\n")).
                filter(s -> !s.startsWith("-----BEGIN")).
                filter(s -> !s.startsWith("-----END"))
                .collect(Collector.of(
                        StringBuilder::new,
                        (b, s) -> b.append(s),
                        (b1, b2) -> b1.append(b2),
                        StringBuilder::toString
                ));

        byte[] encoded = Base64.decode(importedKeyData);
        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(encoded);
        KeyFactory keyFactory = KeyFactory.getInstance("RSA");
        return (RSAPrivateCrtKey) keyFactory.generatePrivate(keySpec);
    }

    void load(String user, String password, String token) throws GeneralSecurityException, IOException, CKR_Exception;

    void generateWrappingKey(String wrappingKeyName) throws GeneralSecurityException, CKR_Exception;

    void generateWrappedKey(String wrappedKeyName) throws GeneralSecurityException, CKR_Exception;

    void wrap(String wrappingKeyName, String wrappedKeyName) throws GeneralSecurityException, CKR_Exception;

    void importKey(String importedKeyName, String importedKeyData) throws GeneralSecurityException, CKR_Exception;

}
