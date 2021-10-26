var CASP_API_BASE;
const inquirer = require('inquirer');
const util = require('./util');
const vaults = require('./vaults');
const transactions = require('./transactions');
const superagent = util.superagent;
const fs = require('fs');
const { default: axios } = require('axios');

// Use locally saved data from previous executions.
// For example, if we already created a vault, use it and don't create again.
// The current execution data is saved at the end.
// To reset, dataFile can be deleted before the next execution.
var appData = {};

// disable certificate validation to allow self-signed certificates used by CASP
process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = 0;

async function test() {
  try {
    var appData = await init();
    util.log();
    appData.caspMngUrl = `${appData.caspUrl}/casp/api/v1.0/mng`;

    var login = require('./login');
    var caspCredentials = await login(appData);
    save({ caspCredentials });

    var accounts = require('./accounts');
    var activeAccount = await accounts.selectActiveAccount(appData);
    save({ activeAccount });

    var participants = require('./participants');
    var activeParticipant = await participants.selectParticipant(appData);
    save({ activeParticipant });

    var activeVault = await vaults.selectActiveVault(appData);
    save({ activeVault });

    switch (appData.demoType) {
      // shows how to generate public key
      case 'GEN_PUB_KEY':
        var publicKey = await transactions.getPublicKeyFromCasp(appData);
        util.log(`Public key:`)
        util.log(JSON.stringify(publicKey, undefined, 2));
        save({ publicKey });
        break;

      // full cycle of deposit, signature and withdrawal for Bitcoin BYOW
      case 'BTCTEST_FULL':
        var addressInfo = appData.addressInfo || await transactions.createAddress(appData);
        save({ addressInfo });

        addressInfo.balance = await transactions.waitForDeposit(appData);
        save({ addressInfo });

        var pendingTransaction = appData.pendingTransaction ||  await transactions.createTransaction(appData);
        save({ pendingTransaction });

        if (!pendingTransaction.signed) {
          let signed = await transactions.signTransaction(appData);
          pendingTransaction.signed = signed;
          save({ pendingTransaction });
        }

        pendingTransaction.transactionHash = pendingTransaction.transactionHash
          || await transactions.sendTransaction(appData);
        save({ pendingTransaction: undefined, lastSuccessfulTransaction: pendingTransaction });

        break;
    }

  } catch (e) {
    util.logError(e);
    console.log(e);
  }
}


/**
 * Initialize and verify connection to CASP server and Infura ledger server
 */
async function init() {
  try {
    appData = require('../data/data');
  } catch (e) { }

  var caspUrl;
  var defaultUrl = 'https://localhost';
  while (!caspUrl) {
    caspUrl = appData.caspUrl || (await inquirer.prompt([{
      name: 'url', message: 'CASP URL: ',
      default: defaultUrl, validate: util.required('CASP URL')
    }])).url;
    CASP_API_BASE = `${caspUrl}/casp/api/v1.0/mng`;
    try {
      util.showSpinner('Connecting to CASP');
      var result = await superagent.get(`${CASP_API_BASE}/status`)
        .query({ withDetails: true });
      util.hideSpinner();
      util.log(`Connected to CASP at ${caspUrl}`)
      util.log(result.body);
      appData.caspUrl = caspUrl;
      appData.caspVersion = result.body.version;
      try {
        appData.caspRelease = parseInt(appData.caspVersion.match(/\d+\.\d+\.(\d+)\./)[1]);
      } catch (e) { }
      save();
    } catch (e) {
      util.hideSpinner();
      util.logError(e);
      util.log(`Could not connect to CASP at '${caspUrl}'. Please check that the URL is correct`);
      defaultUrl = caspUrl;
      appData.caspUrl = caspUrl = undefined;
    }
  }

  var demoType = appData.demoType = appData.demoType || (await inquirer.prompt([
    {
      name: "demoType", message: "Select demo to run: ",
      default: 'BTCTEST_FULL',
      type: "list", choices: [
        {
          name: 'BYOW full cycle with Bitcoin deposit and withdrawal',
          value: 'BTCTEST_FULL'
        },
        {
          name: 'Public key generation only',
          value: 'GEN_PUB_KEY'
        }
      ]
    }
  ])).demoType;
  if (demoType === 'BTCTEST_FULL') {
    var response;
    while(!(response && response.status === 200)) {
      let jwtToken = appData.blocksetJwtToken || (await inquirer.prompt([{
        name: 'token', message: `Blockset JWT token: `,
        validate: util.required('Blockset token')
      }])).token;
      
      try {
        util.showSpinner('Checking blockset health')
        response = await axios({
          method: 'get', url: 'https://api.blockset.com/blocks?max_page_size=5&blockchain_id=bitcoin-testnet',
          headers: { Authorization: `Bearer ${jwtToken}` }
        });
        util.hideSpinner();
        if(response.status === 200) {
          util.log(`Connection Healthy!`);
          appData.blocksetJwtToken = jwtToken;
          save();
        } else {
          throw "Connection to Blockset failed: " + response.statusText;
        }
      } catch (e) {
         jwtToken = appData.blocksetJwtToken = undefined;
        util.hideSpinner();
        util.log(`Could not connect to Blockset, please check your token and try again: ${e.message}`);
      }
    }
  }
  return appData;
}


/**
 * Saves data between app sessions
 *
 * @param  {type} data - optional, data to append to appData
 */
function save(data) {
  if (data) {
    Object.assign(appData, data);
  }
  if (!fs.existsSync('./data')) {
    fs.mkdirSync('./data');
  }
  fs.writeFileSync('./data/data.json', JSON.stringify(appData, undefined, 2));
}

module.exports = test;
