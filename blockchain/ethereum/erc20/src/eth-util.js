const asn1js = require('asn1js');
const EthUtil = require('ethereumjs-util');
const rlp = require('rlp');
const EthereumTx = require('ethereumjs-tx').Transaction;
const CaspClient = require('./casp-client');
const Web3 = require('web3');


class CaspEthUtil {
  constructor(casp) {
   this.casp = casp || new CaspClient();
   this.web3 = new Web3(new Web3.providers.HttpProvider(this.config.infuraHost));
  }

  get config() {
   return this.casp.config;
  }

  /**
   * Returns an rlp serialized transaction for verification by CASP
   * This transaction is sent to CASP as rawTransaction for hash verification
   * CASP will run the keccak 256 algorithm on this transaction and will verify the
   * hash against the hash sent for signature.
   * @param  {type} tx the EthereumJs transaction
   * @return {type} a hex encoded string with rlp-serialized transaction
   */
  getRlpEncodedRawTransactionForSignature(tx) {
    let items
    if (false) {
      items = tx.raw
    } else {
      if (tx._implementsEIP155()) {
        items = [
          ...tx.raw.slice(0, 6),
          EthUtil.toBuffer(tx.getChainId()),
          // TODO: stripping zeros should probably be a responsibility of the rlp module
          EthUtil.stripZeros(EthUtil.toBuffer(0)),
          EthUtil.stripZeros(EthUtil.toBuffer(0)),
        ]
      } else {
        items = tx.raw.slice(0, 6)
      }
    }
    return rlp.encode(items).toString('hex');
  }

  /**
   * Create an Ethereum address from a raw EC public key buffer.
   *
   * @param  {Buffer} rawEcPubKey - raw bytes of the EC public key
   * @return {string} An Ethereum address
   */
  addressFromPublicKey(rawEcPubKeyHex) {
    var rawEcPubKey = Buffer.from(rawEcPubKeyHex, 'hex');
    if(rawEcPubKey.length === 65) {
      // remove prefix
      rawEcPubKey = rawEcPubKey.slice(1);
    }
    if(!EthUtil.isValidPublic(rawEcPubKey)) throw "Invalid public key";
    const address = EthUtil.pubToAddress(rawEcPubKey);
    return EthUtil.toChecksumAddress(`0x${address.toString('hex')}`);
  }

  /**
   * Extract raw public key bytes from DER encoded EC public key.
   * CASP returns genrated keys in DER format, so we need to extract the relevant
   * data for address generation.
   *
   * @param  {string} publicKeyDerHexString - DER hex-encoded ECDSA public key from CASP
   * @return {Buffer} a buffer with byte data of the public key
   */
  getRawEcPublicKeyFromDerHex(publicKeyDerHexString) {
    const pkDerBuf = Buffer.from(publicKeyDerHexString, 'hex');
    const arrayBuffer = new ArrayBuffer(pkDerBuf.length);
    const uint8Array = new Uint8Array(arrayBuffer);
    for(let i = 0; i < pkDerBuf.length; i++) uint8Array[i] = pkDerBuf[i];
    const asn = asn1js.fromBER(arrayBuffer.slice(0));
    var hex = asn.result.valueBlock.value[1].valueBlock.valueHex;
    return Buffer.from(hex);
  }


  /**
   * Convert a public key from CASP in DER format to Ethereum address
   */
  toEthAddress(caspPublicKeyDer) {
    var bytes = this.getRawEcPublicKeyFromDerHex(caspPublicKeyDer);
    return this.addressFromPublicKey(bytes);
  }

  /**
   * Perpares the data for an Ethereum transaction signature request from CASP.
   *
   * The data includes
   * - The transaction hash that should be signed
   * - The transaction data that is later used to reconstruct the EthereumTx object.
   * - The public key to use for signing the transaction. This is required since
   *   BIP44 vaults can have more than one key
   * - In case the transaction is a withdrawal (Eth or ERC20) the data also includes
   *   the serialized transaction that was used to produce the tx hash.
   *   This is used by CASP to verify the hash and extract transaction data
   *   from the serialized tx in order to enforce vault policies, for example,
   *   extract the tx amount and verify that it is within the allowed range
   */
  prepareSignRequest(rawTx, options) {
    var config = this.config;
    options = options || {};
    var tx = new EthereumTx(rawTx, {chain: config.ethChainId});
    var hashToSign = tx.hash(false).toString('hex');
    var signRequest = {
      dataToSign: [
        hashToSign
      ],
      publicKeys: [
        options.publicKeyDer
      ],
      description: options.description || 'Test transaction Eth',
      // the details are shown to the user when requesting approval
      details: JSON.stringify({rawTx}, undefined, 2),
      // callbackUrl: can be used to receive notifictaion when the sign operation
      // is approved
    };

    // when the transaction is a transfer transaction (ERC20 or Eth) we can pass
    // the raw transaction to CASP for parsing and enforcement of vault policies.
    // In case its a contract deploy transaction, this should not be included.
    if(options.includeRawTx) {
      signRequest.rawTransactions = [
        this.getRlpEncodedRawTransactionForSignature(tx)
      ],
      signRequest.ledgerHashAlgorithm = 'SHA3_256'
    }
    return signRequest;
  }

  /**
   * Apply the signature data from CASP to the target Ethereum transaction.
   *
   * After CASP signature request is approved by the minimum required vault members,
   * CASP creates the signature for the transaction hash with the vault key.
   *
   * This method applies the signature to the original transaction and creates
   * an Ethereum Transaction object which is later serialized and sent to the
   * Eth network
   * @param  {type} signOp A completed sign operation from CASP
   * @return {type}        An EthereumTx object with the original transaction signed
   */
  getSignedTransactionFromCaspOp(signOp) {
    var signature = signOp.signatures[0];

    // According to https://github.com/ethereum/EIPs/blob/master/EIPS/eip-155.md
    var chainId = this.config.ethChainId;
    var data = JSON.parse(signOp.details);
    var rawTx = data.rawTx;
    var tx = new EthereumTx(rawTx, { chain: chainId});
    var v = tx.v  = chainId * 2 + signOp.v[0] + 35;
    var r = tx.r = new Buffer.from(signature.slice(0, 64).toLowerCase(), 'hex');
    var s = tx.s = new Buffer.from(signature.slice(64).toLowerCase(), 'hex');

    // validate that the from address decoded from signature is our address
    var fromAddress = rawTx.from;

    // another method for verification which interanlly runs similar code to the
    // one above
    if(tx.from.toString('hex').toLowerCase() !== fromAddress.toLowerCase().slice(2)) {
      throw new Error("Failed to sign transaction, invalid v value");
    }
    return tx;
  }

  async createSignOperation(rawTx, vault, description) {
    var signRequest = this.prepareSignRequest(rawTx, {...vault, description});
    return this.casp.createSignOperation(vault, signRequest);
  }

  async getBalanceEth(address) {
    var balanceWei = await this.web3.eth.getBalance(address);
    return ( balanceWei/ 1e18);
  }

  async getBalanceErc(address, contractData) {
    var contract = new this.web3.eth.Contract(contractData.abi, contractData.contractAddress);
    const balance = await contract.methods.balanceOf(address).call();
    return balance / Math.pow(10, contractData.info.decimals);
  }

  async sendSignedTransaction(signedTxHex) {
    return this.web3.eth.sendSignedTransaction(signedTxHex);
  }

  async waitForTransactionReceipt(txHash) {
    var receipt = await this.web3.eth.getTransactionReceipt(txHash);
    return receipt || await Promise.delay(1000)
                      .then(() => this.waitForTransactionReceipt(txHash))
  }

  async getGasData() {
    var web3 = this.web3;
    var gasPrice = await web3.eth.getGasPrice();
    var highPrice = this.config.gasPriceGwei * Math.pow(10,9);
    // were using high price otherwise contract deployment
    // tx get stuck for long time
    gasPrice = highPrice;
    return {
      gasPrice: Web3.utils.toHex(gasPrice)
      // gasLimit: Web3.utils.toHex(6000000)
    }
  }

  async initRawTx(rawTx, vault) {
    var gasData = await this.getGasData();
    var nonce = Web3.utils.toHex(await this.web3.eth.getTransactionCount(vault.address, "pending"));
    Object.assign(rawTx, {...gasData, nonce, from: vault.address, ...rawTx});
    var estimate = await this.estimateGas(rawTx);
    rawTx.gasLimit = Web3.utils.toHex(estimate);
    return rawTx;
  }

  async getEthTransaction(to, amountEth, vault) {
    var amountWei = Web3.utils.toWei(amountEth.toString());
    return await this.initRawTx({
      to: to,
      value: Web3.utils.toHex(amountWei)
    }, vault)
  }

  async estimateGas(rawTx) {
    return this.web3.eth.estimateGas(rawTx);
  }

  async getContractDeployTx(contractData, vault) {
    var contract = new this.web3.eth.Contract(contractData.abi);
    var encodedContractData = contract.deploy({ data: '0x' + contractData.bytecode}).encodeABI();
    var rawTx = {
      data: encodedContractData,
    };
    return this.initRawTx(rawTx, vault);
  }

  async getAddressData(vault) {
    var publicKeyDer = await this.casp.getVaultDefaultKey(vault);
    var address = this.toEthAddress(publicKeyDer);
    return { publicKeyDer, address };
  }

  async getContractInfo(contractData) {
    var contract = new this.web3.eth.Contract(contractData.abi, contractData.contractAddress);
    var decimals = await contract.methods.decimals().call();
    var symbol = await contract.methods.symbol().call();
    var name = await contract.methods.name().call();
    var totalSupply = await contract.methods.totalSupply().call();
    return {decimals, symbol, name, totalSupply};
  }

  async getMintTx(contractData, amount, vault) {
    var contract = new this.web3.eth.Contract(contractData.abi, contractData.contractAddress);
    var decimals = contractData.info.decimals;
    var tokenToMint = amount * Math.pow(10, decimals);
    var encodedAbi  = contract.methods.mint(vault.address, tokenToMint).encodeABI();
    return this.initRawTx(
      {
        to: contractData.contractAddress,
        data: encodedAbi
      }, vault
    )
  }

  async getBurnTx(contractData, amount, vault) {
    var contract = new this.web3.eth.Contract(contractData.abi, contractData.contractAddress);
    var decimals = contractData.info.decimals;
    var tokenToBurn = amount * Math.pow(10, decimals);
    var encodedAbi  = contract.methods.burn(tokenToBurn).encodeABI();
    return this.initRawTx(
      {
        to: contractData.contractAddress,
        data: encodedAbi
      }, vault
    )
  }

  /**
   * Builds the parameters for a 'transfer' tokens ERC20 transaction from a CASP vault
   */
  async getTransferTx(contractData, vault, toAddress, amountTokens) {
    var contract = new this.web3.eth.Contract(contractData.abi, contractData.contractAddress);
    var decimals = contractData.info.decimals;
    var tokenToTransfer = amountTokens * Math.pow(10, decimals);
    var encodedAbi  = contract.methods.transfer(toAddress, tokenToTransfer).encodeABI();
    return this.initRawTx(
      {
        to: contractData.contractAddress,
        data: encodedAbi
      }, vault
    )
  }

  /**
   * Check if this is a valid ethereum address
   */
  isValidAddress(address) {
    return EthUtil.isValidAddress(address);
  }
}

module.exports = CaspEthUtil;
