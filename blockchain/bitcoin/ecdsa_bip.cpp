#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../dy_pkcs11.h"

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
	CK_ULONG ckk_generic_secret = CKK_GENERIC_SECRET;
	CK_ULONG cko_private_key = CKO_PRIVATE_KEY;
	CK_ULONG ckk_ec = CKK_EC;

	rv = C_Initialize(NULL);
	if (rv != CKR_OK) halt(rv);
	rv = C_OpenSession(slot, CKF_SERIAL_SESSION | CKF_RW_SESSION, NULL, NULL, &hSession);
	if (rv != CKR_OK) halt(rv);

	unsigned char seed_key[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
	unsigned char k256_oid[] = { 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x0a }; //SECP256K1 curve

	// 1. Create generic secret object
	CK_ATTRIBUTE t_new_seed_key[] =	{
		{CKA_CLASS,     &cko_secret_key,     sizeof(CK_ULONG)},
		{CKA_KEY_TYPE,  &ckk_generic_secret, sizeof(CK_ULONG)},
		{CKA_TOKEN,     &ckTrue,            sizeof(CK_BBOOL)},
		{CKA_DERIVE,    &ckTrue,            sizeof(CK_BBOOL)},
		{CKA_VALUE,     seed_key,           sizeof(seed_key)},
	};
	CK_OBJECT_HANDLE seed_handle = 0;
	test_rv(C_CreateObject(hSession, t_new_seed_key, sizeof(t_new_seed_key) / sizeof(CK_ATTRIBUTE), &seed_handle));

	// 2. Derive key using ECDSA BIP mechanism
	CK_ATTRIBUTE t_new_ec_key[] =	{
		{CKA_CLASS,     &cko_private_key,  sizeof(CK_ULONG)},
		{CKA_KEY_TYPE,  &ckk_ec,           sizeof(CK_ULONG)},
		{CKA_TOKEN,     &ckTrue,          sizeof(CK_BBOOL)},
		{CKA_EC_PARAMS, k256_oid,          sizeof(k256_oid)},
		{CKA_SIGN,      &ckTrue,          sizeof(CK_BBOOL)},
		{CKA_DERIVE,    &ckTrue,          sizeof(CK_BBOOL)},
	};
	CK_MECHANISM bip_mech = { DYCKM_DERIVE_ECDSA_BIP, NULL, 0 };
	CK_OBJECT_HANDLE derived_handle = 0;
	rv = C_DeriveKey(hSession, &bip_mech, seed_handle, t_new_ec_key, sizeof(t_new_ec_key) / sizeof(CK_ATTRIBUTE), &derived_handle);

	C_CloseSession(hSession);
	C_Finalize(NULL);
}