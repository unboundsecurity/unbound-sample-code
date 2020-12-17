import com.dyadicsec.cryptoki.*;
import sun.security.provider.SecureRandom;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

import static com.dyadicsec.cryptoki.CK.*;
import static com.dyadicsec.cryptoki.CK.CKA_KEY_TYPE;
import static com.dyadicsec.cryptoki.CK.CKK_EC;

public class Main {

    static Map curves = new HashMap<String, byte[]>();

    static String SECP_256_R1 = "secp256r1";
    static String SECP_384_R1 = "secp384r1";
    static String SECP_521_R1 = "secp521r1";
    static String SECP_256_K1 = "secp256k1";

    static {
        curves.put(SECP_256_R1, new byte[]{(byte) 0x06, (byte) 0x08, (byte) 0x2a, (byte) 0x86, (byte) 0x48, (byte) 0xce, (byte) 0x3d, (byte) 0x03, (byte) 0x01, (byte) 0x07});
        curves.put(SECP_384_R1, new byte[]{(byte) 0x06, (byte) 0x05, (byte) 0x2b, (byte) 0x81, (byte) 0x04, (byte) 0x00, (byte) 0x22});
        curves.put(SECP_521_R1, new byte[]{(byte) 0x06, (byte) 0x05, (byte) 0x2b, (byte) 0x81, (byte) 0x04, (byte) 0x00, (byte) 0x23});
        curves.put(SECP_256_K1, new byte[]{(byte) 0x06, (byte) 0x05, (byte) 0x2b, (byte) 0x81, (byte) 0x04, (byte) 0x00, (byte) 0x0a});
    }

    /**
     * open PKSC11 session
     * @return
     * @throws CKR_Exception
     */
    public static CK_SESSION_HANDLE openSession(int slotId) throws CKR_Exception {
        Library.C_Initialize();
        return Library.C_OpenSession(slotId, CK.CKF_RW_SESSION | CK.CKF_SERIAL_SESSION);
    }

    /**
     * close PKSC11 session
     * @param hSession
     * @throws CKR_Exception
     */
    public static void closeSession(CK_SESSION_HANDLE hSession) throws CKR_Exception {
        Library.C_CloseSession(hSession);
    }

    /**
     * get key by id
     * @param hSession
     * @param id
     * @return
     * @throws CKR_Exception
     */
    public static int findKey(CK_SESSION_HANDLE hSession, String id) throws CKR_Exception {
        CK_ATTRIBUTE[] tAttr =
                {
                        new CK_ATTRIBUTE(CKA_ID, id.getBytes()),
                };

        Library.C_FindObjectsInit(hSession, tAttr);
        int hPrvKey = Library.C_FindObjects(hSession, 1)[0];
        Library.C_FindObjectsFinal(hSession);
        return hPrvKey;
    }

    /**
     * delete key by handle
     * @param hSession
     * @param hPrvKey
     * @throws CKR_Exception
     */
    public static void deleteKey(CK_SESSION_HANDLE hSession, int hPrvKey) throws CKR_Exception {
        if (hPrvKey != 0) Library.C_DestroyObject(hSession, hPrvKey);
    }

    /**
     * generate ECDSA private key
     * @param hSession
     * @param id
     * @return
     * @throws CKR_Exception
     */
    public static int generateEcdsa(CK_SESSION_HANDLE hSession, String id) throws CKR_Exception {

        CK_ATTRIBUTE[] tPrv =
                {
                        new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
                        new CK_ATTRIBUTE(CKA_CLASS, CKO_PRIVATE_KEY),
                        new CK_ATTRIBUTE(CKA_KEY_TYPE, CKK_EC),
                        new CK_ATTRIBUTE(CKA_SIGN, true),
                        new CK_ATTRIBUTE(CKA_ID, id.getBytes()),
                };
        CK_ATTRIBUTE[] tPub =
                {
                        new CK_ATTRIBUTE(CK.CKA_TOKEN, false),
                        new CK_ATTRIBUTE(CKA_CLASS, CKO_PUBLIC_KEY),
                        new CK_ATTRIBUTE(CKA_KEY_TYPE, CKK_EC),
                        new CK_ATTRIBUTE(CK.CKA_EC_PARAMS, curves.get(SECP_256_R1)),
                };

        int[] hKeys = Library.C_GenerateKeyPair(hSession, new CK_MECHANISM(CK.CKM_EC_KEY_PAIR_GEN), tPub, tPrv);
        deleteKey(hSession, hKeys[0]); //delete the local public key
        return hKeys[1];
    }

    /**
     * ECDSA sign
     * @param hSession
     * @param hPrvKey
     * @param data
     * @return
     * @throws CKR_Exception
     */
    public static byte[] signEcdsa(CK_SESSION_HANDLE hSession, int hPrvKey, byte[] data) throws CKR_Exception {
        Library.C_SignInit(hSession, new CK_MECHANISM(CK.CKM_ECDSA_SHA256), hPrvKey);
        return Library.C_Sign(hSession, data);
    }

    /**
     * verify ECDSA signature
     * @param hSession
     * @param hPrvKey
     * @param data
     * @param signature
     * @throws CKR_Exception
     */
    public static void verifyEcdsa(CK_SESSION_HANDLE hSession, int hPrvKey, byte[] data, byte[] signature) throws CKR_Exception {
        CK_ATTRIBUTE[] tAttr =
                {
                        new CK_ATTRIBUTE(CKA_EC_POINT),
                        new CK_ATTRIBUTE(CKA_EC_PARAMS),
                };

        Library.C_GetAttributeValue(hSession, hPrvKey, tAttr);

        tAttr = new CK_ATTRIBUTE[]{
                tAttr[0],
                tAttr[1],
                new CK_ATTRIBUTE(CK.CKA_TOKEN, false),
                new CK_ATTRIBUTE(CKA_CLASS, CKO_PUBLIC_KEY),
                new CK_ATTRIBUTE(CKA_KEY_TYPE, CKK_EC),
        };

        int hPubKey = Library.C_CreateObject(hSession, tAttr); // crate a local key for the verification
        Library.C_VerifyInit(hSession, new CK_MECHANISM(CK.CKM_ECDSA_SHA256), hPubKey);
        Library.C_Verify(hSession, data, signature);
        deleteKey(hSession, hPubKey);
    }

    public static void main(String[] args) throws CKR_Exception {
        int slotId =0;
        String id = "ecdsa-private-key";

        CK_SESSION_HANDLE hSession = new CK_SESSION_HANDLE();
        int hPrvKey = 0;

        byte[] data = "test".getBytes();

        try {
            hSession = openSession(slotId);
            generateEcdsa(hSession, id);
            hPrvKey = findKey(hSession, id);

            byte[] signature = signEcdsa(hSession, hPrvKey, data);
            verifyEcdsa(hSession, hPrvKey, data, signature);
        } finally {
            deleteKey(hSession, hPrvKey);
            closeSession(hSession);
        }
    }
}
