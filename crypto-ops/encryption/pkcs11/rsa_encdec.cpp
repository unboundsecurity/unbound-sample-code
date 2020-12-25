#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cryptoki.h"

static void halt(CK_RV rv)
{
	printf("Error %08lx\n", rv);
	exit(-1);
}

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

	char password[] = ""; // ------ set your password here -------
	rv = C_Login(hSession, CKU_USER, CK_CHAR_PTR(password), CK_ULONG(strlen(password)));
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

	// Encrypt data
	CK_RSA_PKCS_OAEP_PARAMS oaep = {CKM_SHA256, CKG_MGF1_SHA256, 0, NULL, 0};
	CK_MECHANISM oaep_mech = {CKM_RSA_PKCS_OAEP, &oaep, sizeof(CK_RSA_PKCS_OAEP_PARAMS)};
	rv = C_EncryptInit(hSession, &oaep_mech, pub_key);
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

	// Decrypt data
	rv = C_DecryptInit(hSession, &oaep_mech, prv_key);
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
	else if (0 != memcmp(plain_data, decrypted, dec_len))
		printf("Decrypted/plain data mismatch");

	free(encrypted);
	free(decrypted);

	C_DestroyObject(hSession, pub_key);
	C_DestroyObject(hSession, prv_key);
	// Close PKCS#11 session
	C_CloseSession(hSession);
	C_Finalize(NULL);
}