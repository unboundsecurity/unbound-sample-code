"use strict";
const inquirer = require('inquirer');
const util = require('./util');
const superagent = util.superagent;

/**
 * Selects an active CASP account.
 * Tries to use last selected account.
 * If there is only one active account, use it.
 * Othewise, if there are multiple active accounts, let the user select one.
 *
 * @param  {Object} options
 * @param  {string} options.caspMngUrl - The URL of CASP management API
 * @param  {Object} options.activeAccount - last selected active account
 * @param  {string} options.activeAccount.id - id of last selected active account
 * @param  {string} options.activeAccount.name - name of last selected active account
 * @return {Object} The selected account details (id, name etc...)
 */
async function selectActiveAccount(options) {
  util.showSpinner('Fetching accounts');
  let accounts = (await superagent.get(`${options.caspMngUrl}/accounts`)).body;
  accounts = accounts.items || accounts;
  util.hideSpinner();
  var account = accounts[0];
  if(accounts.length > 1) {
    // try to use last selected account
    account = accounts.find(a => a.id === (options.activeAccount || {}).id);
    if(!account) {
      //let the user choose an account
      var answers = await inquirer.prompt([{
        name: 'account', message: 'Choose account:', type: 'list',
        choices: accounts.map((p, i) => ({...p, value: p})),
        validate: util.required('Account')
      }]);
      account = answers.account;
    }
  } else if(!accounts.length) {
    util.log('No accounts found, please create one');
    var answers = await inquirer.prompt([{
      name: 'name', message: 'Account Name:',
      default: 'Test',
      validate: util.required('Account Name')}]);
    util.showSpinner('Creating account');
    try {
      account = (await superagent.post(`${options.caspMngUrl}/accounts`)
        .send(answers)).body;
      util.hideSpinner();
    } catch(e) {
      util.logError(e);
    }
  }
  util.log(`Using account '${account.name}'`);
  return account;
}

module.exports = {
  selectActiveAccount
}
