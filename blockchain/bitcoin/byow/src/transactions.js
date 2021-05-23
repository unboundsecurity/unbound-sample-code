"use strict";
const inquirer = require('inquirer');
const asn1js = require('asn1js');
const EthUtil = require('ethereumjs-util');
const EthereumTx = require('ethereumjs-tx');
const chainId = 3; //eth-ropsten
const util = require('./util');
const Promise = require('bluebird');
const rlp = require('rlp');
const bitcore = require('bitcore-lib');
const { default: axios } = require('axios');
const bitcoinjs = require('bitcoinjs-lib');
const BufferUtil = bitcore.util.buffer;



/**
 * Extract raw public key bytes from DER encoded EC public key.
 * CASP returns genrated keys in DER format, so we need to extract the relevant
 * data for address generation.
 *
 * @param  {string} publicKeyDerHexString - DER hex-encoded ECDSA public key from CASP
 * @return {Buffer} a buffer with byte data of the public key
 */
function getRawEcPublicKeyFromDerHex(publicKeyDerHexString) {
  const pkDerBuf = Buffer.from(publicKeyDerHexString, 'hex');
  const arrayBuffer = new ArrayBuffer(pkDerBuf.length);
  const uint8Array = new Uint8Array(arrayBuffer);
  for (let i = 0; i < pkDerBuf.length; i++) uint8Array[i] = pkDerBuf[i];
  const asn = asn1js.fromBER(arrayBuffer.slice(0));
  var hex = asn.result.valueBlock.value[1].valueBlock.valueHex;
  return Buffer.from(hex);
}


/**
 * Returns an rlp serialized transaction for verification by CASP
 * This transaction is sent to CASP as rawTransaction for hash verification
 * CASP will run the keccak 256 algorithm on this transaction and will verify the
 * hash against the hash sent for signature.
 * @param  {type} tx the EthereumJs transaction
 * @return {type} a hex encoded string with rlp-serialized transaction
 */
function getRlpEncodedRawTransactionForSignature(tx) {
  var chainId = tx._chainId;
  var items;
  if (chainId > 0) {
    const raw = tx.raw.slice();
    tx.v = chainId;
    tx.r = 0;
    tx.s = 0;
    items = tx.raw;
    tx.raw = raw;
  } else {
    items = tx.raw.slice(0, 6);
  }
  return rlp.encode(items).toString('hex');
}

/**
 * Gets a public key from CASP
 * For BIP44 vaults this will create a new key
 * For non-BIP44 vaults it will return the only key of the vault
 *
 * @param  {Object} options
 * @param  {string} options.caspMngUrl - The URL of CASP management API
 * @param  {Object} options.activeVault - Details of the last selected vault(id, name, hierarchy)
 * @return {Object} Details of the public key with id and raw bytes.
 *          The id is used with CASP sign as a reference to the key to sign with
 */
async function getPublicKeyFromCasp(options) {
  options = { ...{ coinId: 1 }, ...options };
  const vault = options.activeVault;
  const isBip44 = vault.hierarchy === 'BIP44';
  var coinId = options.coinId;

  var publicKeyFromCasp;
  if (isBip44) {
    util.log(`Generating key with CASP for BIP44 vault`);
    // for BIP-44 vaults generate a new public key
    publicKeyFromCasp = (await util.superagent.post(`${options.caspMngUrl}/vaults/${vault.id}/coins/${coinId}/accounts/0/chains/external/addresses`))
      .body.publicKey;
  } else {
    util.log(`Getting public key from CASP for non-BIP44 vault`);
    // for non BIP-44 vaults, get the single vault public key
    publicKeyFromCasp = (await util.superagent.get(`${options.caspMngUrl}/vaults/${vault.id}/publickey`)).body.publicKey;
  }

  // for ECDSA CASP returns DER encoded key so we extract raw key from it
  // for EDDSA CASP returns the raw key directly
  var publicKeyRaw = (vault.cryptoKind === 'ECDSA')
    ? getRawEcPublicKeyFromDerHex(publicKeyFromCasp)
    : publicKeyFromCasp;
  return {
    // this will be used to reference the key in sign requests
    keyId: publicKeyFromCasp,
    publicKeyRaw: publicKeyRaw.toString('hex')
  };
}

/**
 * Creates a new BIP44 Ethereum address with CASP
 *
 * @param  {Object} options
 * @param  {string} options.caspMngUrl - The URL of CASP management API
 * @param  {Object} options.activeVault - Details of the last selected vault(id, name)
 * @return {Object} Address information for the generated Ethereum address,
 * includes the address, DER encoded public key and hex encoded raw public key bytes
 */
async function createAddress(options) {
  var publicKeyInfo = await getPublicKeyFromCasp(options);

  let publicKey = bitcore.PublicKey(publicKeyInfo.publicKeyRaw,{network:'testnet'});

  // convert to Bitcoin address
  var address = publicKey.toAddress('testnet').toString();

  util.log(`Generated address: ${address}`)
  return { ...publicKeyInfo, address: address };
}

/**
 * Waits until funds are deposited for address
 * Uses web3.js
 * @param  {Object} options
 * @param  {string} options.infuraUrl - URL for Infura ledger server
 * @param  {Object} options.addressInfo - Information for the requested address(address)
 * @return {string} The balance as string
 */
async function waitForDeposit(options) {

  var address = options.addressInfo.address;
  // wait for deposit
  util.showSpinner(`Waiting for deposit to ${address}`);
  var balance = BigInt(0);
  try {
    do {
      //aab98afc0e7d5c82b46580971620361276262a3cf635d30d9c30cf549cd7af9c
      balance = await getBalance(address, options);
      balance = BigInt(balance);
      await Promise.delay(50)
    } while (balance === BigInt(0));
    util.hideSpinner();
    util.log(`Using address: ${address}`)
    util.log(`Balance is: ${balance} BTCTST`);
    return balance.toString();
  } catch (e) {
    console.log(e);
    util.hideSpinner();
    util.logError(e);
  }
}

/**
 *  Gets the balance of the account 
 * @param {string} address The address of the account 
 * @param {Object} options System options of the demo
 * @returns The balance of the account
 */
async function getBalance(address, options) {
  return (await getUnspentOutputs(address, options)).reduce((total, curr) => {
    return total + curr.value;
  }, 0).toString();
}

/**
 * Calculates the balance of the account using all transactions and other information
 * @param {string} address The address  of the account
 * @param {Object} options System options of the demo
 * @returns The balance of the account
 */
async function getUnspentOutputs(address, options) {
  const transactions = (await getTransactions(address, options.jwtToken)).filter(t => t.confirmations >= 0);
  const transfers = transactions.map(tr => tr._embedded && tr._embedded.transfers || []).reduce((a, b) => a.concat(b), []);
  const outputs = transfers.filter(t => t.to_address === address);
  const inputs = transfers.filter(t => t.from_address === address);
  const unspentOutputs = outputs.filter(o => !inputs.some(i => o.transaction_id.endsWith(i.meta.txid)));

  return unspentOutputs.map(uo => {
    const parentTx = transactions.find(t => t.transaction_id === uo.transaction_id);
    const parentTxOutputs = new bitcore.Transaction(Buffer.from(parentTx.raw, 'base64')).outputs;

    return {
      tx_hash: parentTx.identifier,
      tx_output_n: Number(uo.meta.n),
      value: Number(uo.amount.amount),
      confirmations: parentTx.confirmations,
      script: parentTxOutputs[Number(uo.meta.n)].script.toBuffer()
    };
  });
}


/**
 * 
 * @param {String} address The address of the account
 * @param {String} jwtToken The token used to authenticate blockset requests
 * @returns The list of of transactions
 */
async function getTransactions(address, jwtToken) {
  const options = [
    { option: 'include_raw', value: 'true' },
    { option: 'address', value: address }
  ];

  let url = `https://api.blockset.com/transactions?blockchain_id=bitcoin-testnet`;

  url = url.concat(options.reduce(
    (total, curr) => {
      return `${total}&${curr.option}=${escape(curr.value)}`;
    }, ''));


  return axios({
    method: 'get', url: url, headers: { Authorization: `Bearer ${jwtToken}` }
  }).then(response => {
    return response.data._embedded ? response.data._embedded.transactions : [];
  }).catch(error => {
    util.logError(error);
  });
}

/**
 * Creates an Bitcoin transaction
 * @param  {Object} options
 * @return {bitcore.Transaction} Details of the created transaction including the parameters
 *                  that were used to create it and the transaction hash for signature
 */
async function createTransaction(options) {

  const transaction = new bitcore.Transaction();
  const address = options.addressInfo.address;

  let to = (await inquirer.prompt([{
    name: 'to', validate: util.required('To address'),
    message: 'To address: '
  }]));
  let fee = 0;
  let inputCount = 0;
  let outputCount = 2;

  const utxos = await axios.get(
    `https://sochain.com/api/v2/get_tx_unspent/BTCTEST/${address}`
  );

  var amount = options.addressInfo.balance;

  let inputs = [];
  utxos.data.data.txs.forEach(async (element) => {
    let utxo = {};

    utxo.satoshis = Math.floor(Number(element.value) * 100000000);
    utxo.script = element.script_hex;
    utxo.address = utxos.data.data.address;
    utxo.txId = element.txid;
    utxo.outputIndex = element.output_no;

    inputCount += 1;
    inputs.push(utxo);
  });

  let transactionSize = inputCount * 180 + outputCount * 34 + 10 - inputCount;

  fee = transactionSize * 20;

  //Set transaction input
  transaction.from(inputs);

  // set the recieving address and the amount to send
  transaction.to(to.to, +amount - fee);

  // Set change address - Address to receive the left over funds after transfer
  transaction.change(to.to);

  //manually set transaction fees: 20 satoshis per byte
  transaction.fee(fee);

  let serializedTransaction = transaction.serialize({ disableIsFullySigned: true });

  const vaultId = options.activeVault.id;

  util.showSpinner('Requesting signature from CASP');
  var signRequest = {
    dataToSign: [
      serializedTransaction
    ],
    publicKeys: [
      options.addressInfo.keyId
    ],
    description: 'Test transaction BTCTEST',
    // the details are shown to the user when requesting approval
    details: JSON.stringify(transaction, undefined, 2),
    // callbackUrl: can be used to receive notifictaion when the sign operation
    // is approved
  };

  var quorumRequestOpId = (await util.superagent.post(`${options.caspMngUrl}/vaults/${vaultId}/sign`)
    .send(signRequest)).body.operationID;
  util.hideSpinner();
  util.log('Signature process started, signature must be approved by vault participant');
  util.log(`To approve with bot, run: 'java -Djava.library.path=. -jar BotSigner.jar -u http://localhost/casp -p ${options.activeParticipant.id} -w 1234567890'`);
  util.showSpinner('Waiting for signature quorum approval');
  var signOp;
  do {
    signOp = (await util.superagent.get(`${options.caspMngUrl}/operations/sign/${quorumRequestOpId}`))
      .body;
    await Promise.delay(1000);
  } while (signOp.status !== 'COMPLETED'); //
  util.hideSpinner();
  util.log(`Signature created: ${signOp.signatures[0]}`)

  var signature = signOp.signatures[0];

//  let signedHex = signTransaction(serializedTransaction, signature, options.addressInfo.publicKeyRaw);

//   return signedHex;

  var r  = new Buffer.from(signature.slice(0, 64).toLowerCase(), 'hex');
  var s = new Buffer.from(signature.slice(64).toLowerCase(), 'hex');

  let rBN = new bitcore.crypto.BN(r);
  let sBN = new bitcore.crypto.BN(s);

  let lastInput= inputs[inputs.length-1];
  let lastOutput= transaction.outputs[0];

  let builtSignature = new bitcore.Transaction.Signature( {signature: new bitcore.crypto.Signature(rBN,sBN),
    prevTxId:lastInput.txId,
    outputIndex: 0,
    inputIndex: inputs.length-1,
    sigtype: bitcore.crypto.Signature.SIGHASH_ALL,
    publicKey: new bitcore.PublicKey(options.addressInfo.publicKeyRaw,{network:'testnet'})
  });

  //transaction.applySignature(builtSignature,"ecdsa");

  let inputToSign = transaction.inputs[inputs.length-1];

  if (inputToSign.output.script.isWitnessPublicKeyHashOut() || inputToSign.output.script.isScriptHashOut()) {
    inputToSign.setWitnesses([
      BufferUtil.concat([
        builtSignature.signature.toDER(),
        BufferUtil.integerAsSingleByteBuffer(builtSignature.sigtype)
      ]),
      builtSignature.publicKey.toBuffer()
    ]);
  } else {
    inputToSign.setScript(bitcore.Script.buildPublicKeyHashIn(
      builtSignature.publicKey,
      builtSignature.signature.toDER(),
      builtSignature.sigtype
    ));
  }//tb1q7q3h9726tes70cqvx6hgu2dj3z9mgx3c473ndw

  return transaction.serialize();

  // console.log(signature);
  // console.log(new bitcore.Transaction(signature));

  // console.log(transaction.isFullySigned());
  //tb1qnnd5xscvttt8475xvwdl20h0zmm2zedzc2p7w3
}

function signTransaction(txHex, signature, publicKeyRaw) {
  const transaction = bitcoinjs.Transaction.fromHex(txHex);

  const encodedSignature = bitcoinjs.script.signature.encode(Buffer.from(signature, 'hex'), bitcoinjs.Transaction.SIGHASH_ALL);
  const publicKey = bitcoinjs.ECPair.fromPublicKey(Buffer.from(publicKeyRaw, 'hex'), bitcoinjs.networks.testnet).publicKey;
  const scriptSig = bitcoinjs.payments.p2pkh({
    signature: encodedSignature,
    pubkey: publicKey,
    network: bitcoinjs.networks.testnet
  });
  transaction.setInputScript(0, scriptSig.input);


  return transaction.toHex();
}

/**
 * Sign a transaction with CASP
 * @param  {Object} options
 * @param  {string} options.caspMngUrl - The URL for CASP management API
 * @param  {string} options.infuraUrl - URL for Infura ledger server
 * @param  {Object} options.activeVault - Details of the CASP vault to use for signature
 * @param  {Object} options.addressInfo - Information of the address that is the
 *                  source(from) of the transaction (address, publicKeyDER )
 * @param  {bitcore.Transaction} options.pendingTransaction - Details of the transaction to sign,
 *                  including: hashToSign and txData
 * @return {Object} Signature data with:
 *                  signOperation - details of the CASP signature quorum operation
 *                  serializedSignedTransaction - serialized signed transaction that can be sent to the ledger
 */
async function signTransactionOld(options) {
  var web3 = new Web3(options.infuraUrl);
  const vaultId = options.activeVault.id;
  var pendingTransaction = options.pendingTransaction;
  util.showSpinner('Requesting signature from CASP');
  var signRequest = {
    dataToSign: [
      pendingTransaction.hashToSign
    ],
    publicKeys: [
      options.addressInfo.keyId
    ],
    description: 'Test transaction Eth',
    // the details are shown to the user when requesting approval
    details: JSON.stringify(pendingTransaction, undefined, 2),
    // callbackUrl: can be used to receive notifictaion when the sign operation
    // is approved
  };

  // casp added support for eth rawTransaction verification
  var useRawTransaction = options.caspRelease >= 1812;
  if (useRawTransaction) {
    signRequest.rawTransactions = [
      pendingTransaction.rawTransaction
    ];
    signRequest.ledgerHashAlgorithm = 'SHA3_256';
  }

  try {
    var quorumRequestOpId = (await util.superagent.post(`${options.caspMngUrl}/vaults/${vaultId}/sign`)
      .send(signRequest)).body.operationID;
    util.hideSpinner();
    util.log('Signature process started, signature must be approved by vault participant');
    util.log(`To approve with bot, run: 'java -Djava.library.path=. -jar BotSigner.jar -u http://localhost/casp -p ${options.activeParticipant.id} -w 1234567890'`);
    util.showSpinner('Waiting for signature quorum approval');
    var signOp;
    do {
      signOp = (await util.superagent.get(`${options.caspMngUrl}/operations/sign/${quorumRequestOpId}`))
        .body;
      await Promise.delay(500);
    } while (signOp.status !== 'COMPLETED'); //
    util.hideSpinner();
    util.log(`Signature created: ${signOp.signatures[0]}`)

    var signature = signOp.signatures[0];
    var v = signOp.v[0];
    // workaround for CASP pre-November bug
    // starting CASP 11-2018 v is returned as 0 or 1
    if (v === 27) v = 1;
    if (v === 28) v = 0;
    var tx = new EthereumTx(pendingTransaction.txData);
    var r = tx.r = new Buffer.from(signature.slice(0, 64).toLowerCase(), 'hex');
    var s = tx.s = new Buffer.from(signature.slice(64).toLowerCase(), 'hex');
    // According to https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md
    var v = tx.v = chainId * 2 + v + 35;
    // validate that the from address decoded from signature is our address
    var fromAddress = pendingTransaction.txData.from;

    // verify signature by recovering the public-key/address and comparing against
    // our original 'from' address
    try {
      var recoveredPublicKey = EthUtil.ecrecover(new Buffer.from(pendingTransaction.hashToSign, 'hex'), v, r, s, chainId).toString('hex');
      var recoveredAddress = addressFromPublicKey(recoveredPublicKey);
      if (fromAddress.toLowerCase() !== recoveredAddress.toLowerCase()) {
        throw `Invalid recovered address: ${fromAddress.toLowerCase()} <> ${recoveredAddress.toLowerCase()}`;
      }
      util.log(`Signature verified successfully`);
    } catch (e) {
      util.log('Signature verification failed');
      throw e;
    }

    // another method for verification which interanlly runs similar code to the
    // one above
    if (tx.from.toString('hex').toLowerCase() !== fromAddress.toLowerCase().slice(2)) {
      throw new Error("Failed to sign transaction, invalid v value");
    }
    return {
      signOperation: signOp,
      serializedSignedTransaction: '0x' + tx.serialize().toString('hex')
    }
  } catch (e) {
    util.log(e);
    util.hideSpinner();
    util.logError(e);
  }
}

/**
 * Sends a signed transaction to Infura ledger
 * @param  {Object} options
 * @return {string} transaction hash of the sent transaction
 */
async function sendTransaction(options) {

  // const result = await axios({
  //   method: "POST",
  //   url: `https://sochain.com/api/v2/send_tx/BTCTEST`,
  //   data: {
  //     tx_hex: options.pendingTransaction,
  //   },
  // });
  // console.log(result.data.data);
  // return result.data.data;

    let url = `https://api.blockset.com/transactions`;
    const serializedRequest = options.pendingTransaction;

    const data = Buffer.from(serializedRequest, 'hex').toString('base64');
    const hash = bitcoinjs.Transaction.fromHex(serializedRequest).getHash().toString('hex');;

    util.showSpinner('Sending signed transaction to ledger');
    var res = await axios({
      method: 'post',
      url: url,
      data: {
        data: data,
        blockchain_id: 'bitcoin-testnet',
        transaction_id: `bitcoin-testnet:${hash}`
    },
      headers: {
          Authorization: `Bearer ${options.jwtToken}`
      }
  });
    util.hideSpinner();
    util.log(`Transaction sent successfully, txHash is: ${res.transactionHash}`);
    // console.log(res.data);
    return res.transactionHash;
}

module.exports = {
  createAddress,
  waitForDeposit,
  createTransaction,
  signTransaction,
  sendTransaction,
  getPublicKeyFromCasp
}

