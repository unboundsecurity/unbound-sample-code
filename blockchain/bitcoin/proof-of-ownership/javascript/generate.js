/**
 * Generates a 'proof of ownership' message hash.
 * The generated hash can be used as CASP dataToSign
 * 1. Start a CASP sign operation, using the hash as 'dataToSign' (see https://www.unboundtech.com/docs/CASP/API/casp-byow.html#start-a-signing-process)
 * 2. Once the signature is complete, copy the response from (https://www.unboundtech.com/docs/CASP/API/casp-byow.html#get-sign-operation) 
 *    into the casp-op.json file in this project
 * 3. Run `npm start` to verify the signature and get verification data for external verifiers
 */
const btcMessage = require('bitcoinjs-message');
const messageText = "This is an example of a signed message.";
const hashTosign = btcMessage.magicHash(messageText);
const hexEncoded = hashTosign.toString('hex');

console.log("Message hash for signature is:");
console.log(hexEncoded);
