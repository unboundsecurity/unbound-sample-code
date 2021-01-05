
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cryptoki.h"

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

	char wrapping_key_name[] = "Wrapping key";
	CK_ULONG wrapping_key_len = (CK_ULONG)strlen(wrapping_key_name);
	char wrapped_key_name[] = "Wrapped key";
	CK_ULONG wrapped_key_len = (CK_ULONG)strlen(wrapped_key_name);

	CK_ULONG ckk_aes = CKK_AES;
	CK_ULONG cko_secret_key = CKO_SECRET_KEY;
	CK_BBOOL ck_true = CK_TRUE;
	CK_BBOOL ck_false = CK_FALSE;
	CK_ULONG value_len = 32;

	CK_ATTRIBUTE t_create_wrapping_key[] = {
		{CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
		{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
		{CKA_KEY_TYPE, &ckk_aes, sizeof(CK_ULONG)},
		{CKA_VALUE_LEN, &value_len, sizeof(CK_ULONG)},
		{CKA_LABEL, wrapping_key_name, (CK_ULONG)strlen(wrapping_key_name)},
	};

	CK_ATTRIBUTE t_create_wrapped_key[] = {
		{CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
		{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
		{CKA_KEY_TYPE, &ckk_aes, sizeof(CK_ULONG)},
		{CKA_VALUE_LEN, &value_len, sizeof(CK_ULONG)},
		{CKA_EXTRACTABLE, &ck_true, sizeof(CK_BBOOL)},
		{CKA_WRAP_WITH_TRUSTED, &ck_false, sizeof(CK_BBOOL)},
		{CKA_LABEL, wrapped_key_name, (CK_ULONG)strlen(wrapped_key_name)},
	};

	CK_ATTRIBUTE t_unwrapped_key[] = {
		{CKA_TOKEN, &ck_true, sizeof(CK_BBOOL)},
		{CKA_CLASS, &cko_secret_key, sizeof(CK_ULONG)},
		{CKA_KEY_TYPE, &ckk_aes, sizeof(CK_ULONG)},
		{CKA_LABEL, wrapped_key_name, (CK_ULONG)strlen(wrapped_key_name)},
	};

	CK_MECHANISM wrap_mech = {CKM_AES_ECB, NULL, 0};
	CK_MECHANISM gen_mech = {CKM_AES_KEY_GEN, NULL, 0};

	CK_ULONG wrapped_data_len;
	CK_OBJECT_HANDLE hWrappingKey = CK_INVALID_HANDLE;
	CK_OBJECT_HANDLE hWrappedKey = CK_INVALID_HANDLE;
	CK_OBJECT_HANDLE hUnwrappedKey = CK_INVALID_HANDLE;

	CK_BYTE_PTR wrapped_data = NULL;

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

	// Generate a wrapping key
	rv = C_GenerateKey(hSession, &gen_mech, t_create_wrapping_key, sizeof(t_create_wrapping_key) / sizeof(CK_ATTRIBUTE), &hWrappingKey);
	if (rv != CKR_OK)
		halt(rv);

	// Generate a key to be wrapped
	rv = C_GenerateKey(hSession, &gen_mech, t_create_wrapped_key, sizeof(t_create_wrapped_key) / sizeof(CK_ATTRIBUTE), &hWrappedKey);
	if (rv != CKR_OK)
		halt(rv);

	// wrap key
	rv = C_WrapKey(hSession, &wrap_mech, hWrappingKey, hWrappedKey, NULL, &wrapped_data_len);
	if (rv != CKR_OK)
		halt(rv);

	wrapped_data = (CK_BYTE_PTR)malloc(wrapped_data_len);
	rv = C_WrapKey(hSession, &wrap_mech, hWrappingKey, hWrappedKey, wrapped_data, &wrapped_data_len);
	if (rv != CKR_OK)
		halt(rv);

	// destroy original key
	rv = C_DestroyObject(hSession, hWrappedKey);
	if (rv != CKR_OK)
		halt(rv);

	// unwrap key
	rv = C_UnwrapKey(hSession, &wrap_mech, hWrappingKey, wrapped_data, wrapped_data_len, t_unwrapped_key, sizeof(t_unwrapped_key) / sizeof(CK_ATTRIBUTE), &hUnwrappedKey);
	if (rv != CKR_OK)
		halt(rv);

	C_CloseSession(hSession);
	C_Finalize(NULL);
}
