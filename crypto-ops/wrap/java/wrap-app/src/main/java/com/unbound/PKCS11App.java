package com.unbound;

import com.dyadicsec.cryptoki.*;
import com.unbound.common.Converter;

import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.InvalidKeyException;
import java.security.interfaces.RSAPrivateCrtKey;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Locale;

import static com.dyadicsec.cryptoki.CK.*;

public class PKCS11App implements App {

    private static final String prefix = "PKCS11";
    private CK_SESSION_HANDLE session;

    @Override
    public String getPrefix() {
        return prefix;
    }

    @Override
    public void load(String user, String password,String token) throws CKR_Exception {
        System.out.println("Loading PKCS11");

        // for token - if u ==so

        int ckaUser = "so".equals(user) ? CK.CKU_SO : CK.CKU_USER;

        char[] auth =
                (token != null ? String.format("{\"token\":\"%s\"}", token).toCharArray() :
                        (password == null ? null : password.toCharArray()));

        int slotId = 0;
        Library.C_Initialize();

        session = Library.C_OpenSession(slotId, CK.CKF_RW_SESSION | CK.CKF_SERIAL_SESSION);
        Library.C_Login(session, ckaUser, auth);
    }

    @Override
    public void generateWrappingKey(String wrappingKeyName) throws CKR_Exception {
        System.out.println("Generate Key Pair");

        Library.C_GenerateKeyPair(session,
                new CK_MECHANISM(CK.CKM_RSA_PKCS_KEY_PAIR_GEN), // RSA generation mechanism
                new CK_ATTRIBUTE[]
                        {
                                new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
                                new CK_ATTRIBUTE(CK.CKA_MODULUS_BITS, 2048),
                                new CK_ATTRIBUTE(CK.CKA_TRUSTED, true),
                                new CK_ATTRIBUTE(CK.CKA_ID, pub(wrappingKeyName).getBytes(StandardCharsets.UTF_8))
                        },
                new CK_ATTRIBUTE[]
                        {
                                new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
                                new CK_ATTRIBUTE(CK.CKA_ID, prv(wrappingKeyName).getBytes(StandardCharsets.UTF_8))
                        });
    }

    @Override
    public void generateWrappedKey(String wrappedKeyName) throws CKR_Exception {
        System.out.println("Generate Key For Wrapping");

        Library.C_GenerateKey(session, new CK_MECHANISM(CK.CKM_AES_KEY_GEN),
                new CK_ATTRIBUTE[]
                        {
                                new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
                                new CK_ATTRIBUTE(CK.CKA_CLASS, CK.CKO_SECRET_KEY),
                                new CK_ATTRIBUTE(CK.CKA_KEY_TYPE, CK.CKK_AES),
                                new CK_ATTRIBUTE(CK.CKA_VALUE_LEN, 32),
                                new CK_ATTRIBUTE(CK.CKA_SENSITIVE, true),
                                new CK_ATTRIBUTE(CK.CKA_EXTRACTABLE, true),
                                new CK_ATTRIBUTE(CK.CKA_WRAP_WITH_TRUSTED, true),
                                new CK_ATTRIBUTE(CK.CKA_ENCRYPT, true),
                                new CK_ATTRIBUTE(CK.CKA_ID, wrapped(wrappedKeyName).getBytes(StandardCharsets.UTF_8))
                        });
    }

    @Override
    public void wrap(String wrappingKeyName, String wrappedKeyName) throws CKR_Exception , GeneralSecurityException {
        int publicKey = findKey(session, new CK_ATTRIBUTE[]
                {
                        new CK_ATTRIBUTE(CK.CKA_ID, pub(wrappingKeyName).getBytes(StandardCharsets.UTF_8))
                });

        int privateKey = findKey(session, new CK_ATTRIBUTE[]
                {
                        new CK_ATTRIBUTE(CK.CKA_ID, prv(wrappingKeyName).getBytes(StandardCharsets.UTF_8))
                });

        int wrappedKey = findKey(session, new CK_ATTRIBUTE[]
                {
                        new CK_ATTRIBUTE(CK.CKA_ID, wrapped(wrappedKeyName).getBytes(StandardCharsets.UTF_8))
                });

        CK_RSA_PKCS_OAEP_PARAMS oaepParams = new CK_RSA_PKCS_OAEP_PARAMS();
        oaepParams.hashAlg = CK.CKM_SHA256;
        oaepParams.mgf = CK.CKG_MGF1_SHA256;

        System.out.println("Wrapping");
        byte[] encrypted = Library.C_WrapKey(session,
                new CK_MECHANISM(CK.CKM_RSA_PKCS_OAEP, oaepParams),
                publicKey,
                wrappedKey);

        System.out.println("UnWrapping");
        int tempKey = Library.C_UnwrapKey(session,
                new CK_MECHANISM(CK.CKM_RSA_PKCS_OAEP, oaepParams),
                privateKey,
                encrypted,
                new CK_ATTRIBUTE[]
                        {
                                new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
                                new CK_ATTRIBUTE(CK.CKA_CLASS, CK.CKO_SECRET_KEY),
                                new CK_ATTRIBUTE(CK.CKA_KEY_TYPE, CK.CKK_AES),
                                new CK_ATTRIBUTE(CK.CKA_ENCRYPT, true),
                                new CK_ATTRIBUTE(CK.CKA_ID, TEMP_KEY_NAME.getBytes(StandardCharsets.UTF_8))
                        });

        Library.C_EncryptInit(session,
                new CK_MECHANISM(CK.CKM_AES_ECB),
                wrappedKey);

        byte[] e1 = Library.C_Encrypt(session, new byte[32]);

        Library.C_EncryptInit(session,
                new CK_MECHANISM(CK.CKM_AES_ECB),
                tempKey);

        byte[] e2 = Library.C_Encrypt(session, new byte[32]);

        boolean ok = Arrays.equals(e1, e2);
        System.out.println(ok ? "Succeeded" : "Failed");

        Library.C_DestroyObject(session,tempKey);
    }

    @Override
    public void importKey(String importedKeyName, String importedKeyData) throws GeneralSecurityException, CKR_Exception {
        System.out.println("Import key");

        RSAPrivateCrtKey privateKey = pemToPrivateKey(importedKeyData);

        int keySize = privateKey.getModulus().bitLength() / 8;

        ArrayList<CK_ATTRIBUTE> attributes = new ArrayList();

        attributes.add(new CK_ATTRIBUTE(CKA_TOKEN, true));
        attributes.add(new CK_ATTRIBUTE(CKA_CLASS, CKO_PRIVATE_KEY));
        attributes.add(new CK_ATTRIBUTE(CKA_KEY_TYPE, CKK_RSA));
        attributes.add(new CK_ATTRIBUTE(CKA_MODULUS, Converter.bigNumToBin(privateKey.getModulus(), keySize)));
        attributes.add(new CK_ATTRIBUTE(CKA_PUBLIC_EXPONENT, Converter.bigNumToBin(privateKey.getPublicExponent())));
        attributes.add(new CK_ATTRIBUTE(CKA_PRIVATE_EXPONENT, Converter.bigNumToBin(privateKey.getPrivateExponent(), keySize)));
        attributes.add(new CK_ATTRIBUTE(CKA_PRIME_1, Converter.bigNumToBin(privateKey.getPrimeP(), keySize/2)));
        attributes.add(new CK_ATTRIBUTE(CKA_PRIME_2, Converter.bigNumToBin(privateKey.getPrimeQ(), keySize/2)));
        attributes.add(new CK_ATTRIBUTE(CKA_EXPONENT_1, Converter.bigNumToBin(privateKey.getPrimeExponentP(), keySize/2)));
        attributes.add(new CK_ATTRIBUTE(CKA_EXPONENT_2, Converter.bigNumToBin(privateKey.getPrimeExponentQ(), keySize/2)));
        attributes.add(new CK_ATTRIBUTE(CKA_COEFFICIENT, Converter.bigNumToBin(privateKey.getCrtCoefficient(), keySize/2)));
        attributes.add(new CK_ATTRIBUTE(CK.CKA_ID, imported(importedKeyName).getBytes(StandardCharsets.UTF_8)));

        Library.C_CreateObject(session, getAttrs(attributes));

    }

    static int findKey(CK_SESSION_HANDLE hSession, CK_ATTRIBUTE[] t) throws CKR_Exception, InvalidKeyException {
        Library.C_FindObjectsInit(hSession, t);
        int[] handles = Library.C_FindObjects(hSession, 1);
        Library.C_FindObjectsFinal(hSession);
        if (handles.length == 0) throw new InvalidKeyException("Key not found");
        return handles[0];
    }

    static CK_ATTRIBUTE[] getAttrs(ArrayList<CK_ATTRIBUTE> t)
    {
        CK_ATTRIBUTE[] attrs = new CK_ATTRIBUTE[t.size()];
        t.toArray(attrs);
        return attrs;
    }
}
