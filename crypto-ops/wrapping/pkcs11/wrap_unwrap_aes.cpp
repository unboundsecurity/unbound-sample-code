#include "sample_pkcs11.h"

void wrap_unwrap_aes()
{
  CK_RV rv;
  CK_SLOT_ID slot_id = 0; // default slot ID
  CK_SESSION_HANDLE session = 0;

  CK_CHAR_PTR wrapping_key_name = (CK_CHAR_PTR)"Wrapping key";
  CK_ULONG wrapping_key_name_len = (CK_ULONG)strlen((const char*)wrapping_key_name);
  
  CK_CHAR_PTR wrapped_key_name  = (CK_CHAR_PTR)"Wrapped key";
  CK_ULONG wrapped_key_name_len = (CK_ULONG)strlen((const char*)wrapped_key_name);   
  
  CK_ULONG ckk_aes = CKK_AES;
  CK_ULONG cko_secret_key = CKO_SECRET_KEY;
  CK_BBOOL ck_true = CK_TRUE;
  CK_BBOOL ck_false = CK_FALSE;
  CK_ULONG value_len = 32;
  
  CK_ATTRIBUTE t_create_wrapping_key[] = 
  {
    {CKA_TOKEN,     &ck_true,           sizeof(CK_BBOOL)},
    {CKA_CLASS,     &cko_secret_key,    sizeof(CK_ULONG)},
    {CKA_KEY_TYPE,  &ckk_aes,           sizeof(CK_ULONG)},
    {CKA_VALUE_LEN, &value_len,         sizeof(CK_ULONG)},
    {CKA_LABEL,     wrapping_key_name,  wrapping_key_name_len},
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

  CK_MECHANISM wrap_mech = {CKM_AES_ECB, NULL, 0 };
  CK_MECHANISM gen_mech = {CKM_AES_KEY_GEN, NULL, 0 };
  
  CK_ULONG wrapped_data_len;
  CK_OBJECT_HANDLE hWrappingKey = CK_INVALID_HANDLE;
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

  
  // Generate a wrapping key
  rv = pkcs11_funcs->C_GenerateKey(session, &gen_mech, t_create_wrapping_key, sizeof(t_create_wrapping_key)/sizeof(CK_ATTRIBUTE), &hWrappingKey);
  if (rv!=CKR_OK) leave("C_GenerateKey failed", rv);
  
  // Generate a key to be wrapped
  rv = pkcs11_funcs->C_GenerateKey(session, &gen_mech, t_create_wrapped_key, sizeof(t_create_wrapped_key)/sizeof(CK_ATTRIBUTE), &hWrappedKey);
  if (rv!=CKR_OK) leave("C_GenerateKey failed", rv);
  
  // wrap key
  rv = pkcs11_funcs->C_WrapKey(session, &wrap_mech, hWrappingKey, hWrappedKey, NULL, &wrapped_data_len);
  if (rv!=CKR_OK) leave("C_WrapKey failed", rv);
  
  wrapped_data = new CK_BYTE[wrapped_data_len];
  rv = pkcs11_funcs->C_WrapKey(session, &wrap_mech, hWrappingKey, hWrappedKey, wrapped_data, &wrapped_data_len);
  if (rv!=CKR_OK) leave("C_WrapKey failed", rv);
  
  // destroy original key
  rv = pkcs11_funcs->C_DestroyObject(session, hWrappedKey);
  if (rv!=CKR_OK) leave("C_DestroyObject failed", rv);
  
  // unwrap key
  rv = pkcs11_funcs->C_UnwrapKey(session, &wrap_mech, hWrappingKey, wrapped_data, wrapped_data_len, t_unwrapped_key, sizeof(t_unwrapped_key)/sizeof(CK_ATTRIBUTE), &hUnwrappedKey);
  if (rv!=CKR_OK) leave("C_UnwrapKey failed", rv);
  
  delete[] wrapped_data;
  
  pkcs11_funcs->C_CloseSession(session);
  uninitialize_pkcs11();
}
