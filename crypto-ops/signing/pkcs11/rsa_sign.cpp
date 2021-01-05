#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dy_pkcs11.h"

// The sample demonstrates digital signing and dignature verification with RSA key
// signature mechanism used: PKCS with SHA-256 hashing

void halt(CK_RV rv)
{
	printf("Error %08lx\n", rv);
	exit(-1);
}

static const char rsa_sample_key_name[] = "RSASAMPLEKEYNAME";
static const int rsa_sample_key_name_len = sizeof(rsa_sample_key_name) - 1; // exclude zero termintaor

static const char data_to_sign[] = "data to sign";

void find_and_verify(CK_BYTE_PTR signature, CK_ULONG signature_len)
{
	CK_SLOT_ID slot_id = 0; // default slot ID
	CK_SESSION_HANDLE hSession = 0;
	CK_RV rv;

	// Open PKCS#11 session
	rv = C_OpenSession(slot_id, CKF_SERIAL_SESSION | CKF_RW_SESSION, 0, 0, &hSession);
	if (rv != CKR_OK)
		halt(rv);

	char password[] = ""; // ------ set your password here -------
	rv = C_Login(hSession, CKU_USER, CK_CHAR_PTR(password), CK_ULONG(strlen(password)));
	if (rv != CKR_OK)
		halt(rv);

	CK_BBOOL ck_false = CK_FALSE;
	CK_BBOOL ck_true = CK_TRUE;
	CK_ULONG ckk_rsa = CKK_RSA;
	CK_ULONG cko_public_key = CKO_PUBLIC_KEY;
	CK_ULONG cko_private_key = CKO_PRIVATE_KEY;

	CK_ATTRIBUTE find_template[] =
		{
			{CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
			{CKA_CLASS, &cko_private_key, sizeof(CK_ULONG)},
			{CKA_KEY_TYPE, &ckk_rsa, sizeof(CK_ULONG)},
			{CKA_ID, (CK_BYTE_PTR)rsa_sample_key_name, (CK_ULONG)rsa_sample_key_name_len},
		};
	CK_OBJECT_HANDLE prv_key = 0;

	// Find the private key object
	rv = C_FindObjectsInit(hSession, find_template, sizeof(find_template) / sizeof(CK_ATTRIBUTE));
	if (rv != CKR_OK)
		halt(rv);
	CK_ULONG found = 0;
	rv = C_FindObjects(hSession, &prv_key, 1, &found);
	if (rv != CKR_OK)
		halt(rv);
	C_FindObjectsFinal(hSession);
	if (rv != CKR_OK)
		halt(rv);

	CK_ATTRIBUTE pub_key_template[] =
		{
			{CKA_MODULUS, NULL, 0},
			{CKA_PUBLIC_EXPONENT, NULL, 0},
			{CKA_TOKEN, &ck_false, sizeof(CK_BBOOL)},
			{CKA_CLASS, &cko_public_key, sizeof(CK_ULONG)},
			{CKA_KEY_TYPE, &ckk_rsa, sizeof(CK_ULONG)},
		};

	// Get RSA public key material
	rv = C_GetAttributeValue(hSession, prv_key, pub_key_template, 2); // 2 first attributes, sizes only
	if (rv != CKR_OK)
		halt(rv);
	pub_key_template[0].pValue = (CK_BYTE_PTR)malloc(pub_key_template[0].ulValueLen);
	pub_key_template[1].pValue = (CK_BYTE_PTR)malloc(pub_key_template[1].ulValueLen);
	rv = C_GetAttributeValue(hSession, prv_key, pub_key_template, 2); // 2 first attributes
	if (rv != CKR_OK)
		halt(rv);

	// Create temporary RSA public key
	CK_OBJECT_HANDLE pub_key = 0;
	rv = C_CreateObject(hSession, pub_key_template, sizeof(pub_key_template) / sizeof(CK_ATTRIBUTE), &pub_key);
	if (rv != CKR_OK)
		halt(rv);

	// free temporary buffers
	free(pub_key_template[0].pValue);
	free(pub_key_template[1].pValue);

	// Verify signature
	CK_MECHANISM rsa_sign = {CKM_SHA256_RSA_PKCS, NULL, 0};
	rv = C_VerifyInit(hSession, &rsa_sign, pub_key);
	if (rv != CKR_OK)
		halt(rv);

	rv = C_Verify(hSession, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), signature, signature_len);
	if (rv != CKR_OK)
		halt(rv);

	// Close PKCS#11 session (it destroys temporary key)
	C_CloseSession(hSession);
}

int main(int argc, char *argv[])
{
	CK_SLOT_ID slot_id = 0; // default slot ID
	CK_SESSION_HANDLE hSession = 0;
	CK_RV rv;

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
			{CKA_ID, (CK_BYTE_PTR)rsa_sample_key_name, (CK_ULONG)rsa_sample_key_name_len},
		};

	// Generate key pair
	CK_OBJECT_HANDLE pub_key = 0, prv_key = 0;
	rv = C_GenerateKeyPair(hSession, &rsa_gen,
						   pub_template, sizeof(pub_template) / sizeof(CK_ATTRIBUTE),
						   prv_template, sizeof(prv_template) / sizeof(CK_ATTRIBUTE),
						   &pub_key, &prv_key);
	if (rv != CKR_OK)
		halt(rv);

	// Sign data
	CK_MECHANISM rsa_sign = {CKM_SHA256_RSA_PKCS, NULL, 0};
	rv = C_SignInit(hSession, &rsa_sign, prv_key);
	if (rv != CKR_OK)
		halt(rv);

	CK_ULONG signature_len;
	rv = C_Sign(hSession, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), NULL, &signature_len);
	if (rv != CKR_OK)
		halt(rv);

	CK_BYTE_PTR signature = (CK_BYTE_PTR)malloc(signature_len);
	rv = C_Sign(hSession, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), signature, &signature_len);
	if (rv != CKR_OK)
		halt(rv);

	// Verify signature
	rv = C_VerifyInit(hSession, &rsa_sign, pub_key);
	if (rv != CKR_OK)
		halt(rv);

	rv = C_Verify(hSession, (CK_BYTE_PTR)data_to_sign, (CK_ULONG)sizeof(data_to_sign), signature, signature_len);
	if (rv != CKR_OK)
		halt(rv);

	// Close PKCS#11 session
	C_CloseSession(hSession);

	// start another session, find the key and verify signature
	find_and_verify(signature, signature_len);

	C_Finalize(NULL);
}
