#include "sample_pkcs11.h"

void ecdsa_generate()
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

  // EC generation mechanism
  CK_MECHANISM ecdsa_gen = {CKM_EC_KEY_PAIR_GEN, NULL, 0};

  // ECDSA curve OID
  CK_BYTE p256_curve[] = { 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07 };

  CK_BBOOL ck_false = CK_FALSE;
  CK_BBOOL ck_true  = CK_TRUE;
  char key_name[] = "TEST ECDSA KEY";

  CK_ATTRIBUTE pub_template[] = 
  {
    {CKA_TOKEN,     &ck_false,   sizeof(CK_BBOOL)},
    {CKA_EC_PARAMS, p256_curve,  sizeof(p256_curve)},
  };

  CK_ATTRIBUTE prv_template[] = 
  {
    {CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
    {CKA_SIGN,  &ck_true, sizeof(CK_BBOOL)},
    {CKA_ID,    key_name, sizeof(key_name)-1},
  };

  // Generate key pair
  CK_OBJECT_HANDLE pub_key = 0, prv_key = 0;
  rv = pkcs11_funcs->C_GenerateKeyPair(session, &ecdsa_gen, 
    pub_template, sizeof(pub_template)/sizeof(CK_ATTRIBUTE), 
    prv_template, sizeof(prv_template)/sizeof(CK_ATTRIBUTE), 
    &pub_key, &prv_key);
  if (rv) leave ("C_GenerateKeyPair failed", rv);

  // Find EC key type
  CK_ULONG cko_private_key = CKO_PRIVATE_KEY;
  CK_ULONG ckk_ec = CKK_EC;
  CK_ATTRIBUTE find_template[] = 
  {
    {CKA_TOKEN,    &ck_true,         sizeof(CK_BBOOL)},
    {CKA_CLASS,    &cko_private_key, sizeof(CK_ULONG)},
    {CKA_KEY_TYPE, &ckk_ec,          sizeof(CK_ULONG)},
    {CKA_SIGN,     &ck_true,         sizeof(CK_BBOOL)},
    {CKA_ID,       key_name,         sizeof(key_name)-1},
  };
  rv = pkcs11_funcs->C_FindObjectsInit(session, find_template, sizeof(find_template)/sizeof(CK_ATTRIBUTE));
  if (rv) leave ("C_FindObjectsInit failed", rv);
  CK_ULONG found = 0;
  rv = pkcs11_funcs->C_FindObjects(session, &prv_key, 1, &found);
  if (rv) leave ("C_FindObjects failed", rv);
  pkcs11_funcs->C_FindObjectsFinal(session);

  if (found!=1) leave ("Key not found");


  // Close PKCS#11 session
  pkcs11_funcs->C_CloseSession(session);

  uninitialize_pkcs11();
}