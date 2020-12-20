const path = require('path');
const fs = require('fs');
const solc = require('solc');
const md5File = require('md5-file');

var solcInput = {
    language: "Solidity",
    sources: { },
    settings: {
        optimizer: {
            enabled: true
        },
        evmVersion: "byzantium",
        outputSelection: {
            "*": {
              "": [
                "legacyAST",
                "ast"
              ],
              "*": [
                "abi",
                "evm.bytecode.object",
                "evm.bytecode.sourceMap",
                "evm.deployedBytecode.object",
                "evm.deployedBytecode.sourceMap",
                "evm.gasEstimates"
              ]
            },
          }
    }

};

function findImports(importFile) {
    try {
        // Find in contracts folder first
        result = fs.readFileSync("contracts/" + importFile, 'utf8');
        return { contents: result };
    } catch (error) {
        // Try to look into node_modules
        try {
            result = fs.readFileSync("node_modules/" + importFile, 'utf8');
            return { contents: result };
        } catch (error) {
            console.log(error.message);
            return { error: 'File not found' };
        }
    }
}

function compile(contractSourceFile, options) {
    let result = false;
    if(!fs.existsSync(contractSourceFile)) {
      throw new Error(`Can't find contract source file: ${contractSourceFile}`);
    }
    fs.mkdirSync(options.outputFolderPath, {recursive: true});
    let contractFileChecksum = md5File.sync(contractSourceFile);
    var pathComponents = path.parse(contractSourceFile);
    var contractName = pathComponents.name;
    var contractFileName = contractName + pathComponents.ext;
    var jsonOutputFile = path.join(options.outputFolderPath, `${contractName}.json`);
    if(fs.existsSync(jsonOutputFile)) {
      //check if compile needed
      let jsonObject = JSON.parse(fs.readFileSync(jsonOutputFile, 'utf8'));
      let buildChecksum = '';
      if (typeof jsonObject['contracts'][contractFileName]['checksum'] != 'undefined') {
          buildChecksum = jsonObject['contracts'][contractFileName]['checksum'];
          if (contractFileChecksum === buildChecksum) return true;
      }
    }

    let contractContent = fs.readFileSync(contractSourceFile, 'utf8');
    solcInput.sources[contractFileName] = {
        "content": contractContent
    };

    var jsonOutput = JSON.parse(solc.compile(JSON.stringify(solcInput), findImports))

    let isError = false;

    if (jsonOutput.errors) {
        jsonOutput.errors.forEach(error => {
            console.log(error.severity + ': ' + error.component + ': ' + error.formattedMessage);
            if (error.severity == 'error') {
                isError = true;
            }
        });
    }

    if (isError) {
        // Compilation errors
        console.log('Compile error!');
        return false;
    }

    // Update the sol file checksum
    jsonOutput['contracts'][contractFileName]['checksum'] = contractFileChecksum;

    let formattedJson = JSON.stringify(jsonOutput, null, 4);

    fs.writeFileSync(jsonOutputFile, formattedJson);
    return true;
  }

module.exports = compile;
