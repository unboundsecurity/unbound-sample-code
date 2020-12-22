const axios = require('axios');

class _ApiClient {
    basePath = ''
    authentications = { basicAuth: {} }
}
const ApiClient = new _ApiClient();
ApiClient.instance = ApiClient;

function setupAxiosClient() {
    basicAuth = ApiClient.authentications.basicAuth
    return axios.create({
        baseURL: ApiClient.basePath + '/api/v1',
        timeout: 1000,
        headers: {
            'Content-Type': 'application/json',
            'Authorization': 'Basic' + Buffer.from(basicAuth.username + ":" + basicAuth.password).toString("base64"),
        }
    });
}

class KeysApi {
    constructor() {
        this.ukcCient = setupAxiosClient()
    }
    async getPublicKeyData(keyId) {
        try {
            const response = await this.ukcCient.get('/keys/' + keyId + '/public');
            return response.data
        } catch (error) {
            console.error(error);
        }
    }
    async signWithKey(keyId, params) {
        try {
            const response = await this.ukcCient.post('/keys/' + keyId + '/sign', params.body);
            return response.data
        } catch (error) {
            console.error(error);
        }
    }
}

// async function test() {
//     let keyId = '2d0c9f4c0423974f'
//     ApiClient.authentications.basicAuth = { username: 'user@casp', password: 'Unbound1!' }
//     ApiClient.basePath = 'https://localhost:8443'
//     keysApi = new KeysApi()
//     resp = await keysApi.getPublicKeyData(keyId)
//     console.log(resp)
//     resp = await keysApi.signWithKey(keyId,
//         {
//             body: {
//                 data: '0102abcd',
//                 dataEncoding: "HEX",
//                 doHash: false,
//             }
//         }
//     )
//     console.log(resp)
// }

// test()
//     .then(() => console.log(`Transaction sent`))
//     .catch(e => {
//         console.log(e);
//     })

module.exports = { ApiClient, KeysApi }