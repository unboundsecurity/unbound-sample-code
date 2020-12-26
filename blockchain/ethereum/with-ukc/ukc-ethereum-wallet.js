/**
 * This sample shows how to create an Ethereum transaction and sign it with a key stored in UKC
 */
'use strict'

const ethUtil = require('ethereumjs-util')
const ECKey = require('ec-key');
const EthereumTransaction = require('ethereumjs-tx').Transaction;
const Web3 = require('web3');
const axios = require('axios');

// Ignore self-signed UKC certificate validation. Use only for testing.
process.env['NODE_TLS_REJECT_UNAUTHORIZED'] = 0;
const DEFAULT_PASSWORD = 'Unbound1!';

const ethAddressFromPublicKey = (rawKeybuffer) => "0x" + ethUtil.pubToAddress(rawKeybuffer).toString('hex');

const print = (message) => {
	const ukcErrorData = message.response && message.response.data;
	console.log(ukcErrorData || message.toString());
}

module.exports = class UkcEthereumWallet {
	/**
	 * @param {string} [web3ProviderUrl] - The endpoint URL for the web3 provider. See
	 * @param {string} [ukcHost] - The host url of UKC server, default to https://localhost
	 * @param {string} [partitionName] - The partition to use for key storage. 
	 * @param {string} [keyName] - The name of the key to find or create.
	 * @param {string} partitionUserName - The partition user name with role 'user'.
	 * @param {string} [partitionUserPassword] - The partition user password.
	 * @param {string} [rootSoPassword] - Root SO (System admin) password.
	 * @param {string} [partitionSoPassword] - Partition SO (Partition admin) password.
	 */
	constructor(web3ProviderUrl, ukcHost, partitionName, keyName, partitionUserName, partitionUserPassword,
		rootSoPassword, partitionSoPassword) {
		if (!web3ProviderUrl) {
			throw "Must specify a web3 provider. See https://infura.io";
		}
		this.web3ProviderUrl = web3ProviderUrl;
		this.ukcHost = ukcHost || "https://localhost";
		this.ukcApiBase = ukcHost + "/api/v1"
		this.partitionName = partitionName || "eth-test";
		this.keyName = keyName || "eth1";
		this.partitionUserName = partitionUserName || "ethuser";
		this.rootSoPassword = rootSoPassword || DEFAULT_PASSWORD;
		this.partitionSoPassword = partitionSoPassword || DEFAULT_PASSWORD;
		this.partitionUserPassword = partitionUserPassword || DEFAULT_PASSWORD;
	}

	/**
	 * Verifies that the key exists and valid.
	 * If the key or partition don't exist they will be auto created.
	 */
	async init() {
		const rootSoUsername = "so";
		const ukcAdmin = axios.create({
			baseURL: this.ukcApiBase,
			auth: {
				username: rootSoUsername + "@root",
				password: this.rootSoPassword
			}
		});
		const partitionName = this.partitionName;
		try {
			await ukcAdmin.get(`partitions/${partitionName}`);
		} catch (e) {
			if ((e.response || {}).status != 400) throw e;
			print(`Partition not found. Creating partition '${partitionName}'`);
			await ukcAdmin.post(`partitions`, {
				name: partitionName,
				soPassword: this.partitionSoPassword,
				newClient: {
					name: "default"
				}
			})
		}

		const partitionAdmin = axios.create({
			baseURL: this.ukcApiBase,
			auth: {
				username: "so@" + partitionName,
				password: this.partitionSoPassword
			}
		});
		const keyName = this.keyName;
		try {
			const keyFormat = (await partitionAdmin.get(`keys/${keyName}`)).data.keyFormat;
			if (keyFormat.type !== "ECC" || keyFormat.curve !== "SECP_256K_1") {
				throw `Key '${keyName}' not compatible with Ethereum. Must be ECC SECP_256K_1, but its ${JSON.stringify(keyFormat)}`;
			};
		} catch (e) {
			if ((e.response || {}).status != 400) throw e;
			print(`Key not found. Creating key '${keyName}'`);
			await partitionAdmin.post(`keys/generate`, {
				keyId: keyName,
				keyFormat: {
					curve: "SECP_256K_1",
					type: "ECC"
				}
			})
		}
		const partitionUserName = this.partitionUserName;
		try {
			await partitionAdmin.get(`users/${partitionUserName}`);
		} catch (e) {
			if (e.response.status != 400) throw e;
			print(`Creating partition user: '${partitionUserName}'`)
			await partitionAdmin.post('users', {
				name: partitionUserName,
				role: 'user',
				password: this.partitionUserPassword
			})
		}
	}

	/**
	 * Sends Ethereum from this wallet to another address.
	 */
	async send(transactionParams) {
		const partitionUser = axios.create({
			baseURL: this.ukcApiBase,
			timeout: 1000,
			auth: {
				username: `${this.partitionUserName}@${this.partitionName}`,
				password: this.partitionUserPassword
			}
		});

		// Get public key from UKC and convert to Ethereum address
		// The key must be of type ECC with curve SECP256K1
		var response = await partitionUser.get(`keys/${this.keyName}/public`);
		var key = new ECKey(response.data.keyData, 'pem');
		var fromAddress = ethAddressFromPublicKey(Buffer.concat([key.x, key.y]));;

		// Prepare the unsigned transaction
		var web3 = new Web3(new Web3.providers.HttpProvider(this.web3ProviderUrl));
		var balanceBefore = await web3.eth.getBalance(fromAddress);
		print("Sender address: " + fromAddress);
		print("Current balance: " + web3.utils.fromWei(balanceBefore) + " Eth");
		var nonce = await web3.eth.getTransactionCount(fromAddress);
		var chain = await web3.eth.net.getId();
		print("Using ethereum network: " + await web3.eth.net.getNetworkType())
		var gasPrice = await web3.eth.getGasPrice();
		var tx = new EthereumTransaction({
			"nonce": web3.utils.toHex(nonce),
			"gasLimit": web3.utils.toHex(25000),
			"gasPrice": web3.utils.toHex(gasPrice),
			"to": transactionParams.to,
			"value": web3.utils.toHex(web3.utils.toWei(transactionParams.amountInEth))
		}, {
			chain
		});

		// Get signature from UKC and apply to the transaction
		var txHash = tx.hash(false).toString('hex');
		response = await partitionUser.post(`keys/${this.keyName}/sign`, {
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
		const recover = (v) => ethAddressFromPublicKey(ethUtil.ecrecover(tx.hash(false), v, sigStruct.r, sigStruct.s));
		var recoveryParam = [27, 28].find(v => fromAddress === recover(v));
		sigStruct.v = Buffer.from([chain * 2 + (recoveryParam - 27) + 35]);
		Object.assign(tx, sigStruct);
		print("\nTx data:\n" + JSON.stringify(tx.toJSON(true), undefined, 2));
		if (!tx.validate()) throw "Invalid signature";

		// Broadcast the signed transaction to the Eth network
		var serializedTx = '0x' + tx.serialize().toString('hex');
		print("\nBroadcasting transaction...");
		var transaction = await web3.eth.sendSignedTransaction(serializedTx);

		// Output the URL to the transaction in block-explorer
		var txUrl;
		switch (chain) {
			case 3: //ropsten testnet
				txUrl = `https://ropsten.etherscan.io/tx/${transaction.transactionHash}`;
				break;
			case 1:
				txUrl = `https://etherscan.io/tx/${transaction.transactionHash}`;
				break;
			default:
				txUrl = transaction.transactionHash;
		}
		print("Transaction sent successfully:\n" + txUrl);
		return transaction;
	}

	async run() {
		await this.createUkcPartitionAndKeyIfNeseccary();
		await this.sendEthWithUkc();
	}
}