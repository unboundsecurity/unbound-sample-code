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
	CK_SLOT_ID slot_id = 0; // default slot ID
	CK_SESSION_HANDLE hSession = 0;
	CK_RV rv;
	const char plain_data[] = "plain data";

	rv = C_Initialize(NULL);
	if (rv != CKR_OK)
		halt(rv);
	// Open PKCS#11 session
	rv = C_OpenSession(slot_id, CKF_SERIAL_SESSION | CKF_RW_SESSION, 0, 0, &hSession);
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

	// RSA generation mechanism
	CK_MECHANISM rsa_gen = {CKM_RSA_PKCS_KEY_PAIR_GEN, NULL, 0};

	// RSA key size
	CK_ULONG modulus_bits = 2048;

	CK_BBOOL ck_false = CK_FALSE;
	CK_BBOOL ck_true = CK_TRUE;

	CK_ATTRIBUTE pub_template[] =
		{
			{CKA_TOKEN, &ck_false, sizeof(CK_BBOOL)},
			{CKA_MODULUS_BITS, &modulus_bits, sizeof(CK_ULONG)},
		};

	CK_ATTRIBUTE prv_template[] =
		{
			{CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
		};

	// Generate key pair
	CK_OBJECT_HANDLE pub_key = 0, prv_key = 0;
	rv = C_GenerateKeyPair(hSession, &rsa_gen,
						   pub_template, sizeof(pub_template) / sizeof(CK_ATTRIBUTE),
						   prv_template, sizeof(prv_template) / sizeof(CK_ATTRIBUTE),
						   &pub_key, &prv_key);
	if (rv != CKR_OK)
		halt(rv);

	// Generate temporary AES key
	CK_ULONG cko_secret_key = CKO_SECRET_KEY;
	CK_ULONG ckk_aes = CKK_AES;
	CK_ULONG aes_key_length = 16;
	CK_OBJECT_HANDLE temp_aes_key;
	CK_MECHANISM gen_aes = {CKM_AES_KEY_GEN, NULL, 0};
	CK_ATTRIBUTE aes_gen_template[] =
		{
			{CKA_TOKEN, &ck_false, sizeof(CK_BBOOL)},
			{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
			{CKA_KEY_TYPE, &ckk_aes, sizeof(CK_ULONG)},
			{CKA_VALUE_LEN, &aes_key_length, sizeof(CK_ULONG)},
		};
	rv = C_GenerateKey(hSession, &gen_aes, aes_gen_template, sizeof(aes_gen_template) / sizeof(CK_ATTRIBUTE), &temp_aes_key);
	if (rv != CKR_OK)
		halt(rv);

	// Wrap AES key
	CK_RSA_PKCS_OAEP_PARAMS oaep_params = {CKM_SHA256, CKG_MGF1_SHA256, 0, NULL, 0};
	CK_MECHANISM oaep_mech = {CKM_RSA_PKCS_OAEP, &oaep_params, sizeof(CK_RSA_PKCS_OAEP_PARAMS)};
	CK_ULONG wrapped_key_len;
	rv = C_WrapKey(hSession, &oaep_mech, pub_key, temp_aes_key, NULL, &wrapped_key_len);
	if (rv != CKR_OK)
		halt(rv);
	CK_BYTE_PTR wrapped_key = (CK_BYTE_PTR)malloc(wrapped_key_len);

	rv = C_WrapKey(hSession, &oaep_mech, pub_key, temp_aes_key, wrapped_key, &wrapped_key_len);
	if (rv != CKR_OK)
		halt(rv);

	// Generate random IV
	CK_BYTE iv[16];
	rv = C_GenerateRandom(hSession, iv, sizeof(iv));
	if (rv != CKR_OK)
		halt(rv);

	// Encrypt data
	CK_GCM_PARAMS gcm_params = {iv, sizeof(iv), 0, NULL, 0, 128};
	CK_MECHANISM gcm_mech = {CKM_AES_GCM, &gcm_params, sizeof(gcm_params)};

	rv = C_EncryptInit(hSession, &gcm_mech, temp_aes_key);
	if (rv != CKR_OK)
		halt(rv);

	CK_ULONG enc_len;
	rv = C_Encrypt(hSession, (CK_BYTE_PTR)plain_data, (CK_ULONG)sizeof(plain_data), NULL, &enc_len);
	if (rv != CKR_OK)
		halt(rv);

	CK_BYTE_PTR encrypted = (CK_BYTE_PTR)malloc(enc_len);

	rv = C_Encrypt(hSession, (CK_BYTE_PTR)plain_data, (CK_ULONG)sizeof(plain_data), encrypted, &enc_len);
	if (rv != CKR_OK)
		halt(rv);

	// Destroy temporary AES key
	rv = C_DestroyObject(hSession, temp_aes_key);
	if (rv != CKR_OK)
		halt(rv);

	// Unwrap AES key
	CK_ATTRIBUTE aes_unwrap_template[] =
		{
			{CKA_TOKEN, &ck_false, sizeof(CK_BBOOL)},
			{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
			{CKA_KEY_TYPE, &ckk_aes, sizeof(CK_ULONG)},
		};
	rv = C_UnwrapKey(hSession, &oaep_mech, prv_key, wrapped_key, wrapped_key_len, aes_unwrap_template, sizeof(aes_unwrap_template) / sizeof(CK_ATTRIBUTE), &temp_aes_key);
	if (rv != CKR_OK)
		halt(rv);
	free(wrapped_key);

	// Decrypt data
	rv = C_DecryptInit(hSession, &gcm_mech, temp_aes_key);
	if (rv != CKR_OK)
		halt(rv);

	CK_ULONG dec_len;
	rv = C_Decrypt(hSession, encrypted, enc_len, NULL, &dec_len);
	if (rv != CKR_OK)
		halt(rv);

	CK_BYTE_PTR decrypted = (CK_BYTE_PTR)malloc(dec_len);
	rv = C_Decrypt(hSession, encrypted, enc_len, decrypted, &dec_len);
	if (rv != CKR_OK)
		halt(rv);

	if (dec_len != sizeof(plain_data))
		printf("Decrypted/plain length mismatch");
	if (0 != memcmp(plain_data, decrypted, dec_len))
		printf("Decrypted/plain data mismatch");

	free(encrypted);
	free(decrypted);

	// Close PKCS#11 session
	C_CloseSession(hSession);
	C_Finalize(NULL);
}