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
	char password[] = ""; // ------ set your password here -------
	rv = C_Login(hSession, CKU_USER, CK_CHAR_PTR(password), CK_ULONG(strlen(password)));
	if (rv != CKR_OK)
		halt(rv);

	// generate PQC LIMA key pair
	CK_OBJECT_HANDLE hPubKey = 0;
	CK_OBJECT_HANDLE hPrvKey = 0;

	CK_MECHANISM genMech = {DYCKM_LIMA_KEY_GEN, NULL, 0};
	CK_ULONG dyckk_lima = DYCKK_LIMA;
	CK_ATTRIBUTE tPubKey[] = {
		{CKA_TOKEN, &ckFalse, sizeof(CK_BBOOL)},
		{CKA_ENCRYPT, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_KEY_TYPE, &dyckk_lima, sizeof(CK_ULONG)},
	};
	CK_ATTRIBUTE tPrvKey[] = {
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_PRIVATE, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_DECRYPT, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_KEY_TYPE, &dyckk_lima, sizeof(CK_ULONG)},
	};
	rv = C_GenerateKeyPair(hSession, &genMech, tPubKey, sizeof(tPubKey) / sizeof(CK_ATTRIBUTE), tPrvKey, sizeof(tPrvKey) / sizeof(CK_ATTRIBUTE), &hPubKey, &hPrvKey);
	if (rv != CKR_OK)
		halt(rv);

	// encrypt your secret to protect it from Post-quantum computers
	unsigned char test[32] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
	CK_MECHANISM mech_lima = {DYCKM_LIMA, 0, 0};
	rv = C_EncryptInit(hSession, &mech_lima, hPubKey);
	if (rv != CKR_OK)
		halt(rv);
	CK_ULONG out_len;
	rv = C_Encrypt(hSession, test, sizeof(test), NULL, &out_len);
	if (rv != CKR_OK)
		halt(rv);
	CK_BYTE_PTR enc = (CK_BYTE_PTR)malloc(out_len);
	rv = C_Encrypt(hSession, test, sizeof(test), enc, &out_len);
	if (rv != CKR_OK)
		halt(rv);

	C_DestroyObject(hSession, hPubKey);
	C_DestroyObject(hSession, hPrvKey);
	C_CloseSession(hSession);
	C_Finalize(NULL);
}