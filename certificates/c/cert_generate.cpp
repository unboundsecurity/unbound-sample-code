#include "sample_pkcs11.h"


typedef CK_RV (CK_CALL_SPEC* CK_DYC_SelfSignX509)(
  CK_SESSION_HANDLE      hSession,
  CK_OBJECT_HANDLE       hPrivateKey,
  CK_MECHANISM_TYPE      hashAlg,
  CK_CHAR_PTR            pSubjectStr, 
  CK_BYTE_PTR            pSerial, 
  CK_ULONG               ulSerial, 
  CK_ULONG               days, 
  CK_BYTE_PTR            pX509, 
  CK_ULONG_PTR           pulX509Len);

typedef CK_RV (CK_CALL_SPEC* CK_DYC_CreateX509Request)(
  CK_SESSION_HANDLE      hSession,
  CK_OBJECT_HANDLE       hPrivateKey,
  CK_MECHANISM_TYPE      hashAlg,
  CK_CHAR_PTR            pSubjectStr,
  CK_BYTE_PTR            pCSR,
  CK_ULONG_PTR           pulCSRLen);

typedef CK_RV (CK_CALL_SPEC* CK_DYC_SignX509)(
  CK_SESSION_HANDLE      hSession,
  CK_OBJECT_HANDLE       hPrivateKey,
  CK_BYTE_PTR            pX509CA, 
  CK_ULONG               ulX509CA, 
  CK_MECHANISM_TYPE      hashAlg,
  CK_BYTE_PTR            pX509Request, 
  CK_ULONG               ulX509Request, 
  CK_BYTE_PTR            pSerial, 
  CK_ULONG               ulSerial, 
  CK_ULONG               days, 
  CK_BYTE_PTR            pX509, 
  CK_ULONG_PTR           pulX509Len);


void cert_generate()
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
  char ca_key_name[] = "CA RSA KEY";

  CK_ATTRIBUTE pub_template[] = 
  {
    {CKA_TOKEN,        &ck_false,     sizeof(CK_BBOOL)},
    {CKA_MODULUS_BITS, &modulus_bits, sizeof(CK_ULONG)},
  };

  CK_ATTRIBUTE ca_prv_template[] = 
  {
    {CKA_TOKEN, &ck_true,     sizeof(CK_BBOOL)},
    {CKA_ID,    ca_key_name,  sizeof(ca_key_name)-1},
  };

  // Generate key pair (CA)
  CK_OBJECT_HANDLE ca_pub_key = 0, ca_prv_key = 0;
  rv = pkcs11_funcs->C_GenerateKeyPair(session, &rsa_gen, 
    pub_template,    sizeof(pub_template)/sizeof(CK_ATTRIBUTE), 
    ca_prv_template, sizeof(ca_prv_template)/sizeof(CK_ATTRIBUTE), 
    &ca_pub_key, &ca_prv_key);
  if (rv) leave ("C_GenerateKeyPair failed", rv);

  CK_DYC_SelfSignX509 ptr_DYC_SelfSignX509 = nullptr;

#if defined(_WIN32)
  ptr_DYC_SelfSignX509 = (CK_DYC_SelfSignX509)GetProcAddress(pkcs11_lib, "DYC_SelfSignX509");
#else 
  ptr_DYC_SelfSignX509 = (CK_DYC_SelfSignX509)dlsym(pkcs11_lib, "DYC_SelfSignX509");
#endif
  
  if (!ptr_DYC_SelfSignX509) leave("DYC_SelfSignX509 not found");

  CK_ULONG ca_cert_der_size = 0;
  rv = ptr_DYC_SelfSignX509(session, ca_prv_key, CKM_SHA256, (CK_CHAR_PTR)"CN=CA", NULL, 0, 365, NULL, &ca_cert_der_size);
  if (rv) leave ("DYC_SelfSignX509 failed", rv);
  CK_BYTE_PTR ca_cert_der = new CK_BYTE[ca_cert_der_size];
  rv = ptr_DYC_SelfSignX509(session, ca_prv_key, CKM_SHA256, (CK_CHAR_PTR)"CN=CA", NULL, 0, 365, ca_cert_der, &ca_cert_der_size);
  if (rv) leave ("DYC_SelfSignX509 failed", rv);

  CK_ULONG cko_certificate = CKO_CERTIFICATE;
  CK_ULONG ckc_x509 = CKC_X_509;

  CK_ATTRIBUTE ca_cert_template[] =
  {
    {CKA_TOKEN,            &ck_true,          sizeof(CK_BBOOL)},
    {CKA_CLASS,            &cko_certificate,  sizeof(CK_ULONG)},
    {CKA_CERTIFICATE_TYPE, &ckc_x509,         sizeof(CK_ULONG)},
    {CKA_VALUE,            ca_cert_der,       ca_cert_der_size},
  };

  CK_OBJECT_HANDLE ca_cert = 0;
  rv = pkcs11_funcs->C_CreateObject(session, ca_cert_template, sizeof(ca_cert_template)/sizeof(CK_ATTRIBUTE), &ca_cert);
  if (rv) leave ("C_CreateObject(cert) failed", rv);

  char key_name[] = "TEST RSA KEY";
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

  CK_DYC_CreateX509Request ptr_DYC_CreateX509Request = nullptr;

#if defined(_WIN32)
  ptr_DYC_CreateX509Request = (CK_DYC_CreateX509Request)GetProcAddress(pkcs11_lib, "DYC_CreateX509Request");
#else 
  ptr_DYC_CreateX509Request = (CK_DYC_CreateX509Request)dlsym(pkcs11_lib, "DYC_CreateX509Request");
#endif

  if (!ptr_DYC_CreateX509Request) leave("DYC_CreateX509Request not found");

  CK_ULONG csr_size = 0;
  rv = ptr_DYC_CreateX509Request(session, prv_key, CKM_SHA256, (CK_CHAR_PTR)"CN=test", NULL, &csr_size);
  if (rv) leave ("DYC_CreateX509Request failed", rv);
  CK_BYTE_PTR csr = new CK_BYTE[csr_size];
  rv = ptr_DYC_CreateX509Request(session, prv_key, CKM_SHA256, (CK_CHAR_PTR)"CN=test", csr, &csr_size);
  if (rv) leave ("DYC_CreateX509Request failed", rv);

  CK_DYC_SignX509 ptr_DYC_SignX509 = nullptr;

#if defined(_WIN32)
  ptr_DYC_SignX509 = (CK_DYC_SignX509)GetProcAddress(pkcs11_lib, "DYC_SignX509");
#else 
  ptr_DYC_SignX509 = (CK_DYC_SignX509)dlsym(pkcs11_lib, "DYC_SignX509");
#endif

  if (!ptr_DYC_SignX509) leave("DYC_SignX509 not found");

  CK_ULONG cert_der_size = 0;
  rv = ptr_DYC_SignX509(session, ca_prv_key, ca_cert_der, ca_cert_der_size, CKM_SHA256, csr, csr_size, NULL, 0, 365, NULL, &cert_der_size);
  if (rv) leave ("DYC_SignX509 failed", rv);
  CK_BYTE_PTR cert_der = new CK_BYTE[cert_der_size];
  rv = ptr_DYC_SignX509(session, ca_prv_key, ca_cert_der, ca_cert_der_size, CKM_SHA256, csr, csr_size, NULL, 0, 365, cert_der, &cert_der_size);
  if (rv) leave ("DYC_SignX509 failed", rv);
  
  delete[] ca_cert_der;
  delete[] csr;

  CK_ATTRIBUTE cert_template[] =
  {
    {CKA_TOKEN,            &ck_true,          sizeof(CK_BBOOL)},
    //{CKA_PRIVATE,          &ck_true,          sizeof(CK_BBOOL)},
    {CKA_CLASS,            &cko_certificate,  sizeof(CK_ULONG)},
    {CKA_CERTIFICATE_TYPE, &ckc_x509,         sizeof(CK_ULONG)},
    {CKA_VALUE,            cert_der,          cert_der_size},
  };
  
  CK_OBJECT_HANDLE cert = 0;
  rv = pkcs11_funcs->C_CreateObject(session, cert_template, sizeof(cert_template)/sizeof(CK_ATTRIBUTE), &cert);
  if (rv) leave ("C_CreateObject(cert) failed", rv);

  delete[] cert_der;

  pkcs11_funcs->C_DestroyObject(session, ca_prv_key);
  pkcs11_funcs->C_DestroyObject(session, prv_key);
  pkcs11_funcs->C_DestroyObject(session, ca_cert);
  pkcs11_funcs->C_DestroyObject(session, cert);

  // Close PKCS#11 session
  pkcs11_funcs->C_CloseSession(session);

  uninitialize_pkcs11();
}