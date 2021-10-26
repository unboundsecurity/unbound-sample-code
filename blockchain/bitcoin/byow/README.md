# CASP BYOW Bitcoin JavaScript Demo

This demo is a terminal application that shows how to use the CASP (Crypto Asset Security Platform) API from an external Bitcoin wallet for creating and signing transactions.

CASP provides the necessary APIs so that you can bring your own wallet (**BYOW**), meaning that you can use whatever ledger you have, and control the vault and key operations with CASP. Using BYOW, you can create an implementation that can handle any coin type, as well as any special operations that you use to communicate with your ledger and for ledger processing.

More info and samples about BYOW can be found [here](https://www.unboundsecurity.com/docs/CASP/API/casp-byow.html)

## Overview
This demo shows the following:
- **Connecting to CASP**   
Connect to CASP and authenticate with bearer token
- **Accounts and Participants**
- **Vaults**    
Create a vault and activate it by joining a participant   
- **Generating Bitcoin address**   
Use CASP to generate a new ECDSA key-pair and use it to generate a new Bitcoin address
- **Deposit**   
Poll an address for balance with Blockset
- **Withdrawal**   
Create a Bitcoin transaction to withdraw funds from a CASP generated address
- **Signature**   
Request a signature approval from vault participants and use the signature to sign the Bitcoin transaction
- **Send transaction to ledger**

## Usage
### Requirements
* A running CASP server
* A Blockset Token (get it from [here](https://blockset.com))
* Node JS LTS

### Installation
* Get the source code
    ```
    $ git clone git@github.com:unboundsecurity/unbound-sample-code.git
    ```
* Install dependencies
    ```
    $ cd blockchain/bitcoin/byow
    $ npm install
    ```
* Run the demo
    ```
    $ npm start
    ```
    During the demo you will be asked to deposit test Bitcoin into an address created by CASP.
    This can be done using one of the online Bitcoin testnet faucets such as this: https://testnet-faucet.mempool.co
* Show curl commands  
    To show communication to the server as `curl` commands use the following command on Linux based systems
    ```
    $ DEBUG=super-curl npm start
    ```
* Reset the demo data 
    If you want to run the demo again from scratch you need to delete the file ``` data/data.json ```
    This could be done by running
    ```
    npm run reset
    ```
