"use strict";
const Promise = require('bluebird');
const Spinner = require('cli-spinner').Spinner;
const figlet = require('figlet');
const figletP = Promise.promisify(figlet);
const wrap = require('word-wrap');
const readline = require('readline');

const titleOptions = {
  font: "Calvin S"
}

/**
 * A utility class providing various console-appliation services
 */
class CliApp {


  /**
   * Prints a message to console
   */
  log(message) {
    var indent = "  ";
    message = (message || "").toString();
    if(message.startsWith(">> ")) {
      message = message.substr(3);
      indent = "> ";
    }
    message = wrap(message, {width: 80, indent});
    console.log(message || '');
  }


  /**
   * Prints an emphasized large title to console
   */
  async writeTitle(title, first) {
    if(!first) console.log();
    var lines = await figletP(title, titleOptions);
    lines = lines.split("\n");
    lines.forEach((l, i) => {
      if(i < (lines.length - 1)) {
        this.log(l)
      } else {
        process.stdout.write("  " + l);
      }
    });
    // this.log(lines);
    var width = process.stdout.columns;
    var text = "Unbound ERC20 Demo";
    readline.cursorTo(process.stdout, width - text.length - 3 );
    process.stdout.write(text + '\n');
    this.log(new Array(width-4).join("="));
  }

  // inquirer validator
  required(name) {
    return (v) => v && true || `${name} is required`;
  }


  /**
   * Exrtracts an error message to show the user from an error
   */
  getErrMessage(e) {
    var resp = e.response || {};
    return resp && resp.body && resp.body.title
      || resp.error || e.message;
  }


  /**
   * Prints an error message to the console
   */
  logError(e, title) {
    this.hideSpinner();
    title = title || 'Something went wrong';
    this.log(`${title}: ${this.getErrMessage(e)}`);
    // var details = e.response && e.response.text;
    // if(details) {
    //   this.log(details);
    // } else {
    //   this.log(e);
    // }
    // console.log(e);
  }


  /**
   * Shows an animated spinner with a text as indicator for ongoing activity
   */
  showSpinner(text) {
    var spinner = this.spinner = new Spinner(text);
    spinner.setSpinnerString('|/-\\');
    spinner.start();
    return spinner;
  }


  /**
   * Shows an animated spinner while an asynchronous activity (Promise) is taking place.
   * The spinner is removed when the activity ends.
   */
  async runWithSpinner(promise, message) {``
    try {
      this.showSpinner(message);
      return await promise;
    } finally {
      this.hideSpinner();
    }
  }


  /**
   * Hides an active spinner.
   */
  hideSpinner() {
    if(!this.spinner) return;
    this.spinner.stop(true);
    this.spinner = undefined;
  }


  /**
   * Wait for the user to click any key.
   * Exit the program if Esc or CTRL-C are clicked.
   */
  async clickAnyKeyToContinue(msg) {
    msg = msg || 'Click any key to continue...'
    msg = ">> " + msg;
    process.stdout.write(msg);
    process.stdin.setRawMode(true);
    process.stdin.resume();
    return new Promise(resolve => {
      var listener = (keys) => {
        var key = keys[0];
        process.stdin.setRawMode(false);
        readline.clearLine(process.stdout, 0);
        readline.cursorTo(process.stdout, 0);
        process.stdin.pause();
        process.stdin.removeListener('data', listener);
        if(key === 27 || key === 3) {
          process.exit(0);
        }
        resolve();
      }
      var p = process.stdin.on('data', listener);
    })
  }

  /**
   * Waits with animated message for 600 ms
   */
  async waitWithMessage(msg, clearLine) {
    if(clearLine) {
      readline.clearLine(process.stdout, 0);
      readline.cursorTo(process.stdout, 0);
    }
    process.stdout.write("> " + msg);
    for(let i=0; i<4; i++) {
      await Promise.delay(200);
      process.stdout.write(".");
    }
  }

  /**
   * Waits until a condition is met with animated message
   */
  async waitUntil(test, msg, deep) {
    var result = await test();
    if(result) {
      if(deep) readline.clearLine(process.stdout, 0);
      return result;
    }
    process.stdout.write('> ' + msg);
    for(let i=0; i<4; i++) {
      await Promise.delay(200);
      process.stdout.write(".");
    }
    readline.clearLine(process.stdout, 0);
    readline.cursorTo(process.stdout, 0);
    return this.waitUntil(test, msg, true);
  }
}

module.exports = CliApp;
