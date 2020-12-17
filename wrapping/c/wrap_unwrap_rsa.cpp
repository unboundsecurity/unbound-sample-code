#include "sample_pkcs11.h"

void wrap_unwrap_rsa()
{
  CK_RV rv;
  CK_SLOT_ID slot_id = 0; // default slot ID
  CK_SESSION_HANDLE session = 0;

 
  CK_CHAR_PTR wrapped_key_name  = (CK_CHAR_PTR)"Wrapped key";
  CK_ULONG wrapped_key_name_len = (CK_ULONG)strlen((const char*)wrapped_key_name);   

  CK_CHAR_PTR unwrapping_key_name = (CK_CHAR_PTR)"Unwrapping key";
  CK_ULONG unwrapping_key_name_len = (CK_ULONG)strlen((const char*)unwrapping_key_name);
  
  CK_ULONG ckk_aes = CKK_AES;
  CK_ULONG cko_secret_key = CKO_SECRET_KEY;
  CK_BBOOL ck_true = CK_TRUE;
  CK_BBOOL ck_false = CK_FALSE;
  CK_ULONG value_len = 32;
  CK_ULONG modulus_bits = 2048;
  
CK_ATTRIBUTE t_unwrapping_prv_key[] = 
  {
    {CKA_TOKEN,     &ck_true,             sizeof(CK_BBOOL)},
    {CKA_LABEL,     unwrapping_key_name,  unwrapping_key_name_len},
  };
  
  CK_ATTRIBUTE t_wrapping_pub_key[] = 
  {
    {CKA_TOKEN,        &ck_true,           sizeof(CK_BBOOL)},
    {CKA_MODULUS_BITS, &modulus_bits,      sizeof(CK_ULONG)},
  };

  CK_ATTRIBUTE t_create_wrapped_key[] = 
  {
    {CKA_TOKEN,             &ck_true,           sizeof(CK_BBOOL)},
    {CKA_CLASS,             &cko_secret_key,    sizeof(CK_ULONG)},
    {CKA_KEY_TYPE,          &ckk_aes,           sizeof(CK_ULONG)},
    {CKA_VALUE_LEN,         &value_len,         sizeof(CK_ULONG)},
    {CKA_EXTRACTABLE,       &ck_true,           sizeof(CK_BBOOL)},
    {CKA_WRAP_WITH_TRUSTED, &ck_false,          sizeof(CK_BBOOL)},
    {CKA_LABEL,             wrapped_key_name,   wrapped_key_name_len},
  };

  CK_ATTRIBUTE t_unwrapped_key[] = 
  {   
    {CKA_TOKEN,     &ck_true,           sizeof(CK_BBOOL)},
    {CKA_CLASS,     &cko_secret_key,    sizeof(CK_ULONG)},
    {CKA_KEY_TYPE,  &ckk_aes,           sizeof(CK_ULONG)},
    {CKA_LABEL,     wrapped_key_name,   wrapped_key_name_len},
  };

  CK_RSA_PKCS_OAEP_PARAMS oaep_info = {CKM_SHA256, CKG_MGF1_SHA256, CKZ_DATA_SPECIFIED, NULL, 0};
  CK_MECHANISM wrap_mech =  {CKM_RSA_PKCS_OAEP, &oaep_info, sizeof(CK_RSA_PKCS_OAEP_PARAMS)};
  CK_MECHANISM rsa_gen_mech = {CKM_RSA_PKCS_KEY_PAIR_GEN, NULL, 0 };
  CK_MECHANISM aes_gen_mech = {CKM_AES_KEY_GEN, NULL, 0 };
  
  CK_ULONG wrapped_data_len;
  CK_OBJECT_HANDLE hWrappingPubKey = CK_INVALID_HANDLE;
  CK_OBJECT_HANDLE hUnwrappingPrvKey = CK_INVALID_HANDLE;
  CK_OBJECT_HANDLE hWrappedKey = CK_INVALID_HANDLE;
  CK_OBJECT_HANDLE hUnwrappedKey = CK_INVALID_HANDLE;
  
  CK_BYTE_PTR wrapped_data = NULL;

  initialize_pkcs11();
  
  rv = pkcs11_funcs->C_OpenSession(slot_id, CKF_SERIAL_SESSION|CKF_RW_SESSION, NULL, NULL, &session);
  if (rv) leave ("C_OpenSession failed", rv);

#ifdef PKCS11_PASSWORD
  rv = pkcs11_funcs->C_Login(session, CKU_USER, (CK_CHAR_PTR)PKCS11_PASSWORD, (CK_ULONG)strlen(PKCS11_PASSWORD));
  if (rv) leave ("C_Login failed", rv);
#endif

  
  // Generate a wrapping/unwrapping RSA key pair
  rv = pkcs11_funcs->C_GenerateKeyPair(session, &rsa_gen_mech, t_wrapping_pub_key, sizeof(t_wrapping_pub_key)/sizeof(CK_ATTRIBUTE), t_unwrapping_prv_key, sizeof(t_unwrapping_prv_key)/sizeof(CK_ATTRIBUTE), &hWrappingPubKey, &hUnwrappingPrvKey);
  if (rv!=CKR_OK) leave("C_GenerateKeyPair failed", rv);
  
  // Generate a key to be wrapped
  rv = pkcs11_funcs->C_GenerateKey(session, &aes_gen_mech, t_create_wrapped_key, sizeof(t_create_wrapped_key)/sizeof(CK_ATTRIBUTE), &hWrappedKey);
  if (rv!=CKR_OK) leave("C_GenerateKey failed", rv);
  
  // wrap key
  rv = pkcs11_funcs->C_WrapKey(session, &wrap_mech, hWrappingPubKey, hWrappedKey, NULL, &wrapped_data_len);
  if (rv!=CKR_OK) leave("C_WrapKey failed", rv);
  
  wrapped_data = new CK_BYTE[wrapped_data_len];
  rv = pkcs11_funcs->C_WrapKey(session, &wrap_mech, hWrappingPubKey, hWrappedKey, wrapped_data, &wrapped_data_len);
  if (rv!=CKR_OK) leave("C_WrapKey failed", rv);
  
  // destroy original key
  rv = pkcs11_funcs->C_DestroyObject(session, hWrappedKey);
  if (rv!=CKR_OK) leave("C_DestroyObject failed", rv);
  
  // unwrap key
  rv = pkcs11_funcs->C_UnwrapKey(session, &wrap_mech, hUnwrappingPrvKey, wrapped_data, wrapped_data_len, t_unwrapped_key, sizeof(t_unwrapped_key)/sizeof(CK_ATTRIBUTE), &hUnwrappedKey);
  if (rv!=CKR_OK) leave("C_UnwrapKey failed", rv);
  
  delete[] wrapped_data;
  
  pkcs11_funcs->C_CloseSession(session);
  uninitialize_pkcs11();
}
