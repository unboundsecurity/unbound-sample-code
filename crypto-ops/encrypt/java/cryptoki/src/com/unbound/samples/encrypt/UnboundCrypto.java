package com.unbound.samples.encrypt;
import java.nio.charset.Charset;

import com.dyadicsec.cryptoki.*;
public class UnboundCrypto {
    CK_SESSION_HANDLE session;
    static final int slotId = 0;

    public static class EncryptedData { 
      byte[] cipher; 
      
      CK_GCM_PARAMS gcmParams;
      
      // The key that was used for encryption.
      AesKey key;
    }

    public class AesKey {   
      String label; String uid; int keyHandle;
      
      public AesKey(String label, String uid, int keyHandle) {
        this.label = label;
        this.uid = uid;
        this.keyHandle = keyHandle;
      }

      public EncryptedData encrypt(String plainData) throws CKR_Exception {
        byte[] iv = Library.C_GenerateRandom(session, 12);
        CK_GCM_PARAMS gcmParams = new CK_GCM_PARAMS();
        gcmParams.pIv = iv;
        gcmParams.ulTagBits = 96;

        Library.C_EncryptInit(session, new CK_MECHANISM(CK.CKM_AES_GCM,gcmParams), keyHandle);
        byte[] encrypted = Library.C_Encrypt(session, plainData.getBytes());
        EncryptedData result = new EncryptedData();
        result.cipher = encrypted;
        result.gcmParams = gcmParams;
        result.key = this;
        return result;
      }

      public void delete() throws CKR_Exception {
        Library.C_DestroyObject(session, keyHandle);
      }

      public byte[] decrypt(EncryptedData encryptedData) throws CKR_Exception {
        Library.C_DecryptInit(session, new CK_MECHANISM(CK.CKM_AES_GCM,
                      encryptedData.gcmParams), keyHandle);
        return Library.C_Decrypt(session, encryptedData.cipher);
      }

      public String getKeyDataStr() {
        return "Key label: '" + label +"', uid:" + uid;
      }

      
    }
    
    public void login(String username, String password) throws CKR_Exception {
      Library.C_Initialize();
      session = Library.C_OpenSession(slotId, CK.CKF_RW_SESSION | CK.CKF_SERIAL_SESSION);
      String userInfo = " { \"username\": \"" + username + "\", \"password\": \"" + password +"\" }";
      Library.C_Login(session, CK.CKU_USER, userInfo.toCharArray());
    }

    public void logout() throws CKR_Exception {
      Library.C_CloseSession(session);
      Library.C_Finalize();
      session = null;
    }

    private AesKey getAttributesByHandle(int keyHandle) throws CKR_Exception {
      CK_ATTRIBUTE[] t = new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.DYCKA_UID),
        new CK_ATTRIBUTE(CK.CKA_ID)
      };
      Library.C_GetAttributeValue(session, keyHandle, t);
      String uid =  Long.toHexString(t[0].getLongValue());
      return new AesKey(new String(t[1].getBytesValue()), uid, keyHandle);
    }

    public AesKey findKeyByLabel(String label) throws CKR_Exception {
      Library.C_FindObjectsInit(session, 
        new CK_ATTRIBUTE[]
          {
            new CK_ATTRIBUTE(CK.CKA_ID, Charset.forName("UTF-8").encode(label).array())
          }
      );
      int[] handles = Library.C_FindObjects(session, 1);
      Library.C_FindObjectsFinal(session);
      if(handles.length < 1) return null;
      return getAttributesByHandle(handles[0]);
    }

    public AesKey createAesKey(String label, boolean exportable) throws CKR_Exception {
      int len = 16; // AES 128 bit
      int aesKey = Library.C_GenerateKey(session, new CK_MECHANISM(CK.CKM_AES_KEY_GEN),
        new CK_ATTRIBUTE[]
        {
          new CK_ATTRIBUTE(CK.CKA_TOKEN, true),
          new CK_ATTRIBUTE(CK.CKA_CLASS, CK.CKO_SECRET_KEY),
          new CK_ATTRIBUTE(CK.CKA_SENSITIVE, !exportable),
          new CK_ATTRIBUTE(CK.CKA_KEY_TYPE, CK.CKK_AES),
          new CK_ATTRIBUTE(CK.CKA_VALUE_LEN, len), 
          new CK_ATTRIBUTE(CK.CKA_ID, Charset.forName("UTF-8").encode(label).array())
        });
      // CK_TOKEN_INFO tokenInfo = Library.C_GetTokenInfo(slotId);

      CK_ATTRIBUTE[] t = new CK_ATTRIBUTE[]
      {
        new CK_ATTRIBUTE(CK.DYCKA_UID),
      };
      Library.C_GetAttributeValue(session, aesKey, t);
      String uid =  Long.toHexString(t[0].getLongValue());
      AesKey key = new AesKey(label, uid, aesKey);
      return key;   
    }
  
  }
