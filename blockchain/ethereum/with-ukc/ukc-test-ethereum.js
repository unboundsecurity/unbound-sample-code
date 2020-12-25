// This is a simple program that demonstrates the use of UKC REST Api for ethereum transaction sign
'use strict'

const ethUtil = require('ethereumjs-util')
const ECKey = require('ec-key');
const EthereumTransaction = require('ethereumjs-tx').Transaction;
const Web3 = require('web3');
const axios = require('axios');

// Ignore self-signed UKC certificate validation errors.
// This should be used only for testing.
process.env['NODE_TLS_REJECT_UNAUTHORIZED'] = 0;

// The provider url for web3
// If you don't have one, get it for free from by opening an account at https://infura.io
// It is recommended to use Ropsten testnet for tests
const InfuraEndpoint = 'https://ropsten.infura.io/v3/431778d1fb804338ace2e1a5d509fb62';

// UKC server configuration
// Make sure you configure your UKC url, port and user credentials
const UKC_PARTITION = 'test'
const UKC_URL = 'https://localhost:8443'
const UKC_USER = 'ethuser'
const UKC_PASSWORD = 'Password1!'

// The name or UID of the key to use for Eth address
// The key must be of type ECC and with curve ECC-SECP_256K_1
const UKC_KEY_UID = 'eth' // 3eeca3c05c982f4d'  // signing key

const recipientAddress = '0xDC839532D5ab3e7Cc958b4F86d733CbD1617ee4A';
const amountToSendInEth = "0.0004"; // amount in Ethereum

const ukcClient = axios.create({
	baseURL: UKC_URL,
	timeout: 1000,
	headers: {
		'Content-Type': 'application/json',
		'Authorization': 'Basic' + Buffer.from(UKC_USER + '@' + UKC_PARTITION + ":" + UKC_PASSWORD).toString("base64"),
	}
});

function ethAddressFromPubKeyBuffer(buffer) {
	return "0x" + ethUtil.pubToAddress(buffer).toString('hex');
}

async function test() {
	// Get public key from UKC and convert to Ethereum address
	// The key must be of type ECC with curve SECP256K1
	var response = await ukcClient.get(`/api/v1/keys/${UKC_KEY_UID}/public`);
	var key = new ECKey(response.data.keyData, 'pem');
	var fromAddress = ethAddressFromPubKeyBuffer(Buffer.concat([key.x, key.y]));;

	// Configures web3 connection to the ethereum network
	var web3 = new Web3(new Web3.providers.HttpProvider(InfuraEndpoint));
	var balanceBefore = await web3.eth.getBalance(fromAddress);
	console.log("Sender address: " + fromAddress);
	console.log("Initial balance: " + web3.utils.fromWei(balanceBefore) + " Eth");
	var nonce = await web3.eth.getTransactionCount(fromAddress);
	var chain = await web3.eth.net.getId();
	var gasPrice = await web3.eth.getGasPrice();
	var tx = new EthereumTransaction({
		"nonce": web3.utils.toHex(nonce),
		"gasLimit": web3.utils.toHex(25000),
		"gasPrice": web3.utils.toHex(gasPrice),
		"to": recipientAddress,
		"value": web3.utils.toHex(web3.utils.toWei(amountToSendInEth))
	}, {
		chain
	});

	var txHash = tx.hash(false).toString('hex');
	response = await ukcClient.post(`/api/v1/keys/${UKC_KEY_UID}/sign`, {
		data: txHash,
		dataEncoding: "HEX",
		doHash: false,
	});

	var sig = Buffer.from(response.data.signature, "base64");
	const sigStruct = {
		r: sig.slice(0, 32),
		s: sig.slice(32)
	};
	// find recovery param by trial and error
	// According to https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md
	const recover = (v) => ethAddressFromPubKeyBuffer(ethUtil.ecrecover(tx.hash(false), v, sigStruct.r, sigStruct.s));
	var recoveryParam = [27, 28].find(v => fromAddress === recover(v));
	sigStruct.v = Buffer.from([chain * 2 + (recoveryParam -27) + 35]);
	Object.assign(tx, sigStruct);
	console.log(tx.toJSON(true));

	if(!tx.validate()) throw "Invalid signature";

	var serializedTx = '0x' + tx.serialize().toString('hex');
	console.log("\nBroadcasting transaction...");
	var transaction = await web3.eth.sendSignedTransaction(serializedTx);

	var txUrl;
	switch(chain) {
		case 3: //ropsten testnet
			txUrl = `https://ropsten.etherscan.io/tx/${transaction.transactionHash}`;
			break;
		case 1: 
			txUrl = `https://etherscan.io/tx/${transaction.transactionHash}`;
			break;
		default:
			txUrl = transaction.transactionHash;
	}
	console.log("Transaction sent successfully:\n" + txUrl );
	
	return transaction;
}

test().catch(e => console.log(e.toString()))