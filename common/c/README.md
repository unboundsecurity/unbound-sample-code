# Unbound Key Control PKCS#11 Extensions and Sample Code

Unbound provides PKCS#11 extensions and sample code as described in the following sections.

## Extensions

The Unbound PKCS#11 library supports the standard PKCS#11 specifications. It supports PCKS#11 version 2.20, but also includes some of the features of the more advanced versions, 2.30 and 2.40.

The latest version of the PCKS#11 specification can be found [here](http://docs.oasis-open.org/pkcs11/pkcs11-base/v2.40/os/pkcs11-base-v2.40-os.html).
 

In addition to the standard, UKC includes some proprietary features, which either reflect advanced crypto mechanisms not yet supported by the standard, or features that are proprietary to UKC.

The associated PKCS#11 extended key types, attributes, and mechanisms are provided in [dy_pkcs11.h](./dy_pkcs11.h) and includes the following:

1. Proprietary key attributes, such as the key UID or the previous key UID (in case of using the re-key operation).
1. Advanced symmetric cryptography:
    - AES SIV (https://tools.ietf.org/html/rfc5297).
    - AES XTS (https://en.wikipedia.org/wiki/Disk_encryption_theory#XTS)
1. Proprietary password protection mechanism.
    
	This option allows you to hash and encrypt a password and do password verification without ever having the password value or the password hash in clear memory. Password validation is done using MPC on the encrypted value, without ever decrypting it.
1. Software Defined Encryption (SDE) Features.
    
	This UKC component enables different preserving types of encryption, including:
    - [MPC based PRF](https://en.wikipedia.org/wiki/Pseudorandom_function_family) for key derivation based on metadata.
    - Functions for size, type and order preserving encryption.

1. PQC Encryption.

    UKC includes an MPC implementation of a Post Quantum Cryptography encryption algorithm called LIMA. This is a lattice based encryption scheme which is part of the NIST PQC Contest.
    
    For more information, see [here](https://csrc.nist.gov/Projects/Post-Quantum-Cryptography/Round-1-Submissions).  


1. NIST Key Derivation Function.

    Contains a NIST Key Derivation Function for Counter Mode, with a sub-mode for CMAC.
    
    For more information, see [here](https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-108.pdf).
    
1. Support EDDSA signature.

    For more information about EDDSA, see [here](https://tools.ietf.org/html/rfc8032).
	
1. Supports BIP32.

    For more information about BIP32, see [here](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki).
	
1. Enhanced destroy object.
    When destroying an object, it remains in the system, but marked as destroyed. To completely remove the object, call the function *DYC_DestroyObject* with the parameter `flags=1`.
	
1. X.509 functions.

    Includes creating a CSR, signing, and self-signing.
    

# Unbound Key Control PKCS#11 Sample Code

Sample code is provided for the following use cases:

## C examples
- [password.c](https://github.com/unboundsecurity/unbound-sample-code/tree/main/passwords/pkcs11) - proprietary password protection
- [adv_aes.c](https://github.com/unboundsecurity/unbound-sample-code/tree/main/crypto-ops/encrypt/pkcs11) - encrypt and decrypt with advanced symmetric cryptography
- [pqc_lima.c](https://github.com/unboundsecurity/unbound-sample-code/tree/main/crypto-ops/encrypt/pkcs11) - using post-quantum LIMA key
- [eddsa.c](https://github.com/unboundsecurity/unbound-sample-code/tree/main/crypto-ops/sign/pkcs11) - using EdDSA key
- [derive_nist_kdf.c](https://github.com/unboundsecurity/unbound-sample-code/tree/cca7ccdf17b1608f101146366f87721a92feeafa/crypto-ops/derive/pkcs11) - AES CMAC key derivation with NIST KDF
- [ecdsa_bip.c](https://github.com/unboundsecurity/unbound-sample-code/tree/cca7ccdf17b1608f101146366f87721a92feeafa/blockchain/bitcoin/pkcs11) - BIP32 derivation
- [wrap-unwrap.c](https://github.com/unboundsecurity/unbound-sample-code/tree/main/crypto-ops/wrap/pkcs11) - stores a wrapped encrypted key (AES) in UKC and unwraps it
- [wrap-unwrap_rsa.c](https://github.com/unboundsecurity/unbound-sample-code/tree/main/crypto-ops/wrap/pkcs11) - As above, but with RSA wrapping key
- [aes_gcm.c](https://github.com/unboundsecurity/unbound-sample-code/tree/main/crypto-ops/encrypt/pkcs11) - AES GCM operation
- [x509_sign.c](https://github.com/unboundsecurity/unbound-sample-code/tree/main/certificates/pkcs11) - X.509 certificate functions

## Java examples
- [aes_gcm.java](https://github.com/unboundsecurity/unbound-sample-code/tree/main/crypto-ops/encrypt/java/pkcs11)    - AES GCM operation
- [ecdsa.java](https://github.com/unboundsecurity/unbound-sample-code/tree/main/crypto-ops/sign/java/pkcs11)        - ECDSA operation

