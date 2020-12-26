# Ethereum with UKC Demo
This sample shows how to create an Ethereum wallet using Unbound UKC and make an outward transaction, sending funds to an external address.
This demo is a NodeJS javascript program.
## Overview
This demo does the following:

* Create a UKC partition and a key suitable for Ethereum 
* Create an Ethereum transaction
* Use UKC to sign that transaction
* Combine the signature and transaction and post them to the Ethereum network

## Usage
### Requirements
* A running UKC server
* An Infura endpoint URL (recommended ropsten network). Get it from [here](https://infura.io)
* NodeJS LTS

### Installation and configuration
* Install dependencies
    ```
    $ npm install
    ```
* Configure addresses and credentials in the *properties.file*
	
### Running the demo
```
$ npm start
```
### Getting test Ethereum funds
The demo creates an Ethereum address and print it to the screen.
You may want to deposit some test Eth funds to that address so you can then send them to another address.
You can get test Eth from here: https://faucet.metamask.io/
