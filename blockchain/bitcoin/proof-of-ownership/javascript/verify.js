
 /**
 * This is code shows how you can use CASP for generating "Proof Of Ownership" signature for Bitcoin.
 * 
 * The output can be verified with one of the following services:
 * - https://brainwalletx.github.io/#verify
 * - https://tools.bitcoin.com/verify-message/
 */
const CASP_SIGN_OP_DATA = require("./casp-sign-op-complete.json");
const btclib = require("bitcoinjs-lib");
const asn1 = require("asn1");
const btcMessage = require('bitcoinjs-message');
const bech32addresses = false;

const signature = Buffer.from(CASP_SIGN_OP_DATA.signatures[0], 'hex');
const recoveryCode = CASP_SIGN_OP_DATA.v[0] + 27 + 4;

var addressFromPublicKey = (pubKeyBytes) => {
    const network = btclib.networks.bitcoin;
    const pair = btclib.ECPair.fromPublicKey(pubKeyBytes, { network });
    const publicKeyHash = btclib.crypto.hash160(pair.publicKey);
    if (bech32addresses) {
        return btclib.address.toBech32(publicKeyHash, network.pubKeyHash, network.bech32);
    } else {
        return btclib.address.toBase58Check(publicKeyHash, network.pubKeyHash);
    }
}

var getRawEcPublicKeyFromDerHex = (publicKeyDerHex) => {
    var reader = new asn1.BerReader(Buffer.from(publicKeyDerHex, 'hex'));
    reader.readSequence();
    reader.readSequence();
    reader.readOID();
    reader.readOID();
    reader.readSequence();
    // return 65 bytes
    return reader.buffer.slice(1);
  }

const rawPublicKey = getRawEcPublicKeyFromDerHex(CASP_SIGN_OP_DATA.publicKeys[0]);
const address = addressFromPublicKey(rawPublicKey);
console.log(`Bitcoin address: ${address}\n`);
console.log(`public key raw: ${rawPublicKey.toString('hex')}\n`)
console.log(`Signature prefixed by recovery code (Base64):`)
const signatureWithRecoveryCodeBase64 = Buffer.from(`${recoveryCode.toString(16)}${signature.toString('hex')}`, 'hex').toString('base64');
console.log(signatureWithRecoveryCodeBase64);

const messageText = "This is an example of a signed message.";
console.log(`\nMessage text: '${messageText}'\n`);
console.log(`Verified: ${btcMessage.verify(messageText, address, signatureWithRecoveryCodeBase64)}`);

console.log(`\nTry this too: https://tools.bitcoin.com/verify-message/\n\n`)