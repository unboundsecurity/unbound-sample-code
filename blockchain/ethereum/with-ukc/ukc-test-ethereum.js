// This is a simple program that demonstrates the use of UKC REST Api for ethereum transaction sign
'use strict'

const ethUtil = require('ethereumjs-util')
const tder = require('tder');
var ethUtils = require('ethereumjs-util');
const Tx = require('ethereumjs-tx').Transaction;
const Web3 = require('web3');
const axios = require('axios');

// Web3 Ethereum configuration
const WEB3_URL = 'https://mainnet.infura.io/v3/2a629a925495441c97df142905d9b5e7'
const WEB3_TO = 'b96c8438a0e08cfe9f97498d04623b79780cc7ef'

// UKC server configuration
// Make sure you configure your UKC url, port and user credentials
const UKC_PARTITION = 'casp'
const UKC_URL = 'https://localhost:8443'
const UKC_USER = 'user'
const UKC_PASSWORD = 'Unbound1!'
const UKC_KEY_UID = '88b79bbf2816cdb4'  // signing key

process.env['NODE_TLS_REJECT_UNAUTHORIZED'] = 0; // required if your server is using a self signed certificate


const ukcClient = axios.create({
	baseURL: UKC_URL + '/api/v1/keys/',
	timeout: 1000,
	headers: {
		'Content-Type': 'application/json',
		'Authorization': 'Basic' + Buffer.from(UKC_USER + '@' + UKC_PARTITION + ":" + UKC_PASSWORD).toString("base64"),
	}
});

async function getPublicKeyData(keyId) {
	try {
		const response = await ukcClient.get(keyId + '/public');
		return response.data
	} catch (error) {
		console.error(error);
	}
}
async function signWithKey(keyId, params) {
	try {
		const response = await ukcClient.post(keyId + '/sign', params.body);
		return response.data
	} catch (error) {
		console.error(error);
	}
}

// Calculates the sender address by the key-uid if no address given
async function calculateFromAddress() {
	let response = await getPublicKeyData(UKC_KEY_UID);

	var td = tder.parsePem(response.keyData);
	if (td.child && td.child.length === 2) {
		const child = td.child[1];
		if (child.dataLength - child.headerLength === 64) {
			const pk = Buffer.from(child.data.substr(child.headerLength * 2), 'hex');
			return ethUtils.pubToAddress(pk).toString('hex');

		} else {
			throw 'Error publicKey length should be 64!'
		}
	} else {
		throw 'Error parsing publicKey!';
	}
}

// Get nonce from the given source
async function getNonce(from, web3) {
	var nonce = await web3.eth.getTransactionCount("0x" + from, function (err, nonce) {
		console.log(`nonce: ${nonce}`)
		return nonce;
	});
	return Number(nonce);
}

// Create the ethereum transaction
function createTransaction(web3, nonce, gasPrice, gasLimit, chainId) {

	var rawTransaction = {
		"nonce": web3.utils.toHex(nonce),
		"gasLimit": web3.utils.toHex(gasLimit),
		"gasPrice": web3.utils.toHex(gasPrice),
		"to": '0x' + WEB3_TO,
		"value": '0x00',
		"chainId": web3.utils.toHex(chainId),
	};
	console.log('transaction:');
	console.log(rawTransaction);

	return new Tx(rawTransaction);
}


// Sign the transaction using either UKC or local key
async function signTransaction(from, tx, chainId) {

	await ukcSign(from, tx, chainId);

	// validates that the signature corresponds to the given source
	if (Buffer.from(tx.getSenderAddress()).toString('hex') != from) {
		throw "Wrong Sender address";
	}

	console.log(`signature ok: ${tx.validate()}`)
	console.log(`sender address: ${Buffer.from(tx.getSenderAddress()).toString('hex')}`);
	console.log(`sender public key : ${Buffer.from(tx.getSenderPublicKey()).toString('hex')}`);

	return '0x' + tx.serialize().toString('hex');
}

// Set the v value based on the source address
function setv(from, chainId, tx, sigStruct) {
	var v = 27;
	var senderAddress = ethUtil.publicToAddress(ethUtil.ecrecover(tx.hash(false), v, sigStruct.r, sigStruct.s)).toString('hex');
	if (senderAddress != from) {
		v = 28;
	}
	sigStruct.v = v + chainId * 2 + 8;
}

// Sign the transaction using UKC
async function ukcSign(from, tx, chainId) {
	var txHash = tx.hash(false).toString('hex');
	try {
		let response
		response = await signWithKey(UKC_KEY_UID, {
			body: {
				data: txHash,
				dataEncoding: "HEX",
				doHash: false,
			}
		});

		var sig = Buffer.from(response.signature, "base64").toString('hex');

		// builds the signature structure
		const sigStruct = {};
		sigStruct.r = Buffer.from(sig.slice(0, 64).toLowerCase(), "hex");
		sigStruct.s = Buffer.from(sig.slice(64).toLowerCase(), "hex");
		setv(from, chainId, tx, sigStruct);
		Object.assign(tx, sigStruct);
		return tx;

	} catch (e) {
		throw e;
	}
}


// Send the singed transaction
function sendSignedTransaction(web3, raw) {
	var transaction = web3.eth.sendSignedTransaction(raw);

	transaction.on('confirmation', (confirmationNumber, receipt) => {
		console.log('confirmation', confirmationNumber);
	});
	transaction.on('transactionHash', hash => {
		console.log('hash', hash);
	});
	transaction.on('receipt', receipt => {
		console.log('reciept', receipt);
		res.json({
			receipt
		});
	});
	transaction.on('error', console.error);
}

// test
async function test() {
	var from = await calculateFromAddress();
	// Configures web3 connection to the ethereum network
	var web3 = new Web3(new Web3.providers.HttpProvider(WEB3_URL));
	console.log(`web3 version: ${web3.version}`);

	var nonce = await getNonce(from, web3);
	const gasPrice = 10e9;
	const gasLimit = 25000;
	const chainId = 1;
	var tx = createTransaction(web3, nonce, gasPrice, gasLimit, chainId);
	var raw = await signTransaction(from, tx, chainId);

	sendSignedTransaction(web3, raw);
}

test()
	.then(() => console.log(`Transaction sent`))
	.catch(e => {
		console.log(e);
	})