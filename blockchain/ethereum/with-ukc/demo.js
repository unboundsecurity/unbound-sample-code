const UkcEthereumWallet = require('./ukc-ethereum-wallet');

(async function run() {
	try {
		// get your ethereum ropsten provider endpoint by creating an account in https://infura.io
		var wallet = new UkcEthereumWallet("https://<network>.infura.io/v3/YOUR-PROJECT-ID");

		await wallet.init();
		await wallet.send({
			to: '0xDC839532D5ab3e7Cc958b4F86d733CbD1617ee4A',
			amountInEth: "0.0004"
		})
	} catch (e) {
		const ukcErrorData = e.response && e.response.data;
		console.log(ukcErrorData || e.toString());
	}
})();