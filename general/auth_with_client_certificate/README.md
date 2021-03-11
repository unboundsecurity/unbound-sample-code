# Authenticate using a Client Certificate without a Password

Sample code is provided to login and perform a encryption operation using a client certificate without the need to supply a password.

## Prerequisites

- Default user's password must be empty (i.e., not set).
- Client certificate PFX file
- An existing key in UKC (to be used for encryption).
- (Optional) Validating the server certificate requires:
    1. UKC Server CA *pem* file.
    2. In Windows, the server CA needs to be installed in the Trusted ROOT CA directory of current user certificates.

## Usage

- Follow the comments in [login_and_encrypt.cs](./c-sharp/login_and_encrypt.cs) and change the needed variables in **Main** function according to your environment. For example, update the certificate paths and hostname.
- Compile and run the code with no additional arguments.

To validate the server certificate, set the relevant variable in [login_and_encrypt.cs](./c-sharp/login_and_encrypt.cs) to *true*.

