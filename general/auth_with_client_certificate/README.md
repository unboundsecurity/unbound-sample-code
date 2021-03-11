# Authenticate using a Client Certificate without a Password

Sample code is provided to login and perform a encryption operation using a client certificate without the need to supply a password.

## Prerequisites

- Default user's password must be empty (i.e., not set).
- Client certificate PFX file
- An existing key in UKC (to be used for encryption).
- (Optional) Validating the server certificate requires that UKC Server CA *pem* file, and that it is installed in the Trusted ROOT CA directory of current user certificates.

## Usage

1. Compile the code.
2. Run it.
