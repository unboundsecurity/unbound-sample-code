"use strict";
const Spinner = require('cli-spinner').Spinner;
const superagentJsonapify = require('superagent-jsonapify');
const superdebug = require('superdebug');
const defaults = require('superagent-defaults');
const superagent = defaults();


superagentJsonapify(superagent);

module.exports = class Util {
  static log(message) {
    console.log(message || '');
  }

  static get superagent() {
    return superagent.use(superdebug())
  }

  // inquirer validator
  static required(name) {
    return (v) => v && true || `${name} is required`;
  }

  static getErrMessage(e) {
    var resp = e.response || {};
    return resp && resp.body && resp.body.details
      || resp.error || e.message;
  }

  static logError(e, title) {
    this.hideSpinner();
    title = title || 'Something went wrong';
    this.log(`${title}: ${this.getErrMessage(e)}`);
    var details = e.response && e.response.text;
    if(details) {
      this.log(details);
    }
  }

  static showSpinner(text) {
    var spinner = this.spinner = new Spinner(text);
    spinner.setSpinnerString('|/-\\');
    spinner.start();
    return spinner;
  }

  static hideSpinner() {
    if(!this.spinner) return;
    this.spinner.stop(true);
  }

}
