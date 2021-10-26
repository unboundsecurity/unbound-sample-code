module.exports = {
  WELCOME: 'Welcome to the Unbound Security CASP ERC20 Demo.\n'
            + 'This demo guides you through the process of an ERC20 contract deployment and usage.',

  BEFORE_COMPILE: (contractName) => `Lets start by compiling our contract: ${contractName}`,

  CONTRACT_ALREADY_COMPILED: (contractName) => `Our contract is already compiled: ${contractName}`,

  FIRST_VAULT_CONTRACT:
  'Before we can deploy our contract, we need an Ethereum address and a private key.'
  + '\nCASP to the Rescue !'
  + '\nWe are about to create our first CASP vault which will securely store our key.',

  NEED_ETH: `We need some Eth for transaction fee before we can deploy our contract.` + `\nThe recommended amount is no less than 0.03 Eth ...`,

  READY_TO_DEPLOY: 'We are ready to deploy our contract. Click any key to continue..',

  RESET_WARNING: 'About to delete compiled contracts and reset config file data.\n'
    + 'After reset, any funds you might have in current vault addresses will be accessible only from CASP UI.',

  EXCHANGE_VAULT: 'Now that we got our contract deployed and minted, lets create another CASP Vault that will be used to exchange our token.'
    +'\nThis vault will also support multi-addresses with BIP44 HD derivation.',

  TRANSFER_TO_EXCHANGE: 'Lets transfer some tokens from our contract vault to our exchange vault.',

  GOODBYE: 'Please let us know if you have any questions.\n\nGoodbye.'

}
