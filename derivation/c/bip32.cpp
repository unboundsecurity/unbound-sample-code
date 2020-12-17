#include "sample_pkcs11.h"


void bip32()
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

  CK_ULONG cko_secret_key = CKO_SECRET_KEY;
  CK_ULONG ckk_generic_secret = CKK_GENERIC_SECRET;
  CK_ULONG cko_private_key = CKO_PRIVATE_KEY;
  CK_BBOOL ckFalse = CK_FALSE;
  CK_BBOOL ckTrue = CK_TRUE;
  CK_ULONG ckk_ec = CKK_EC;

  CK_BYTE secp256k_oid[] = { 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x0a };

  CK_ULONG seed_value_len = 32;
  CK_ATTRIBUTE t_new_seed_key[] = 
  {
    {CKA_CLASS,     &cko_secret_key,     sizeof(CK_ULONG)},
    {CKA_KEY_TYPE,  &ckk_generic_secret, sizeof(CK_ULONG)},
    {CKA_TOKEN,     &ckTrue,             sizeof(CK_BBOOL)},
    {CKA_DERIVE,    &ckTrue,             sizeof(CK_BBOOL)},
    {CKA_VALUE_LEN, &seed_value_len,     sizeof(CK_ULONG)},
  };

  CK_ATTRIBUTE t_new_ec_key[] = 
  {
    {CKA_CLASS,     &cko_private_key,  sizeof(CK_ULONG)},
    {CKA_KEY_TYPE,  &ckk_ec,           sizeof(CK_ULONG)},
    {CKA_TOKEN,     &ckTrue,           sizeof(CK_BBOOL)},
    {CKA_EC_PARAMS, secp256k_oid,      sizeof(secp256k_oid)},
    {CKA_SIGN,      &ckTrue,           sizeof(CK_BBOOL)},
    {CKA_DERIVE,    &ckTrue,           sizeof(CK_BBOOL)},
  };

  // Generate seed key
  CK_MECHANISM gen = { CKM_GENERIC_SECRET_KEY_GEN, 0, 0 };
  CK_OBJECT_HANDLE hSeedKey = 0;
  rv = pkcs11_funcs->C_GenerateKey(session, &gen, t_new_seed_key, sizeof(t_new_seed_key)/sizeof(CK_ATTRIBUTE), &hSeedKey);
  if (rv) leave("C_GenerateKey failed", rv);

  // Master derivation
  CK_MECHANISM bip32_mech_from_master = {DYCKM_DERIVE_ECDSA_BIP, NULL, 0};
  CK_OBJECT_HANDLE hMasterKey = 0;
  rv = pkcs11_funcs->C_DeriveKey(session, &bip32_mech_from_master, hSeedKey, t_new_ec_key, sizeof(t_new_ec_key)/sizeof(CK_ATTRIBUTE), &hMasterKey);
  if (rv) leave("C_DeriveKey failed", rv);

  // Hardened derivation
  DYCK_DERIVE_BIP_PARAMS params_hardened;
  params_hardened.hardened = CK_TRUE;
  params_hardened.ulChildNumber = CK_ULONG(12345);
  CK_MECHANISM bip32_mech_hardened = {DYCKM_DERIVE_ECDSA_BIP, &params_hardened, sizeof(DYCK_DERIVE_BIP_PARAMS)};
  CK_OBJECT_HANDLE hHardenedDerivedKey = 0;
  rv = pkcs11_funcs->C_DeriveKey(session, &bip32_mech_hardened, hMasterKey, t_new_ec_key, sizeof(t_new_ec_key)/sizeof(CK_ATTRIBUTE), &hHardenedDerivedKey);
  if (rv) leave("C_DeriveKey failed", rv);

  // Normal derivation
  DYCK_DERIVE_BIP_PARAMS params_normal;
  params_normal.hardened = CK_FALSE;
  params_hardened.ulChildNumber = CK_ULONG(7890);
  CK_MECHANISM bip32_mech_normal = {DYCKM_DERIVE_ECDSA_BIP, &params_normal, sizeof(DYCK_DERIVE_BIP_PARAMS)};
  CK_OBJECT_HANDLE hNormalDerivedKey = 0;
  rv = pkcs11_funcs->C_DeriveKey(session, &bip32_mech_hardened, hHardenedDerivedKey, t_new_ec_key, sizeof(t_new_ec_key)/sizeof(CK_ATTRIBUTE), &hNormalDerivedKey);
  if (rv) leave("C_DeriveKey failed", rv);
  
  // Close PKCS#11 session
  pkcs11_funcs->C_CloseSession(session);

  uninitialize_pkcs11();
}