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

	// EC generation mechanism
	CK_MECHANISM ecdsa_gen = {CKM_EC_KEY_PAIR_GEN, NULL, 0};

	// ECDSA curve OID
	CK_BYTE p256_curve[] = {0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07};

	CK_BBOOL ck_false = CK_FALSE;
	CK_BBOOL ck_true = CK_TRUE;
	char key_name[] = "TEST ECDSA KEY";

	CK_ATTRIBUTE pub_template[] =
		{
			{CKA_TOKEN, &ck_false, sizeof(CK_BBOOL)},
			{CKA_EC_PARAMS, p256_curve, sizeof(p256_curve)},
		};

	CK_ATTRIBUTE prv_template[] =
		{
			{CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
			{CKA_SIGN, &ck_true, sizeof(CK_BBOOL)},
			{CKA_ID, key_name, sizeof(key_name) - 1},
		};

	// Generate key pair
	CK_OBJECT_HANDLE pub_key = 0, prv_key = 0;
	rv = C_GenerateKeyPair(hSession, &ecdsa_gen,
						   pub_template, sizeof(pub_template) / sizeof(CK_ATTRIBUTE),
						   prv_template, sizeof(prv_template) / sizeof(CK_ATTRIBUTE),
						   &pub_key, &prv_key);
	if (rv != CKR_OK)
		halt(rv);

	// Find EC key type
	CK_ULONG cko_private_key = CKO_PRIVATE_KEY;
	CK_ULONG ckk_ec = CKK_EC;
	CK_ATTRIBUTE find_template[] =
		{
			{CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
			{CKA_CLASS, &cko_private_key, sizeof(CK_ULONG)},
			{CKA_KEY_TYPE, &ckk_ec, sizeof(CK_ULONG)},
			{CKA_SIGN, &ck_true, sizeof(CK_BBOOL)},
			{CKA_ID, key_name, sizeof(key_name) - 1},
		};
	rv = C_FindObjectsInit(hSession, find_template, sizeof(find_template) / sizeof(CK_ATTRIBUTE));
	if (rv != CKR_OK)
		halt(rv);
	CK_ULONG found = 0;
	rv = C_FindObjects(hSession, &prv_key, 1, &found);
	if (rv != CKR_OK)
		halt(rv);
	rv = C_FindObjectsFinal(hSession);
	if (rv != CKR_OK)
		halt(rv);

	if (found != 1)
		printf("Error: Key not found");

	// Close PKCS#11 session
	C_CloseSession(hSession);
	C_Finalize(NULL);
}