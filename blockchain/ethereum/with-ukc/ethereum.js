'use strict'
/**
 * This is a simple program that demonstrates the use of UKC REST Api for ethereum transaction sign
 */


const log = (text) => console.log(text);

const PropertiesReader = require('properties-reader');
const properties = PropertiesReader('properties.file');

const UnboundKeyControlServer = require('./unbound_key_control_server');

/*
	configs the EKM
*/
function configEkm() {
	const defaultClient = UnboundKeyControlServer.ApiClient.instance;
	defaultClient.basePath = properties.get('ekm.url'); // make sure you use your UKC url and port

	// This is required if your server is using a self signed certificate
	process.env['NODE_TLS_REJECT_UNAUTHORIZED'] = 0;

	// Configure HTTP basic authorization: basicAuth
	const basicAuth = defaultClient.authentications['basicAuth'];
	basicAuth.username = properties.get('ekm.user') + '@' + properties.get('ekm.partition');
	basicAuth.password = properties.get('ekm.password');
}

/*
	calculates the sender address by the key-uid if no address given
*/
async function calculateFromAddress() {
	if (properties.get('web3.from')) return properties.get('web3.from');

	const keysApi = new UnboundKeyControlServer.KeysApi();
	let response
	response = await keysApi.getPublicKeyData(properties.get('ekm.key.uid'));

	const tder = require('tder');
	var ethUtils = require('ethereumjs-util');

	var td = tder.parsePem(response.keyData);
	if (td.child && td.child.length === 2) {
		const child = td.child[1];
		if (child.dataLength - child.headerLength === 64) {
			const pk = Buffer.from(child.data.substr(child.headerLength * 2), 'hex'); //;
			return ethUtils.pubToAddress(pk).toString('hex');

		} else {
			throw 'Error publicKey length should be 64!'
		}
	} else {
		throw 'Error parsing publicKey!';
	}
}

/*
	connect to web3
 */
function web3Connect() {
	const Web3 = require('web3');
	log(properties.get('web3.url'))
	var web3 = new Web3(new Web3.providers.HttpProvider(properties.get('web3.url')));
	console.log(`web3 version: ${web3.version}`);
	return web3;
}

/*
	calculate the nonce of the given source
 */
async function getNonce(from, web3) {
	var nonce = await web3.eth.getTransactionCount("0x" + from, function (err, nonce) {
		console.log(`nonce: ${nonce}`)
		return nonce;
	});
	return Number(nonce);
}


/*
	gas price
 */
function getGasPrice(web3) {
	return 10e9;
}

/*
	gas limit
 */
function getGasLimit() {
	return 25000;
}

/*
	get the chain id (return 1 if not set)
*/
function getChainId() {
	return properties.get('web3.chainid') ? properties.get('web3.chainid') : 1;
}

/*
	create the ethereum transaction
 */
function createTransaction(web3, nonce, gasPrice, gasLimit, chainId) {

	var rawTransaction = {
		"nonce": web3.utils.toHex(nonce),
		"gasLimit": web3.utils.toHex(gasLimit),
		"gasPrice": web3.utils.toHex(gasPrice),
		"to": '0x' + properties.get('web3.to'),
		"value": '0x00',
		"chainId": web3.utils.toHex(chainId),
	};

	console.log('transaction:');
	console.log(rawTransaction);

	const Tx = require('ethereumjs-tx');
	return new Tx(rawTransaction);
}

/*
	sign the transaction using EKM or Private key
 */
async function signTransaction(from, tx, chainId, useEkm) {

	if (useEkm) await ekmSign(from, tx, chainId);
	else localSign(tx)


	// validates that the signature is corresponds to the given source
	if (Buffer.from(tx.getSenderAddress()).toString('hex') != from) {
		throw "Wrong Sender address";
	}

	console.log(`signature ok: ${tx.validate()}`)
	console.log(`sender address: ${Buffer.from(tx.getSenderAddress()).toString('hex')}`);
	console.log(`sender public key : ${Buffer.from(tx.getSenderPublicKey()).toString('hex')}`);

	return '0x' + tx.serialize().toString('hex');
}

/*
	sign using local private key
*/
function localSign(tx) {
	var privateKey = Buffer.from('bbc0c793ca9eecc7ec90d5ebb3f350bb19f1f267d37c5a7c88d237c4d9b466a9', 'hex')
	tx.sign(privateKey);
}

/*
	set the v value based on the source address
*/
function setv(from, chainId, tx, sigStruct) {
	const ethUtil = require('ethereumjs-util')
	var v = 27;
	var senderAddress = ethUtil.publicToAddress(ethUtil.ecrecover(tx.hash(false), v, sigStruct.r, sigStruct.s)).toString('hex');
	if (senderAddress != from) {
		v = 28;
	}
	sigStruct.v = v + chainId * 2 + 8;
}

/*
	sign using ekm
*/
async function ekmSign(from, tx, chainId) {
	const keysApi = new UnboundKeyControlServer.KeysApi();
	var txHash = tx.hash(false).toString('hex');
	try {
		let response
		response = await keysApi.signWithKey(properties.get('ekm.key.uid'), {
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


/*
	send the singed transaction
 */
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

/*
 */
async function test() {
	configEkm();
	var from = await calculateFromAddress();
	var web3 = web3Connect();

	var nonce = await getNonce(from, web3);
	var gasPrice = await getGasPrice(web3);
	var gasLimit = getGasLimit();
	var chainId = getChainId();
	var tx = createTransaction(web3, nonce, gasPrice, gasLimit, chainId);
	var raw = await signTransaction(from, tx, chainId, properties.get('ekm.use') == true);

	sendSignedTransaction(web3, raw);
}

/*
 */
test()
	.then(() => log(`Transaction sent`))
	.catch(e => {
		log(e);
	})