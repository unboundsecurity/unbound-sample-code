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
	CK_ULONG cko_private_key = CKO_PRIVATE_KEY;
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

	// generate key for advanced password protection
	CK_OBJECT_HANDLE hKey = 0;
	CK_ULONG dyckk_adv_password = DYCKK_ADV_PASSWORD;
	const char name[] = "adv_object_name";
	CK_ATTRIBUTE tKey[] = {
		{CKO_PRIVATE_KEY, &cko_private_key, sizeof(CK_ULONG)},
		{CKA_KEY_TYPE, &dyckk_adv_password, sizeof(CK_ULONG)},
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_PRIVATE, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_ID, (CK_CHAR_PTR)name, sizeof(name) - 1},
	};
	CK_MECHANISM gen = {DYCKM_PASSWORD_KEY_GEN, 0, 0};
	rv = C_GenerateKey(hSession, &gen, tKey, sizeof(tKey) / sizeof(CK_ATTRIBUTE), &hKey);
	if (rv != CKR_OK)
		halt(rv);

	CK_BYTE plain_password[] = "test_password";
	// encrypt password
	CK_MECHANISM pwd = {DYCKM_PASSWORD, 0, 0};
	CK_ULONG enc_password_size = 0;
	CK_BYTE enc_password[1024];
	rv = C_EncryptInit(hSession, &pwd, hKey);
	if (rv != CKR_OK)
		halt(rv);
	rv = C_Encrypt(hSession, plain_password, sizeof(plain_password), NULL, &enc_password_size);
	if (rv != CKR_OK)
		halt(rv);
	rv = C_Encrypt(hSession, plain_password, sizeof(plain_password), enc_password, &enc_password_size);
	if (rv != CKR_OK)
		halt(rv);

	// safly verify the encrypted password
	rv = C_VerifyInit(hSession, &pwd, hKey);
	if (rv != CKR_OK)
		halt(rv);
	rv = C_Verify(hSession, plain_password, sizeof(plain_password), enc_password, enc_password_size);
	if (rv != CKR_OK)
		halt(rv);

	rv = C_DestroyObject(hSession, hKey);

	C_CloseSession(hSession);
	C_Finalize(NULL);
}