const bitcoinjs = require('bitcoinjs-lib');
const Transaction = bitcoinjs.Transaction;
const bscript = bitcoinjs.script;
const bcrypto = bitcoinjs.crypto;
const bitcoin = bitcoinjs.networks;
const opcodes = bitcoinjs.opcodes;
// import { bitcoin } from 'bitcoinjs-lib/types/networks';

const varuint = require('varuint-bitcoin');
const bufferutils = require('bitcoinjs-lib/src/bufferutils');

// the below code was copied from https://github.com/bitcoinjs/bitcoinjs-lib/blob/master/src/transaction.js#L254
// in order to extract the raw transaction before hashing
const EMPTY_SCRIPT = Buffer.allocUnsafe(0)
const ONE = Buffer.from('0000000000000000000000000000000000000000000000000000000000000001', 'hex')
const VALUE_UINT64_MAX = Buffer.from('ffffffffffffffff', 'hex')
const BLANK_OUTPUT = {
  script: EMPTY_SCRIPT,
  valueBuffer: VALUE_UINT64_MAX
}
const ONE_RES = { raw: ONE, hash: bcrypto.hash256(ONE) };

/**
 * Serializes a transaction to raw format that can be used to create the hash for signature.
 * The rawTx is parsed by CASP to ensure transaction integrity and show the transaction details to the user
 * This code is extracted from https://github.com/bitcoinjs/bitcoinjs-lib/blob/master/src/transaction.js#L221
 * @param  {bitcoin.Transaction} transaction  -  A bitcoin Transaction object https://github.com/bitcoinjs/bitcoinjs-lib/blob/master/src/transaction.js
 * @param  {number} inIndex - The index of the input index for raw tx -> each input has raw serialized format with its key
 * @param  {[type]} prevOutScript [description]
 * @param  {[type]} hashType      [description]
 * @return {Object}               An object with raw tx buffer and hash created from that tx
 */
function rawTxForHash(transaction,
  inIndex, prevOutScript, hashType) {
  if (inIndex >= transaction.ins.length) return ONE_RES;

  // ignore OP_CODESEPARATOR
  const ourScript = (
      bscript.compile(
          bscript.decompile(prevOutScript).filter(function(x) {
            return x !== opcodes.OP_CODESEPARATOR
          })
      )
  );

  const txTmp = transaction.clone()

  // SIGHASH_NONE: ignore all outputs? (wildcard payee)
  if ((hashType & 0x1f) === Transaction.SIGHASH_NONE) {
    txTmp.outs = []

    // ignore sequence numbers (except at inIndex)
    txTmp.ins.forEach(function(input, i) {
      if (i === inIndex) return

      input.sequence = 0
    })

    // SIGHASH_SINGLE: ignore all outputs, except at the same index?
  } else if ((hashType & 0x1f) === Transaction.SIGHASH_SINGLE) {
    // https://github.com/bitcoin/bitcoin/blob/master/src/test/sighash_tests.cpp#L60
    if (inIndex >= transaction.outs.length) return ONE_RES;

    // truncate outputs after
    txTmp.outs.length = inIndex + 1

    // "blank" outputs before
    for (var i = 0; i < inIndex; i++) {
      txTmp.outs[i] = BLANK_OUTPUT
    }

    // ignore sequence numbers (except at inIndex)
    txTmp.ins.forEach(function(input, y) {
      if (y === inIndex) return

      input.sequence = 0
    })
  }

  // SIGHASH_ANYONECANPAY: ignore inputs entirely?
  if (hashType & Transaction.SIGHASH_ANYONECANPAY) {
    txTmp.ins = [txTmp.ins[inIndex]]
    txTmp.ins[0].script = ourScript

    // SIGHASH_ALL: only ignore input scripts
  } else {
    // "blank" others input scripts
    txTmp.ins.forEach(function(input) { input.script = EMPTY_SCRIPT })
    txTmp.ins[inIndex].script = ourScript
  }

  // serialize and hash
  const buffer = Buffer.allocUnsafe(txTmp['byteLength'](false) + 4);
  buffer.writeInt32LE(hashType, buffer.length - 4);
  txTmp['__toBuffer'](buffer, 0, false);
  return {
    raw: buffer,
    hash: bcrypto.hash256(buffer)
  }
}

const ZERO = Buffer.from(
    '0000000000000000000000000000000000000000000000000000000000000000',
    'hex',
);

function varSliceSize(someScript) {
  const length = someScript.length;
  return varuint.encodingLength(length) + length;
}

const cryptoUtils = { rawTxForHash };

module.exports =  cryptoUtils;
