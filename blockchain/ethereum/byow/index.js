var checkEngine = require('check-engine');

checkEngine().then((result) => {
  if (result.status !== 0) {
      console.log('Failed validating node version');
  } else {
    if((result.message || {}).type !== 'success') {
      console.log(result.message.text);
      result.packages.forEach(p => {
        if(p.type === 'error') {
          console.log(`'${p.name}' has invalid version; expected version is: ${p.expectedVersion}, existing version is: ${p.foundVersion}`);
        }
      })
    } else {
      var test = require('./src/test');
      test();
    }
  }
})
