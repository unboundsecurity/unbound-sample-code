#ifndef _DY_PKCS11_H_
#define _DY_PKCS11_H_

#include "cryptoki.h"

#ifdef __cplusplus
extern "C" {
#endif

  // -----------------------------------------
  // DYCKK - vendor-specific key types:
  // -----------------------------------------

  // Advanced password key type
#define DYCKK_ADV_PASSWORD                    ((unsigned)0x80007101)

// Advanced PRF key type                 
#define DYCKK_ADV_PRF                         ((unsigned)0x80007103)

// AES SIV key type                      
#define DYCKK_AES_SIV                         ((unsigned)0x80007104)

// AES XTS key type                      
#define DYCKK_AES_XTS                         ((unsigned)0x80007105)

// Lima key type                         
#define DYCKK_LIMA                            ((unsigned)0x80007107)

// EDDSA key type                        
#define DYCKK_EDDSA                           ((unsigned)0x80007108)


// -----------------------------------------
// DYCKA - vendor specific attributes:
// -----------------------------------------

// Unique identifier
#define DYCKA_UID                             ((unsigned)0x80007201)

// replaced unique identifier          
#define DYCKA_REPLACED_UID                     ((unsigned)0x80007205)

// Lima public key                          
#define DYCKA_LIMA_PUB_KEY                    ((unsigned)0x80007207)

// Groups the object belongs to
#define DYCKA_OBJECT_GROUPS              ((unsigned)0x8000720f)

// ECDSA BIP attributes
#define DYCKA_ECDSA_BIP_LEVEL                 ((unsigned)0x80007210)
#define DYCKA_ECDSA_BIP_CHILD_NUMBER          ((unsigned)0x80007211)
#define DYCKA_ECDSA_BIP_PARENT_FINGERPRINT    ((unsigned)0x80007212)
#define DYCKA_ECDSA_BIP_CPAR                  ((unsigned)0x80007213)
#define DYCKA_ECDSA_BIP_PARENT_UID            ((unsigned)0x80007214)
#define DYCKA_ECDSA_BIP_HARDENED              ((unsigned)0x80007215)

// EDDSA BIP attributes
#define DYCKA_EDDSA_BIP_LEVEL                 ((unsigned)0x80007230)
#define DYCKA_EDDSA_BIP_CHILD_NUMBER          ((unsigned)0x80007231)
#define DYCKA_EDDSA_BIP_CPAR                  ((unsigned)0x80007233)
#define DYCKA_EDDSA_BIP_PARENT_UID            ((unsigned)0x80007234)
#define DYCKA_EDDSA_BIP_HARDENED              ((unsigned)0x80007235)

// Object state
#define DYCKA_STATE                           ((unsigned)0x80007224)

// Object activation date
#define DYCKA_ACTIVATION_DATE                 ((unsigned)0x80007225)

// Object deactivation date
#define DYCKA_DEACTIVATION_DATE               ((unsigned)0x80007226)

// Object destroy date
#define DYCKA_DESTROY_DATE                    ((unsigned)0x80007227)

// Object process start date
#define DYCKA_PROCESS_START_DATE              ((unsigned)0x80007228)

// Object protect stop date
#define DYCKA_PROTECT_STOP_DATE               ((unsigned)0x80007229)

// Object compromise date
#define DYCKA_COMPROMISE_DATE                 ((unsigned)0x8000722A)

// Object compromise occurence date
#define DYCKA_COMPROMISE_OCC_DATE             ((unsigned)0x8000722B)

// Object revocation reason
#define DYCKA_REVOCATION_REASON               ((unsigned)0x8000722C)

// Object revocation message
#define DYCKA_REVOCATION_MESSAGE              ((unsigned)0x8000722D)

// External storage flag
#define DYCKA_IS_EXTERNAL					            ((unsigned)0x80007250)

// External key store name
#define DYCKA_KEYSTORE       				          ((unsigned)0x80007251)

// External object ID
#define DYCKA_KEYSTORE_OBJECTID				        ((unsigned)0x80007252)

// External bring your own key flag
#define DYCKA_KEYSTORE_IS_BYOK				        ((unsigned)0x80007253)


// -----------------------------------------
// DYCKM - vendor specific mechanisms:
// -----------------------------------------

// AES SIV
#define DYCKM_AES_SIV                         ((unsigned)0x80007e01)

// AES SIV key generation                   
#define DYCKM_AES_SIV_KEY_GEN                 ((unsigned)0x80007e02)

// Advanced PRF                             
#define DYCKM_PRF                             ((unsigned)0x80007e11)

// Advanced PRF key generation              
#define DYCKM_PRF_KEY_GEN                     ((unsigned)0x80007e12)

// ECIES                                    
#define DYCKM_ECIES                           ((unsigned)0x80007e13)

// Format Preserving Encryption             
#define DYCKM_FPE                             ((unsigned)0x80007e14) // format preserving

// Order Preserving Encryption              
#define DYCKM_OPE                             ((unsigned)0x80007e15) // order preserving

// Size Preserving Encryption               
#define DYCKM_SPE                             ((unsigned)0x80007e16) // size preserving

// NIST Key Derivation Function, Counter Mode, CMAC
#define DYCKM_NIST_KDF_CMAC_CTR               ((unsigned)0x80007e17) 

// Advanced password encryption
#define DYCKM_PASSWORD                        ((unsigned)0x80007e21)

// Advanced password encryption key generation
#define DYCKM_PASSWORD_KEY_GEN                ((unsigned)0x80007e22)

// AES XTS                                  
#define DYCKM_AES_XTS                         ((unsigned)0x80007e41)

// AES XTS key generation                   
#define DYCKM_AES_XTS_KEY_GEN                 ((unsigned)0x80007e42)

// Lima                                     
#define DYCKM_LIMA                            ((unsigned)0x80007e51)

// Lima key generation                      
#define DYCKM_LIMA_KEY_GEN                    ((unsigned)0x80007e52)

// DES3 X919 MAC                            
#define DYCKM_DES3_X919_MAC                   ((unsigned)0x80007ea0)

// EDDSA key generation                     
#define DYCKM_EDDSA_KEY_GEN                   ((unsigned)0x80007e82)

// EDDSA signature                          
#define DYCKM_EDDSA                           ((unsigned)0x80007e81)

// ECDSA BIP key derivation                 
#define DYCKM_DERIVE_ECDSA_BIP         	      ((unsigned)0x80007e60)

// EDDSA BIP key derivation                 
#define DYCKM_DERIVE_EDDSA_BIP                ((unsigned)0x80007e61)



// ----------------------------------------------
// DYCK_FPE - Format Preserving Encryption types:
// ----------------------------------------------
#define DYCK_FPE_EMAIL       1
#define DYCK_FPE_CREDIT_CARD 2
#define DYCK_FPE_US_PHONE    3
#define DYCK_FPE_SSN         4
#define DYCK_FPE_STRING      5

// ----------------------------------------------
// DYCK_STATE - Object states:
// ----------------------------------------------
#define DYCK_STATE_ANY                   CK_UNAVAILABLE_INFORMATION
#define DYCK_STATE_PREACTIVE             1
#define DYCK_STATE_ACTIVE                2
#define DYCK_STATE_DEACTIVATED           3
#define DYCK_STATE_COMPROMIZED           4
#define DYCK_STATE_DESTROYED             5
#define DYCK_STATE_DESTROYED_COMPROMIZED 6


// ----------------------------------------------
// DYCK - Vendor-specific structures:
// ----------------------------------------------

// Format preserving encryption mechanism parameters
  typedef struct DYCK_FPE_PARAMS
  {
    CK_ULONG       ulMode;          // DYCK_FPE_*
    CK_CHAR_PTR    pFormat;
    CK_ULONG       ulMaxLen;
  } DYCK_FPE_PARAMS;

  // Size preserving encryption mechanism parameters
  typedef struct DYCK_SPE_PARAMS
  {
    CK_ULONG       ulBits;          // length of data in bits
  } DYCK_SPE_PARAMS;

  // PRF mechanism parameters
  typedef struct DYCK_PRF_PARAMS
  {
    CK_ULONG      ulPurpose;
    CK_BYTE_PTR   pTweak;
    CK_ULONG      ulTweakLen;
    CK_ULONG      ulSecretLen;
  } DYCK_PRF_PARAMS;

  // NIST KDF CMAC CTR parameters
  typedef struct DYCK_NIST_KDF_CMAC_CTR_PARAMS
  {
    CK_BYTE_PTR pLabel;
    CK_ULONG ulLabelLen;
    CK_BYTE_PTR pContext;
    CK_ULONG ulContextLen;
    CK_ULONG ulSecretLen;
  } DYCK_NIST_KDF_CMAC_CTR_PARAMS;

  // SIV data structure
  typedef struct DYCK_DATA {
    CK_BYTE_PTR pData;
    CK_ULONG    ulLen;
  } DYCK_DATA;

  // SIV mechanism parameters
  typedef struct DYCK_AES_SIV_PARAMS {
    CK_ULONG ulAuthCount;
    DYCK_DATA* pAuthData;
  } DYCK_AES_SIV_PARAMS;

  // BIP derivation parameters
  typedef struct DYCK_DERIVE_BIP_PARAMS {
    CK_BBOOL hardened;
    CK_ULONG ulChildNumber;
  } DYCK_DERIVE_BIP_PARAMS;


  // ----------------------------------------------
  // DYC - Vendor-specific functions:
  // ----------------------------------------------


  // X509 signing helper
  CK_RV CK_EXPORT_SPEC CK_CALL_SPEC DYC_SignX509(
    CK_SESSION_HANDLE      hSession,
    CK_OBJECT_HANDLE       hPrivateKey,
    CK_BYTE_PTR            pX509CA,
    CK_ULONG               ulX509CA,
    CK_MECHANISM_TYPE      hashAlg,
    CK_BYTE_PTR            pX509Request,
    CK_ULONG               ulX509Request,
    CK_BYTE_PTR            pSerial,
    CK_ULONG               ulSerial,
    CK_ULONG               days,
    CK_BYTE_PTR            pX509,
    CK_ULONG_PTR           pulX509Len);

  // X509 self-sign certificate helper  
  CK_RV CK_EXPORT_SPEC CK_CALL_SPEC DYC_SelfSignX509(
    CK_SESSION_HANDLE      hSession,
    CK_OBJECT_HANDLE       hPrivateKey,
    CK_MECHANISM_TYPE      hashAlg,
    CK_CHAR_PTR            pSubjectStr,
    CK_BYTE_PTR            pSerial,
    CK_ULONG               ulSerial,
    CK_ULONG               days,
    CK_BYTE_PTR            pX509,
    CK_ULONG_PTR           pulX509Len);

  // Create PKCS#10 certificate request helper
  CK_RV CK_EXPORT_SPEC CK_CALL_SPEC DYC_CreateX509Request(
    CK_SESSION_HANDLE      hSession,
    CK_OBJECT_HANDLE       hPrivateKey,
    CK_MECHANISM_TYPE      hashAlg,
    CK_CHAR_PTR            pSubjectStr,
    CK_BYTE_PTR            pCSR,
    CK_ULONG_PTR           pulCSRLen);

  // PKCS#7 signing helper
  CK_RV CK_EXPORT_SPEC CK_CALL_SPEC DYC_SignPKCS7(
    CK_SESSION_HANDLE      hSession,
    CK_OBJECT_HANDLE       hCert,
    CK_MECHANISM_TYPE      hashAlg,
    CK_BYTE_PTR            pData,
    CK_ULONG               ulData,
    CK_BBOOL               bDetached,
    CK_BYTE_PTR            pSignedPKCS7,
    CK_ULONG_PTR           pulSignedPKCS7);

  // Set integrity check  
  CK_RV CK_EXPORT_SPEC CK_CALL_SPEC DYC_SetIntegrityCheck(
    CK_BBOOL               on);

  // Extended destroy object  
  CK_RV CK_EXPORT_SPEC CK_CALL_SPEC DYC_DestroyObject(
    CK_SESSION_HANDLE hSession,   // the session's handle 
    CK_OBJECT_HANDLE  hObject,    // the object's handle 
    CK_FLAGS flags);              // full delete flag 



#ifdef __cplusplus
} // extern "C" 
#endif

#endif // _DY_PKCS11_H_
