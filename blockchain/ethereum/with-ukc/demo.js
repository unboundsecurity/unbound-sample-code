const UkcEthereumWallet = require('./ukc-ethereum-wallet');

(async function run() {
	try {
		// get your ethereum ropsten provider endpoint by creating an account in https://infura.io
		const web3ProviderUrl = "https://ropsten.infura.io/v3/eece94a9579c406a84634e0f0d42d1c3";
		const ukcHost = "https://localhost:8443";
		if(!web3ProviderUrl) {
			throw "Please set your web3 / Infura provider url, such as 'https://<network>.infura.io/v3/YOUR-PROJECT-ID'. Open account in https://infura.io"
		}
		var wallet = new UkcEthereumWallet(web3ProviderUrl, ukcHost);

		await wallet.init();
		await wallet.send({
			to: '0xDC839532D5ab3e7Cc958b4F86d733CbD1617ee4A',
			amountInEth: "0.0001"
		})
	} catch (e) {
		const ukcErrorData = e.response && e.response.data;
		console.log(ukcErrorData || e.toString());
	}
})();