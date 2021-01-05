#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dy_pkcs11.h"

void halt(CK_RV rv)
{
	printf("Error %08lx\n", rv);
	exit(-1);
}

int main(int argc, char *argv[])
{
	CK_RV rv;
	CK_SESSION_HANDLE hSession;
	CK_SLOT_ID slot = 0;
	CK_BBOOL ckTrue = CK_TRUE;
	CK_BBOOL ckFalse = CK_FALSE;
	CK_ULONG cko_secret_key = CKO_SECRET_KEY;
	CK_ULONG ckk_aes = CKK_AES;

	rv = C_Initialize(NULL);
	if (rv != CKR_OK)
		halt(rv);
	rv = C_OpenSession(slot, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL, NULL, &hSession);
	if (rv != CKR_OK)
		halt(rv);

	// generate base key
	CK_OBJECT_HANDLE hBaseKey = 0;
	CK_BYTE base_key_value[16] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
	CK_BYTE label[] = {10, 20, 30, 40, 50};
	CK_ULONG label_size = sizeof(label);
	CK_ATTRIBUTE t_based[] = {
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
		{CKA_KEY_TYPE, &ckk_aes, sizeof(CK_ULONG)},
		{CKA_VALUE, &base_key_value, sizeof(base_key_value)},
		{CKA_DERIVE, &ckTrue, sizeof(CK_BBOOL)}};
	rv = C_CreateObject(hSession, t_based, sizeof(t_based) / sizeof(CK_ATTRIBUTE), &hBaseKey);
	if (rv != CKR_OK)
		halt(rv);

	// make CMAC derivation using NIST KDF
	CK_BYTE derived_value[24];
	CK_ULONG derived_len = sizeof(derived_value);
	DYCK_NIST_KDF_CMAC_CTR_PARAMS params = {label, label_size, NULL, 0, derived_len};
	CK_MECHANISM mech = {DYCKM_NIST_KDF_CMAC_CTR, &params, sizeof(params)};

	CK_ATTRIBUTE t_derived[] = {
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
		{CKA_KEY_TYPE, &ckk_aes, sizeof(CK_ULONG)},
		{CKA_SENSITIVE, &ckFalse, sizeof(CK_BBOOL)},
		{CKA_VALUE_LEN, &derived_len, sizeof(CK_ULONG)},
	};
	CK_OBJECT_HANDLE hDerivedKey = 0;
	rv = C_DeriveKey(hSession, &mech, hBaseKey, t_derived, sizeof(t_derived) / sizeof(CK_ATTRIBUTE), &hDerivedKey);
	if (rv != CKR_OK)
		halt(rv);

	// destroy base key
	rv = C_DestroyObject(hSession, hBaseKey);
	if (rv != CKR_OK)
		halt(rv);
	// destroy derived key
	rv = C_DestroyObject(hSession, hDerivedKey);
	if (rv != CKR_OK)
		halt(rv);

	C_CloseSession(hSession);
	C_Finalize(NULL);
}