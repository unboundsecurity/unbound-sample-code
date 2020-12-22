#include "sample_pkcs11.h"


void rsa_hybrid_encdec()
{
  initialize_pkcs11();

  CK_SLOT_ID slot_id = 0; // default slot ID
  CK_SESSION_HANDLE session = 0;
  CK_RV rv;
  const char plain_data[] = "plain data";
  
  // Open PKCS#11 session
  rv = pkcs11_funcs->C_OpenSession(slot_id, CKF_SERIAL_SESSION | CKF_RW_SESSION, 0, 0, &session);
  if (rv) leave ("C_OpenSession failed", rv);

#ifdef PKCS11_PASSWORD
  rv = pkcs11_funcs->C_Login(session, CKU_USER, (CK_CHAR_PTR)PKCS11_PASSWORD, (CK_ULONG)strlen(PKCS11_PASSWORD));
  if (rv) leave ("C_Login failed", rv);
#endif

  // RSA generation mechanism
  CK_MECHANISM rsa_gen = {CKM_RSA_PKCS_KEY_PAIR_GEN, NULL, 0};

  // RSA key size
  CK_ULONG modulus_bits = 2048;

  CK_BBOOL ck_false = CK_FALSE;
  CK_BBOOL ck_true  = CK_TRUE;

  CK_ATTRIBUTE pub_template[] = 
  {
    {CKA_TOKEN,        &ck_false,     sizeof(CK_BBOOL)},
    {CKA_MODULUS_BITS, &modulus_bits, sizeof(CK_ULONG)},
  };

  CK_ATTRIBUTE prv_template[] = 
  {
    {CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
  };

  // Generate key pair
  CK_OBJECT_HANDLE pub_key = 0, prv_key = 0;
  rv = pkcs11_funcs->C_GenerateKeyPair(session, &rsa_gen, 
    pub_template, sizeof(pub_template)/sizeof(CK_ATTRIBUTE), 
    prv_template, sizeof(prv_template)/sizeof(CK_ATTRIBUTE), 
    &pub_key, &prv_key);
  if (rv) leave ("C_GenerateKeyPair failed", rv);

  // Generate temporary AES key
  CK_ULONG cko_secret_key = CKO_SECRET_KEY;
  CK_ULONG ckk_aes = CKK_AES;
  CK_ULONG aes_key_length = 16;
  CK_OBJECT_HANDLE temp_aes_key;
  CK_MECHANISM gen_aes = {CKM_AES_KEY_GEN, NULL, 0};
  CK_ATTRIBUTE aes_gen_template[] = 
  {
    {CKA_TOKEN,      &ck_false,       sizeof(CK_BBOOL)},
    {CKA_CLASS,      &cko_secret_key, sizeof(CK_ULONG)},
    {CKA_KEY_TYPE,   &ckk_aes,        sizeof(CK_ULONG)},
    {CKA_VALUE_LEN,  &aes_key_length, sizeof(CK_ULONG)},
  };
  rv = pkcs11_funcs->C_GenerateKey(session, &gen_aes, aes_gen_template, sizeof(aes_gen_template)/sizeof(CK_ATTRIBUTE), &temp_aes_key);
  if (rv) leave ("C_GenerateKey failed", rv); 

  // Wrap AES key
  CK_RSA_PKCS_OAEP_PARAMS oaep_params = { CKM_SHA256, CKG_MGF1_SHA256, 0, NULL, 0};
  CK_MECHANISM oaep_mech = {CKM_RSA_PKCS_OAEP, &oaep_params, sizeof(CK_RSA_PKCS_OAEP_PARAMS)};
  CK_ULONG wrapped_key_len;
  rv = pkcs11_funcs->C_WrapKey(session, &oaep_mech, pub_key, temp_aes_key, NULL, &wrapped_key_len);
  if (rv) leave ("C_WrapKey failed", rv); 
  CK_BYTE_PTR wrapped_key = new CK_BYTE[wrapped_key_len];
  rv = pkcs11_funcs->C_WrapKey(session, &oaep_mech, pub_key, temp_aes_key, wrapped_key, &wrapped_key_len);
  if (rv) leave ("C_WrapKey failed", rv); 

  // Generate random IV
  CK_BYTE iv[16];
  rv = pkcs11_funcs->C_GenerateRandom(session, iv, sizeof(iv));
  if (rv) leave ("C_GenerateRandom failed", rv); 

  // Encrypt data
  CK_GCM_PARAMS gcm_params = {iv, sizeof(iv), NULL, 0, 128};
  CK_MECHANISM gcm_mech = {CKM_AES_GCM, &gcm_params, sizeof(gcm_params)};

  rv = pkcs11_funcs->C_EncryptInit(session, &gcm_mech, temp_aes_key);
  if (rv) leave ("C_EncryptInit failed", rv);

  CK_ULONG enc_len;
  rv = pkcs11_funcs->C_Encrypt(session, (CK_BYTE_PTR)plain_data, (CK_ULONG)sizeof(plain_data), NULL, &enc_len);
  if (rv) leave ("C_Encrypt failed", rv);

  CK_BYTE_PTR encrypted = new CK_BYTE[enc_len];
  rv = pkcs11_funcs->C_Encrypt(session, (CK_BYTE_PTR)plain_data, (CK_ULONG)sizeof(plain_data), encrypted, &enc_len);
  if (rv) leave ("C_Encrypt failed", rv);

  // Destroy temporary AES key
  rv = pkcs11_funcs->C_DestroyObject(session, temp_aes_key);
  if (rv) leave ("C_DestroyObject failed", rv);

  // Unwrap AES key
  CK_ATTRIBUTE aes_unwrap_template[] = 
  {
    {CKA_TOKEN,      &ck_false,       sizeof(CK_BBOOL)},
    {CKA_CLASS,      &cko_secret_key, sizeof(CK_ULONG)},
    {CKA_KEY_TYPE,   &ckk_aes,        sizeof(CK_ULONG)},
  };
  rv = pkcs11_funcs->C_UnwrapKey(session, &oaep_mech, prv_key, wrapped_key, wrapped_key_len, aes_unwrap_template, sizeof(aes_unwrap_template)/sizeof(CK_ATTRIBUTE), &temp_aes_key);
  if (rv) leave ("C_UnwrapKey failed", rv);
  
  delete[] wrapped_key;


  // Decrypt data
  rv = pkcs11_funcs->C_DecryptInit(session, &gcm_mech, temp_aes_key);
  if (rv) leave ("C_DecryptInit failed", rv);

  CK_ULONG dec_len;
  rv = pkcs11_funcs->C_Decrypt(session, encrypted, enc_len, NULL, &dec_len);
  if (rv) leave ("C_Decrypt failed", rv);

  CK_BYTE_PTR decrypted = new CK_BYTE[dec_len];
  rv = pkcs11_funcs->C_Decrypt(session, encrypted, enc_len, decrypted, &dec_len);
  if (rv) leave ("C_Decrypt failed", rv);


  if (dec_len!=sizeof(plain_data)) leave("Decrypted/plain length mismatch");
  if (0!=memcmp(plain_data, decrypted, dec_len)) leave("Decrypted/plain data mismatch");

  delete[] encrypted;
  delete[] decrypted;

  // Close PKCS#11 session
  pkcs11_funcs->C_CloseSession(session);

  uninitialize_pkcs11();
}