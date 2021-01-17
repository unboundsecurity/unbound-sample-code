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

	CK_ULONG cko_secret_key = CKO_SECRET_KEY;
	CK_ULONG ckk_generic_secret = CKK_GENERIC_SECRET;
	CK_ULONG cko_private_key = CKO_PRIVATE_KEY;
	CK_BBOOL ckFalse = CK_FALSE;
	CK_BBOOL ckTrue = CK_TRUE;
	CK_ULONG ckk_ec = CKK_EC;

	CK_BYTE secp256k_oid[] = {0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x0a};

	CK_ULONG seed_value_len = 32;
	CK_ATTRIBUTE t_new_seed_key[] =
		{
			{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
			{CKA_KEY_TYPE, &ckk_generic_secret, sizeof(CK_ULONG)},
			{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
			{CKA_DERIVE, &ckTrue, sizeof(CK_BBOOL)},
			{CKA_VALUE_LEN, &seed_value_len, sizeof(CK_ULONG)},
		};

	CK_ATTRIBUTE t_new_ec_key[] =
		{
			{CKA_CLASS, &cko_private_key, sizeof(CK_ULONG)},
			{CKA_KEY_TYPE, &ckk_ec, sizeof(CK_ULONG)},
			{CKA_TOKEN, &ckTrue, sizeof(CK_BBOOL)},
			{CKA_EC_PARAMS, secp256k_oid, sizeof(secp256k_oid)},
			{CKA_SIGN, &ckTrue, sizeof(CK_BBOOL)},
			{CKA_DERIVE, &ckTrue, sizeof(CK_BBOOL)},
		};

	// Generate seed key
	CK_MECHANISM gen = {CKM_GENERIC_SECRET_KEY_GEN, 0, 0};
	CK_OBJECT_HANDLE hSeedKey = 0;
	rv = C_GenerateKey(hSession, &gen, t_new_seed_key, sizeof(t_new_seed_key) / sizeof(CK_ATTRIBUTE), &hSeedKey);
	if (rv != CKR_OK)
		halt(rv);

	// Master derivation
	CK_MECHANISM bip32_mech_from_master = {DYCKM_DERIVE_ECDSA_BIP, NULL, 0};
	CK_OBJECT_HANDLE hMasterKey = 0;
	rv = C_DeriveKey(hSession, &bip32_mech_from_master, hSeedKey, t_new_ec_key, sizeof(t_new_ec_key) / sizeof(CK_ATTRIBUTE), &hMasterKey);
	if (rv != CKR_OK)
		halt(rv);

	// Hardened derivation
	DYCK_DERIVE_BIP_PARAMS params_hardened;
	params_hardened.hardened = CK_TRUE;
	params_hardened.ulChildNumber = CK_ULONG(12345);
	CK_MECHANISM bip32_mech_hardened = {DYCKM_DERIVE_ECDSA_BIP, &params_hardened, sizeof(DYCK_DERIVE_BIP_PARAMS)};
	CK_OBJECT_HANDLE hHardenedDerivedKey = 0;
	rv = C_DeriveKey(hSession, &bip32_mech_hardened, hMasterKey, t_new_ec_key, sizeof(t_new_ec_key) / sizeof(CK_ATTRIBUTE), &hHardenedDerivedKey);
	if (rv != CKR_OK)
		halt(rv);

	// Normal derivation
	DYCK_DERIVE_BIP_PARAMS params_normal;
	params_normal.hardened = CK_FALSE;
	params_hardened.ulChildNumber = CK_ULONG(7890);
	CK_MECHANISM bip32_mech_normal = {DYCKM_DERIVE_ECDSA_BIP, &params_normal, sizeof(DYCK_DERIVE_BIP_PARAMS)};
	CK_OBJECT_HANDLE hNormalDerivedKey = 0;
	rv = C_DeriveKey(hSession, &bip32_mech_hardened, hHardenedDerivedKey, t_new_ec_key, sizeof(t_new_ec_key) / sizeof(CK_ATTRIBUTE), &hNormalDerivedKey);
	if (rv != CKR_OK)
		halt(rv);

	// Close PKCS#11 session
	C_CloseSession(hSession);
	C_Finalize(NULL);
}