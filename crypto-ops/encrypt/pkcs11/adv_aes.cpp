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

	// generate AES XTS key
	CK_ULONG cko_secret_key = CKO_SECRET_KEY;
	CK_OBJECT_HANDLE hKey = 0;
	CK_ULONG key_size = 32;
	CK_ULONG key_type = DYCKK_AES_XTS;
	CK_ATTRIBUTE t_aes[] = {
		{CKA_PRIVATE, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
		{CKA_KEY_TYPE, &key_type, sizeof(CK_ULONG)},
		{CKA_ENCRYPT, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_DECRYPT, &ckTrue, sizeof(CK_BBOOL)},
		{CKA_VALUE_LEN, &key_size, sizeof(CK_ULONG)},
	};
	CK_MECHANISM xts_gen_mech = {DYCKM_AES_XTS_KEY_GEN, 0, 0};
	rv = C_GenerateKey(hSession, &xts_gen_mech, t_aes, sizeof(t_aes) / sizeof(CK_ATTRIBUTE), &hKey);
	if (rv != CKR_OK)
		halt(rv);

	// encrypt with the AES XTS key
	unsigned char plain_data[32] = {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
	unsigned char iv[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6};
	CK_MECHANISM mech = {DYCKM_AES_XTS, iv, sizeof(iv)};
	rv = C_EncryptInit(hSession, &mech, hKey);
	if (rv != CKR_OK)
		halt(rv);
	CK_ULONG enc_size = 0;
	rv = C_Encrypt(hSession, plain_data, sizeof(plain_data), NULL, &enc_size);
	if (rv != CKR_OK)
		halt(rv);
	CK_BYTE_PTR enc_data = (CK_BYTE_PTR)malloc(enc_size);
	rv = C_Encrypt(hSession, plain_data, sizeof(plain_data), enc_data, &enc_size);
	if (rv != CKR_OK)
		halt(rv);

	// decrypt it back
	rv = C_DecryptInit(hSession, &mech, hKey);
	if (rv != CKR_OK)
		halt(rv);
	CK_ULONG dec_size = 0;
	rv = C_Decrypt(hSession, enc_data, enc_size, NULL, &dec_size);
	if (rv != CKR_OK)
		halt(rv);
	CK_BYTE_PTR dec_data = (CK_BYTE_PTR)malloc(dec_size);
	rv = C_Decrypt(hSession, enc_data, enc_size, dec_data, &dec_size);
	if (rv != CKR_OK)
		halt(rv);

	rv = C_DestroyObject(hSession, hKey);
	if (rv != CKR_OK)
		halt(rv);
	C_CloseSession(hSession);
	C_Finalize(NULL);
}