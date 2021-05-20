"use strict";
const inquirer = require('inquirer');
const util = require('./util');
const Promise = require('bluebird');

/**
 * Selects an active participant.
 * If multiple active participants are available, try to use the last selected one
 * or prompt the user to choose one.
 *
 * If there's only one active participant, use it.
 *
 * If there are no active participants, prompt the user to create a new one and wait
 * for its activation.
 *
 * @param  {Object} options
 * @param  {string} options.caspMngUrl - The URL of CASP management API
 * @param  {Object} options.activeAccount - Details of the active casp accounts (id, name)
 * @param  {Object} options.activeParticipant - Details of the last used participant(id, name)
 * @return {Object} The selected participant details (id, name etc...)
 */
async function selectParticipant(options) {
  const participantsUrl = `${options.caspMngUrl}/accounts/${options.activeAccount.id}/participants`;
  util.showSpinner('Fetching participants');
  var participants = (await util.superagent.get(participantsUrl)).body;
  participants = participants.items || participants;
  util.hideSpinner();
  participants = participants.filter(p => p.isActive);

  // use previously selelcted participant or only active participant
  var selected = participants.find(p => p.id === (options.activeParticipant || {}).id);
  if(participants.length === 1) selected = participants[0];
  // or if more than one active participant, let the user choose
  while(!selected) {
    // more than one active participant and not previously selected
    if(participants.length) {
      // let the user choose the participant
      selected = (await inquirer.prompt([{name: 'participant', type: 'list',
        choices: participants.map(p => ({name: p.name, value: p})),
        message: 'Select participant: ',
        validate:util.required('Participant')
      }])).participant;
    } else {
      util.log('No active participants found, please create one');
      var pData = await inquirer.prompt([
        {name: 'name', message: 'Participant Name:', default: "Bot1",
          validate:util.required('Participant Name')},
        {name: 'email', message: 'Email:', default: answers => `${answers.name}@caspbots.com`, validate:util.required('Email')},
        {name: 'role', message: 'Role:', default: "Bot"}
      ]);
      try {
        util.showSpinner('Creating participant');
        var newP = (await util.superagent.post(participantsUrl).send(pData)).body;
        util.hideSpinner();
        util.log(`Participant '${newP.name}' created. Participant needs activation`);
        util.log(`To activate with phone, use: Participant ID: '${newP.id}', Activation code: '${newP.activationCode}'`);
        util.log(`To activate as BOT, run: 'java -Djava.library.path=. -jar BotSigner.jar -u http://localhost/casp -p ${newP.id} -w 1234567890 -c ${newP.activationCode}'`);
        util.showSpinner('Waiting for activation');
        try {
          while(!newP.isActive) {
            newP = (await util.superagent.get(`${options.caspMngUrl}/participants/${newP.id}`)).body;
            await Promise.delay(500);
          }
          selected = newP;
          util.hideSpinner();
        } catch(e) {
          util.logError(e);
        }
      } catch(e) {
        util.logError(e, 'Could not create participant');
        selected = undefined;
      }
    }
  }

  util.log(`Using participant: '${selected.name}' as the only quorum approver`);
  return selected;
}

module.exports = {
  selectParticipant
}
