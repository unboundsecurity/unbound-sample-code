"use strict";
const util = require('./util');
const inquirer = require('inquirer');

/***
 * Login to CASP.
 * If credentials are provided in options, try to use them.
 * Otherwise, prompt the user for credentials.
 * If necessary, upon first login, prompt the user to change his password.
 *
 * @param  {Object} options
 * @param  {string} options.caspMngUrl - The URL of CASP management API
 * @param  {Object} options.caspCredentials - Last used credentials for CASP
 * @param  {string} options.caspCredentials.user - Last used username
 * @param  {string} options.caspCredentials.password - Last used password
 */
async function login(options) {
  var credentials = options.caspCredentials || {};
  // try to use saved user/password
  if(!(credentials && credentials.user && credentials.password)) {
    util.log("Please enter your CASP credentials to login");
    // otherwise, prompt
    credentials = await inquirer.prompt([
      { name: 'user', type: 'input', default: 'so', validate: util.required('User') },
      { name: 'password', type: 'password', mask: '*', validate: util.required('Password')}
    ]);
  }
  util.showSpinner('Authenticating');
  try {
    var response = await util.superagent.post(`${options.caspMngUrl}/auth/tokens`)
      .auth(encodeURI(credentials.user), encodeURI(credentials.password))
      .send("grant_type=password")
      .send("client_id=test")
      .send("scope=/mng")
      .send(`username=${encodeURI(credentials.user)}`)
      .send(`password=${encodeURI(credentials.password)}`);
    util.hideSpinner();
    util.log(`'${credentials.user}' logged-in. Token will expire in ${response.body.expires_in} seconds`);
    // from now on, add the auth token to every request
    util.superagent.set({'Authorization': 'Bearer ' + response.body.access_token});
    return credentials;
  } catch(e) {
    util.hideSpinner();
    var response = e.response || {};
    if(response.status === 401) {
      if(response.body.type === '/mng/errors/password-must-be-changed') {
        // password must be changed on first login
        util.log('\nYour password must be changed');
        credentials.password = await changePassword(credentials, options.caspMngUrl);
        await login({...options, caspCredentials: credentials});
      } else {
        util.logError(e, "Login failed");
        await login({...options, caspCredentials:{}});
      }
    } else {
      util.logError(e);
      await login({...options, caspCredentials:{}});
    }
  }
}

async function changePassword(currentCredentials, caspMngUrl) {
  var answers;
  while(!answers) {
    answers = await inquirer.prompt([
      { name: 'password', message: 'New password:', type: 'password', mask: '*'
          ,validate: util.required('New password')},
      { name: 'passwordConfirm', message: 'Confirm new password:',
        type: 'password', mask: '*', validate: util.required('Confirm password')}
    ]);
    if(answers.password !== answers.passwordConfirm) {
      answers = undefined;
      util.log("Passwords don't match, please try again");
    }
  }
  var newPassword = answers.password;
  //change password
  try {
    util.showSpinner('Changing password');
    var response = await util.superagent.put(`${caspMngUrl}/auth/users/password`)
      .auth(currentCredentials.user, currentCredentials.password)
      .send({ value: encodeURI(newPassword) });
    util.hideSpinner();
    util.log('Password changed successfully');
    return newPassword;
  } catch(e) {
    util.hideSpinner();
    util.logError(e, `Password change failed`);
    return changePassword(currentCredentials, caspMngUrl);
  }
}

module.exports = login;
