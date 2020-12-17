#include "sample_pkcs11.h"


static const char rsa_sample_key_name[]   = "RSASAMPLEKEYNAME";
static const int  rsa_sample_key_name_len = sizeof(rsa_sample_key_name)-1; // exclude zero termintaor

static const char data_to_sign[] = "data to sign";


void find_and_verify(CK_BYTE_PTR signature, CK_ULONG signature_len)
{
  CK_SLOT_ID slot_id = 0; // default slot ID
  CK_SESSION_HANDLE session = 0;
  CK_RV rv;
  
  // Open PKCS#11 session
  rv = pkcs11_funcs->C_OpenSession(slot_id, CKF_SERIAL_SESSION | CKF_RW_SESSION, 0, 0, &session);
  if (rv) leave ("C_OpenSession failed", rv);

  CK_BBOOL ck_false = CK_FALSE;
  CK_BBOOL ck_true  = CK_TRUE;
  CK_ULONG ckk_rsa  = CKK_RSA;
  CK_ULONG cko_public_key = CKO_PUBLIC_KEY;
  CK_ULONG cko_private_key = CKO_PRIVATE_KEY;

  CK_ATTRIBUTE find_template[] = 
  {
    {CKA_TOKEN,    &ck_true,                          sizeof(CK_BBOOL)},
    {CKA_CLASS,    &cko_private_key,                  sizeof(CK_ULONG)},
    {CKA_KEY_TYPE, &ckk_rsa,                          sizeof(CK_ULONG)},
    {CKA_ID,       (CK_BYTE_PTR)rsa_sample_key_name,  (CK_ULONG)rsa_sample_key_name_len},
  };
  CK_OBJECT_HANDLE prv_key = 0;

  // Find the private key object
  rv = pkcs11_funcs->C_FindObjectsInit(session, find_template, sizeof(find_template)/sizeof(CK_ATTRIBUTE));
  if (rv) leave ("C_FindObjectsInit failed", rv);
  CK_ULONG found = 0;
  rv = pkcs11_funcs->C_FindObjects(session, &prv_key, 1, &found);
  if (rv) leave ("C_FindObjects failed", rv);
  pkcs11_funcs->C_FindObjectsFinal(session);

  if (found!=1) leave ("Key not found");

  CK_ATTRIBUTE pub_key_template[] = 
  {
    {CKA_MODULUS,         NULL,             0},
    {CKA_PUBLIC_EXPONENT, NULL,             0},
    {CKA_TOKEN,           &ck_false,        sizeof(CK_BBOOL)},
    {CKA_CLASS,           &cko_public_key,  sizeof(CK_ULONG)},
    {CKA_KEY_TYPE,        &ckk_rsa,         sizeof(CK_ULONG)},
  };

  // Get RSA public key material
  rv = pkcs11_funcs->C_GetAttributeValue(session, prv_key, pub_key_template, 2); // 2 first attributes, sizes only
  if (rv) leave ("C_GetAttributeValue failed", rv);
  pub_key_template[0].pValue = new CK_BYTE[pub_key_template[0].ulValueLen];
  pub_key_template[1].pValue = new CK_BYTE[pub_key_template[1].ulValueLen];
  rv = pkcs11_funcs->C_GetAttributeValue(session, prv_key, pub_key_template, 2); // 2 first attributes
  if (rv) leave ("C_GetAttributeValue failed", rv);

  // Create temporary RSA public key
  CK_OBJECT_HANDLE pub_key = 0;
  rv = pkcs11_funcs->C_CreateObject(session, pub_key_template, sizeof(pub_key_template)/sizeof(CK_ATTRIBUTE), &pub_key);
  if (rv) leave ("C_CreateObject failed", rv);

  // free temporary buffers
  delete[] pub_key_template[0].pValue;
  delete[] pub_key_template[1].pValue;

  // Verify signature
  CK_MECHANISM rsa_sign = {CKM_SHA256_RSA_PKCS, NULL, 0};
  rv = pkcs11_funcs->C_VerifyInit(session, &rsa_sign, pub_key);
  if (rv) leave ("C_VerifyInit failed", rv);

  rv = pkcs11_funcs->C_Verify(session, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), signature, signature_len);
  if (rv) leave ("C_Verify failed", rv);

  // Close PKCS#11 session (it destroys temporary key)
  pkcs11_funcs->C_CloseSession(session);
}

void rsa_sign()
{
  initialize_pkcs11();

  CK_SLOT_ID slot_id = 0; // default slot ID
  CK_SESSION_HANDLE session = 0;
  CK_RV rv;
  
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
    {CKA_TOKEN, &ck_true,                              sizeof(CK_BBOOL)},
    {CKA_ID,    (CK_BYTE_PTR)rsa_sample_key_name,      (CK_ULONG)rsa_sample_key_name_len},
  };

  // Generate key pair
  CK_OBJECT_HANDLE pub_key = 0, prv_key = 0;
  rv = pkcs11_funcs->C_GenerateKeyPair(session, &rsa_gen, 
    pub_template, sizeof(pub_template)/sizeof(CK_ATTRIBUTE), 
    prv_template, sizeof(prv_template)/sizeof(CK_ATTRIBUTE), 
    &pub_key, &prv_key);
  if (rv) leave ("C_GenerateKeyPair failed", rv);


  // Sign data
  CK_MECHANISM rsa_sign = {CKM_SHA256_RSA_PKCS, NULL, 0};
  rv = pkcs11_funcs->C_SignInit(session, &rsa_sign, prv_key);
  if (rv) leave ("C_SignInit failed", rv);

  CK_ULONG signature_len;
  rv = pkcs11_funcs->C_Sign(session, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), NULL, &signature_len);
  if (rv) leave ("C_Sign failed", rv);

  CK_BYTE_PTR signature = new CK_BYTE[signature_len];
  rv = pkcs11_funcs->C_Sign(session, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), signature, &signature_len);
  if (rv) leave ("C_Sign failed", rv);


  // Verify signature
  rv = pkcs11_funcs->C_VerifyInit(session, &rsa_sign, pub_key);
  if (rv) leave ("C_VerifyInit failed", rv);

  rv = pkcs11_funcs->C_Verify(session, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), signature, signature_len);
  if (rv) leave ("C_Verify failed", rv);

  // Close PKCS#11 session
  pkcs11_funcs->C_CloseSession(session);

  find_and_verify(signature, signature_len);

  delete[] signature;

  uninitialize_pkcs11();
}