#include "sample_pkcs11.h"

void ecdsa_sign()
{
  initialize_pkcs11();

  CK_SLOT_ID slot_id = 0; // default slot ID
  CK_SESSION_HANDLE session = 0;
  CK_RV rv;
  const char data_to_sign[] = "data to sign";
  
  // Open PKCS#11 session
  rv = pkcs11_funcs->C_OpenSession(slot_id, CKF_SERIAL_SESSION | CKF_RW_SESSION, 0, 0, &session);
  if (rv) leave ("C_OpenSession failed", rv);

#ifdef PKCS11_PASSWORD
  rv = pkcs11_funcs->C_Login(session, CKU_USER, (CK_CHAR_PTR)PKCS11_PASSWORD, (CK_ULONG)strlen(PKCS11_PASSWORD));
  if (rv) leave ("C_Login failed", rv);
#endif

  // RSA generation mechanism
  CK_MECHANISM ecdsa_gen = {CKM_EC_KEY_PAIR_GEN, NULL, 0};

  // ECDSA curve OID
  CK_BYTE p256_curve[] = { 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07 };

  CK_BBOOL ck_false = CK_FALSE;
  CK_BBOOL ck_true  = CK_TRUE;

  CK_ATTRIBUTE pub_template[] = 
  {
    {CKA_TOKEN,     &ck_false,   sizeof(CK_BBOOL)},
    {CKA_EC_PARAMS, p256_curve,  sizeof(p256_curve)},
  };

  CK_ATTRIBUTE prv_template[] = 
  {
    {CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
    {CKA_SIGN,  &ck_true,    sizeof(CK_BBOOL)},
  };

  // Generate key pair
  CK_OBJECT_HANDLE pub_key = 0, prv_key = 0;
  rv = pkcs11_funcs->C_GenerateKeyPair(session, &ecdsa_gen, 
    pub_template, sizeof(pub_template)/sizeof(CK_ATTRIBUTE), 
    prv_template, sizeof(prv_template)/sizeof(CK_ATTRIBUTE), 
    &pub_key, &prv_key);
  if (rv) leave ("C_GenerateKeyPair failed", rv);

  // Sign data
  CK_MECHANISM ecdsa_sign = {CKM_ECDSA_SHA256, NULL, 0};
  rv = pkcs11_funcs->C_SignInit(session, &ecdsa_sign, prv_key);
  if (rv) leave ("C_SignInit failed", rv);

  CK_ULONG signature_len;
  rv = pkcs11_funcs->C_Sign(session, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), NULL, &signature_len);
  if (rv) leave ("C_Sign failed", rv);

  CK_BYTE_PTR signature = new CK_BYTE[signature_len];
  rv = pkcs11_funcs->C_Sign(session, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), signature, &signature_len);
  if (rv) leave ("C_Sign failed", rv);


  // Verify signature
  rv = pkcs11_funcs->C_VerifyInit(session, &ecdsa_sign, pub_key);
  if (rv) leave ("C_VerifyInit failed", rv);

  rv = pkcs11_funcs->C_Verify(session, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), signature, signature_len);
  if (rv) leave ("C_Verify failed", rv);

  delete[] signature;

  // Close PKCS#11 session
  pkcs11_funcs->C_CloseSession(session);

  uninitialize_pkcs11();
}