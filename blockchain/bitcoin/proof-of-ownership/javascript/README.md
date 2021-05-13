# Unbound CASP Signature Generation for Bitcoin Proof of Ownership

This sample app shows how to use Unbound CASP to generate a signature that can be used for [Bitcoin Proof of Ownership](https://en.bitcoin.it/wiki/Proof_of_Ownership).

## Usage

1. Create a CASP sign operation with the [getSignOperation](https://www.unboundsecurity.com/docs/CASP/API/casp-byow.html#get-sign-operation) endpoint and save the data received from CASP.
1. Create the input [JSON file](https://github.com/unboundsecurity/unbound-sample-code/blob/main/blockchain/bitcoin/proof-of-ownership/javascript/casp-sign-op-complete.json) using the data from the first step.
1. Test it using your own CASP vault with the [generate script](https://github.com/unboundsecurity/unbound-sample-code/blob/main/blockchain/bitcoin/proof-of-ownership/javascript/generate.js).
