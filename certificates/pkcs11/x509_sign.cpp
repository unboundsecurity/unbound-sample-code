#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dy_pkcs11.h"

void halt(CK_RV rv)
{
	printf("Error %08lx\n", rv);
	exit(-1);
}
/***********************************************
* argv[1] - Default user password, if not empty
***********************************************/
int main(int argc, char *argv[])
{
	CK_RV rv;
	CK_SESSION_HANDLE hSession;
	CK_SLOT_ID slot = 0;
	CK_BBOOL ckTrue = CK_TRUE;
	CK_BBOOL ckFalse = CK_FALSE;

	rv = C_Initialize(NULL);
	if (rv != CKR_OK)
		halt(rv);
	rv = C_OpenSession(slot, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL, NULL, &hSession);
	if (rv != CKR_OK)
		halt(rv);

	CK_CHAR_PTR password = nullptr;
	CK_ULONG pass_len = 0;
	if (argc > 1) 
	{
		password = (CK_CHAR_PTR)argv[1];
		pass_len = (CK_ULONG)strlen(argv[1]);
	}
	
	rv = C_Login(hSession, CKU_USER, CK_CHAR_PTR(password), pass_len);
	if (rv != CKR_OK)
		halt(rv);

	// generate elliptic-curve key pair (curve P-256)
	unsigned char p256_oid[] = {0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07};
	CK_MECHANISM genMechEcc = {CKM_EC_KEY_PAIR_GEN, NULL, 0};
	CK_ATTRIBUTE tPubKeyEcc[] = {
		{CKA_EC_PARAMS, p256_oid, sizeof(p256_oid)},
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
	};
	CK_ATTRIBUTE tPrvKeyEcc[] = {
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
	};
	CK_SESSION_HANDLE hCaPrvKey = 0, hCaPubKey;
	rv = C_GenerateKeyPair(hSession, &genMechEcc, tPubKeyEcc, sizeof(tPubKeyEcc) / sizeof(CK_ATTRIBUTE), tPrvKeyEcc, sizeof(tPrvKeyEcc) / sizeof(CK_ATTRIBUTE), &hCaPubKey, &hCaPrvKey);
	if (rv != CKR_OK)
		halt(rv);

	// Make self-signed certificate
	// 1. create self-singed certificate DER
	CK_ULONG ca_cert_der_size = 0;
	rv = DYC_SelfSignX509(hSession, hCaPrvKey, CKM_SHA256, (CK_CHAR_PTR) "CN=CA", NULL, 0, 365, NULL, &ca_cert_der_size);
	if (rv != CKR_OK)
		halt(rv);
	CK_BYTE_PTR ca_cert_der = (CK_BYTE_PTR)malloc(ca_cert_der_size);
	rv = DYC_SelfSignX509(hSession, hCaPrvKey, CKM_SHA256, (CK_CHAR_PTR) "CN=CA", NULL, 0, 365, ca_cert_der, &ca_cert_der_size);
	if (rv != CKR_OK)
		halt(rv);
	// 2. create the certificate object
	CK_ULONG cko_certificate = CKO_CERTIFICATE;
	CK_ULONG ckc_x509 = CKC_X_509;
	CK_ATTRIBUTE tCaCert[] = {
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_CLASS, &cko_certificate, sizeof(CK_ULONG)},
		{CKA_CERTIFICATE_TYPE, &ckc_x509, sizeof(CK_ULONG)},
		{CKA_VALUE, ca_cert_der, ca_cert_der_size},
	};
	CK_OBJECT_HANDLE hCaCert = 0;
	rv = C_CreateObject(hSession, tCaCert, sizeof(tCaCert) / sizeof(CK_ATTRIBUTE), &hCaCert);

	//generate RSA key pair
	CK_ULONG bits = 2048;
	CK_ATTRIBUTE tPubKeyRsa[] = {
		{CKA_TOKEN, &ckFalse, sizeof(CK_BBOOL)},
		{CKA_MODULUS_BITS, &bits, sizeof(bits)},
	};
	CK_ATTRIBUTE tPrvGenRsa[] = {
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
	};
	CK_MECHANISM rsaGenMech = {CKM_RSA_PKCS_KEY_PAIR_GEN, NULL, 0};
	CK_OBJECT_HANDLE hPubKey, hPrvKey;
	rv = C_GenerateKeyPair(hSession, &rsaGenMech, tPubKeyRsa, sizeof(tPubKeyRsa) / sizeof(CK_ATTRIBUTE), tPrvGenRsa, sizeof(tPrvGenRsa) / sizeof(CK_ATTRIBUTE), &hPubKey, &hPrvKey);

	// Create Sertificate Request
	CK_ULONG csr_size = 0;
	rv = DYC_CreateX509Request(hSession, hPrvKey, CKM_SHA256, (CK_CHAR_PTR) "CN=test", NULL, &csr_size);
	if (rv != CKR_OK)
		halt(rv);
	CK_BYTE_PTR csr = (CK_BYTE_PTR)malloc(csr_size);
	rv = DYC_CreateX509Request(hSession, hPrvKey, CKM_SHA256, (CK_CHAR_PTR) "CN=test", csr, &csr_size);
	if (rv != CKR_OK)
		halt(rv);

	// Sign the Request
	CK_ULONG cert_der_size = 0;
	rv = DYC_SignX509(hSession, hPrvKey, ca_cert_der, ca_cert_der_size, CKM_SHA256, csr, csr_size, NULL, 0, 365, NULL, &cert_der_size);
	if (rv != CKR_OK)
		halt(rv);
	CK_BYTE_PTR cert_der = (CK_BYTE_PTR)malloc(cert_der_size);
	rv = DYC_SignX509(hSession, hPrvKey, ca_cert_der, ca_cert_der_size, CKM_SHA256, csr, csr_size, NULL, 0, 365, cert_der, &cert_der_size);
	if (rv != CKR_OK)
		halt(rv);

	C_CloseSession(hSession);
	C_Finalize(NULL);
}