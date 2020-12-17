#include "sample_pkcs11.h"


void rsa_generate()
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
  char key_name[] = "TEST RSA KEY";

  CK_ATTRIBUTE pub_template[] = 
  {
    {CKA_TOKEN,        &ck_false,     sizeof(CK_BBOOL)},
    {CKA_MODULUS_BITS, &modulus_bits, sizeof(CK_ULONG)},
  };

  CK_ATTRIBUTE prv_template[] = 
  {
    {CKA_TOKEN, &ck_true,  sizeof(CK_BBOOL)},
    {CKA_ID,    key_name,  sizeof(key_name)-1},
  };

  // Generate key pair
  CK_OBJECT_HANDLE pub_key = 0, prv_key = 0;
  rv = pkcs11_funcs->C_GenerateKeyPair(session, &rsa_gen, 
    pub_template, sizeof(pub_template)/sizeof(CK_ATTRIBUTE), 
    prv_template, sizeof(prv_template)/sizeof(CK_ATTRIBUTE), 
    &pub_key, &prv_key);
  if (rv) leave ("C_GenerateKeyPair failed", rv);

  // Find RSA key type
  CK_ULONG cko_private_key = CKO_PRIVATE_KEY;
  CK_ULONG ckk_rsa = CKK_RSA;
  CK_ATTRIBUTE find_template[] = 
  {
    {CKA_TOKEN,    &ck_true,         sizeof(CK_BBOOL)},
    {CKA_CLASS,    &cko_private_key, sizeof(CK_ULONG)},
    {CKA_KEY_TYPE, &ckk_rsa,         sizeof(CK_ULONG)},
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