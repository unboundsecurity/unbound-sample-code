#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cryptoki.h"

static void halt(CK_RV rv)
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

	CK_ULONG ckk_aes = CKK_AES;
	CK_ULONG cko_secret_key = CKO_SECRET_KEY;
	CK_BBOOL ck_true = CK_TRUE;
	CK_BBOOL ck_false = CK_FALSE;
	CK_ULONG value_len = 32;

	CK_BYTE iv[12];

	CK_ATTRIBUTE t_key[] = {
		{CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
		{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
		{CKA_KEY_TYPE, &ckk_aes, sizeof(CK_ULONG)},
		{CKA_VALUE_LEN, &value_len, sizeof(CK_ULONG)},
	};

	CK_MECHANISM gen_mech = {CKM_AES_KEY_GEN, NULL, 0};

	CK_GCM_PARAMS gcm_params;
	CK_MECHANISM mech = {CKM_AES_GCM, &gcm_params, sizeof(CK_GCM_PARAMS)};

	CK_OBJECT_HANDLE hKey = CK_INVALID_HANDLE;

	char plain_data[] = "TEST PLAIN DATA";
	CK_ULONG plain_data_len = sizeof(plain_data);

	CK_BYTE encrypted_data[sizeof(plain_data) + 100];
	CK_ULONG encrypted_data_len = sizeof(encrypted_data);

	char decrypted_data[sizeof(plain_data)];
	CK_ULONG decrypted_data_len = sizeof(decrypted_data);

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

	// Generate aes key
	rv = C_GenerateKey(hSession, &gen_mech, t_key, sizeof(t_key) / sizeof(CK_ATTRIBUTE), &hKey);
	if (rv != CKR_OK)
		halt(rv);

	// Generate random IV
	rv = C_GenerateRandom(hSession, iv, sizeof(iv));
	if (rv != CKR_OK)
		halt(rv);

	// Encrypt
	memset(&gcm_params, 0, sizeof(CK_GCM_PARAMS));
	gcm_params.pIv = iv;
	gcm_params.ulIvLen = sizeof(iv);
	gcm_params.ulTagBits = 96;

	rv = C_EncryptInit(hSession, &mech, hKey);
	if (rv != CKR_OK)
		halt(rv);

	rv = C_Encrypt(hSession, (CK_BYTE_PTR)plain_data, plain_data_len, encrypted_data, &encrypted_data_len);
	if (rv != CKR_OK)
		halt(rv);

	// Decrypt
	rv = C_DecryptInit(hSession, &mech, hKey);
	if (rv != CKR_OK)
		halt(rv);

	rv = C_Decrypt(hSession, encrypted_data, encrypted_data_len, (CK_BYTE_PTR)decrypted_data, &decrypted_data_len);
	if (rv != CKR_OK)
		halt(rv);

	if (decrypted_data_len != plain_data_len ||
		0 != memcmp(decrypted_data, plain_data, plain_data_len))
	{
		printf("Decrypted data mismatch\n");
		exit(-1);
	}

	// destroy aes key
	rv = C_DestroyObject(hSession, hKey);
	if (rv != CKR_OK)
		halt(rv);

	C_CloseSession(hSession);
	C_Finalize(NULL);
	return 0;
}
