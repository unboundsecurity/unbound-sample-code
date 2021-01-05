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

	rv = C_Initialize(NULL);
	if (rv != CKR_OK)
		halt(rv);
	rv = C_OpenSession(slot, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL, NULL, &hSession);
	if (rv != CKR_OK)
		halt(rv);

	char password[] = ""; // ------ set your password here -------
	rv = C_Login(hSession, CKU_USER, CK_CHAR_PTR(password), CK_ULONG(strlen(password)));
	if (rv != CKR_OK)
		halt(rv);

	CK_OBJECT_HANDLE hPubKey = 0;
	CK_OBJECT_HANDLE hPrvKey = 0;
	// generate EDDSA key
	CK_MECHANISM genMech = {DYCKM_EDDSA_KEY_GEN, NULL, 0};
	CK_ULONG dyckk_eddsa = DYCKK_EDDSA;
	CK_ATTRIBUTE tPubKey[] = {
		{CKA_TOKEN, &ckFalse, sizeof(CK_BBOOL)},
		{CKA_VERIFY, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_KEY_TYPE, &dyckk_eddsa, sizeof(CK_ULONG)},
	};
	CK_ATTRIBUTE tPrvKey[] = {
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_PRIVATE, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_SIGN, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_KEY_TYPE, &dyckk_eddsa, sizeof(CK_ULONG)},
	};
	rv = C_GenerateKeyPair(hSession, &genMech, tPubKey, sizeof(tPubKey) / sizeof(CK_ATTRIBUTE), tPrvKey, sizeof(tPrvKey) / sizeof(CK_ATTRIBUTE), &hPubKey, &hPrvKey);
	if (rv != CKR_OK)
		halt(rv);

	// Sign with EDDSA key
	unsigned char testData[32] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
	CK_MECHANISM mech = {DYCKM_EDDSA, 0, 0};
	rv = C_SignInit(hSession, &mech, hPrvKey);
	CK_ULONG sig_len = 64;
	CK_BYTE signature[64];
	rv = C_Sign(hSession, testData, sizeof(testData), signature, &sig_len);
	if (rv != CKR_OK)
		halt(rv);

	rv = C_VerifyInit(hSession, &mech, hPubKey);
	if (rv != CKR_OK)
		halt(rv);
	rv = C_Verify(hSession, (CK_BYTE_PTR)testData, sizeof(testData), signature, sig_len);
	if (rv != CKR_OK)
		halt(rv);

	rv = C_DestroyObject(hSession, hPubKey);
	rv = C_DestroyObject(hSession, hPrvKey);

	C_CloseSession(hSession);
	C_Finalize(NULL);
}