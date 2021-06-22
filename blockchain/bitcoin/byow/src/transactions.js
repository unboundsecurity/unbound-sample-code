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
const axios = require('axios');
const bitcoinjs = require('bitcoinjs-lib');
const BufferUtil = bitcore.util.buffer;
const network = bitcoinjs.networks.testnet;


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

  let ecpair = bitcoinjs.ECPair.fromPublicKey(Buffer.from(publicKeyInfo.publicKeyRaw, 'hex'), { network: network });
  // convert to Bitcoin address
  let address = bitcoinjs.address.toBase58Check(bitcoinjs.crypto.hash160(ecpair.publicKey), network.pubKeyHash);

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

  const psbt = new bitcoinjs.Psbt({ network: network });
  const address = options.addressInfo.address;

  let to = (await inquirer.prompt([{
    name: 'to', validate: util.required('To address'),
    message: 'To address: '
  }]));

  const utxos = (await request('transactions', 'get', [
    { option: 'include_raw', value: 'true' },
    { option: 'address', value: address }
  ], undefined, options.jwtToken)).data._embedded.transactions;


  utxos.forEach(element => {

    // Finds all of the inputs for the transaction
    const transfers = element._embedded.transfers.reduce((a, b) => a.concat(b), []);
    const inputs = transfers.filter(t => t.to_address === address);
    for (let index = 0; index < inputs.length; index++) {
      const input = inputs[index];

      psbt.addInput({
        hash: element.identifier,
        index: Number(input.meta.n === undefined ? input.meta.vout : input.meta.n),
        // VERY IMPORTANT
        nonWitnessUtxo: Buffer.from(
          element.raw,
          'base64',
        ),
        // witnessUtxo: {
        //   script: Buffer.from(input.meta["scriptPubKey.hex"],'hex'),
        //   value: Number(input.amount.amount)
        // }
      });
    }
  });


  const amount = Number(options.addressInfo.balance);

  // Who we send to
  psbt.addOutput({
    address: to.to,
    value: amount/2 ,
  })


  const vaultId = options.activeVault.id;
  // "cc13679623048ed31168c2777bd42af386abd670a8c1b8c949041a649dc14512"
  const c = psbt.__CACHE;
  // if (!disableFeeCheck) {
    // psbt.checkFees(psbt, c, psbt.opts);
  // }

  let tx;

  tx = c.__TX.clone();
  //psbt.inputFinalizeGetAmts(psbt.data.inputs, tx, c, true);
  util.showSpinner('Requesting signature from CASP');
  var signRequest = {
    dataToSign: [
      tx.toHex()
    ],
    publicKeys: [
      options.addressInfo.keyId
    ],
    description: 'Test transaction BTCTEST',
    // the details are shown to the user when requesting approval
    details: JSON.stringify(psbt, undefined, 2),
    // callbackUrl: can be used to receive notifictaion when the sign operation
    // is approved

  };

  let providerData = {
    request: {
      amount: (amount) / 200000000,
      asset: "BTC",
      recipientAddress: to.to,
      fee: '1m',
      description: "payback",
      coin: 1,
      account: 0
    },
    txs: [
      {
        serialized: tx.toHex(),
      },
    ],
    recipients: [
      {
        address: to.to,
        asset: "BTC",
        amount: amount-1000,
      },
    ],
    cryptoKind: "ECDSA",
  };

let data = {
  description: 'BYOW',
  dataToSign: [
    tx.getHash().toString('hex'),
  ],
  publicKeys: [
    options.addressInfo.keyId,
  ],
  ledgerHashAlgorithm: "DOUBLE_SHA256",
  // callbackUrl: "http://localhost:3000/api/v1.0/sign_complete_callback?ledgerId=BTCTEST",
  ledger: "BTCTEST",
  rawTransactions: [
    tx.toHex(),
  ],
  providerData: JSON.stringify(providerData)
};


  var quorumRequestOpId = (await util.superagent.post(`${options.caspMngUrl}/vaults/${vaultId}/sign`)
    .send(/*signRequest*/data)).body.operationID;
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

  let signature = signOp.signatures[0];

  util.log(`Signature created: ${signature}`)

  if (signOp.isApproved) {
    const publicKey = bitcoinjs.ECPair.fromPublicKey(Buffer.from(options.addressInfo.publicKeyRaw, 'hex'), network).publicKey;
    const encodedSignature = bitcoinjs.script.signature.encode(Buffer.from(signature, 'hex'), bitcoinjs.Transaction.SIGHASH_ALL);
    for (let index = 0; index < psbt.inputCount; index++) {
      psbt.updateInput(index, {
        partialSig: [
          {
            pubkey: publicKey,
            signature: encodedSignature,
          }
        ]
      });

    }


    psbt.validateSignaturesOfInput(0);
    psbt.finalizeAllInputs();

    let psbtTrans = psbt.extractTransaction();

    // let transaction = psbt.extractTransaction();
    // transaction.setInputScript(0,publicKey);

    // new bitcore.Transaction(tx.toHex()).verify()

    // return transaction.toHex();
    // "c42058735c1a6465d734d2af387b0b2ba2f29719d47aa424e23397043bc64a41"
    return psbtTrans.toHex();

  }
  else {
    throw Error("Vault participant didn't sign transactions")
  }

}

function signTransaction(txHex, signature, publicKeyRaw) {
  const transaction = bitcoinjs.Transaction.fromHex(txHex);

  const encodedSignature = bitcoinjs.script.signature.encode(Buffer.from(signature, 'hex'), bitcoinjs.Transaction.SIGHASH_ALL);
  const publicKey = bitcoinjs.ECPair.fromPublicKey(Buffer.from(publicKeyRaw, 'hex'), network).publicKey;
  const scriptSig = bitcoinjs.payments.p2pkh({
    signature: encodedSignature,
    pubkey: publicKey,
    network: network
  });
  transaction.setInputScript(0, scriptSig.input);


  return transaction.toHex();
}

/**
 * Sends a signed transaction to Infura ledger
 * @param  {Object} options
 * @return {string} transaction hash of the sent transaction
 */
async function sendTransaction(options) {

  const hex = options.pendingTransaction;
  const transaction = bitcoinjs.Transaction.fromHex(hex);
  const hash = transaction.getHash().toString('hex');


  const data = Buffer.from(hex, 'hex').toString('base64');

  util.showSpinner('Sending signed transaction to ledger');

  // Does the actual transaction
  let postResult = await request('transactions', 'post', null, {
    data: data,
    blockchain_id: 'bitcoin-testnet',
    transaction_id: `bitcoin-testnet:${hash}`
  },
    options.jwtToken);

  util.hideSpinner();
  util.log(`Transaction sent successfully, txHash is: ${postResult.transactionHash}`);
  // console.log(res.data);
  return postResult.transactionHash;
}

async function request(endpoint, method, options, data, clientJwtToken) {
  let url = `https://api.blockset.com/${endpoint}?blockchain_id=bitcoin-testnet`;
  if (options) {
    url = url.concat(options.reduce(
      (total, curr) => {
        return `${total}&${curr.option}=${escape(curr.value)}`;
      }, ''));
  }

  return axios({
    method,
    url,
    data,
    headers: {
      Authorization: `Bearer ${clientJwtToken}`
    }
  });
}


module.exports = {
  createAddress,
  waitForDeposit,
  createTransaction,
  signTransaction,
  sendTransaction,
  getPublicKeyFromCasp
}

