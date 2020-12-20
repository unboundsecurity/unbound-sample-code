const superagentJsonapify = require('superagent-jsonapify');
const superdebug = require('superdebug');
const defaults = require('superagent-defaults');
const superagent = defaults();
const Promise = require('bluebird');
const Web3 = require('web3');

superagentJsonapify(superagent);

// CASP uses ,for the demo, a self-signed certificate so we need to avoid
// validating its root CA
process.env["NODE_TLS_REJECT_UNAUTHORIZED"] = 0;

class CaspClient {
  constructor(config) {
    if(!config) throw new Error("Must provide a config file");
    this.config = config;
    superagent.set('Authorization', 'Bearer ' + config.apiKey);
    this.caspBaseUrl = `https://${config.caspHost}/casp/api/v1.0/mng`;
  }

  get superagent() {
    return this._superagent = this._superagent || superagent.use(superdebug());
  }

  // Initializes the active account and pre-loads participants
  async init() {
    if(this.initialized) return;
    await this.initActiveAccount();
    await this.loadParticipants();
    this.initialized = true;
  }

  async initActiveAccount() {
    if(this.initialized) return;
    // CASP can have many top level accounts
    this.activeAccountId = this.config.activeAccountId
                  || this.config.acitveAccount && this.config.activeAccount.id;
    if(!this.activeAccountId) {
      var allAccounts = await this.listAccounts();
      this.activeAccountId = allAccounts[0].id;
    }
  }

  async loadParticipants() {
    var participants = await this.listParticipants();
    this.participantsByName = participants.reduce((map, p) => {
      map[p.name.toUpperCase()] = p;
      return map;
    }, {})
  }

  // Lists CASP accounts. Accounts are top level containers for vaults and participants
  listAccounts() {
    return this.superagent.get(`${this.caspBaseUrl}/accounts`)
      .then(res => res.body);
  }

  listParticipants(accountId) {
    return this.superagent.get(`${this.caspBaseUrl}/accounts/${encode(this.activeAccountId)}/participants`)
      .then(res => res.body)
  }

  async findVaultByName(name) {
    await this.initActiveAccount();
    var res = await this.superagent.get(`${this.caspBaseUrl}/accounts/${encode(this.activeAccountId)}/vaults?limit=1&filter=${encodeURIComponent(String(name))}`)
    var vault = (res.body && res.body.items || [])[0];
    if(vault && vault.name.toUpperCase() === name.toUpperCase()) {
      return vault;
    }
  }

  createVault(params) {
    if(params.hierarchy === "BIP44") {
      if(params.coinType === undefined) throw new Error(`Must specify coinType for BIP44 vault '${params.name}'`);
      if(params.firstAccountName === undefined) throw new Error(`Must specify firstAccountName for BIP44 vault '${params.name}'`);
    }
    return this.superagent.post(`${this._accountUrl()}/vaults`)
      .send(params)
      .then(res => this.getVault(res.body));
  }

  async getVault(vaultOrVaultId) {
    return this.superagent.get(`${this._vaultBaseUrl(vaultOrVaultId)}`).then(res => res.body)
  }

  async getVaultDefaultKey(vault) {
    if(vault.hierarchy === 'NONE') {
      return this.getVaultPublicKey(vault);
    } else {
      var coins = await this.getVaultCoins(vault);
      var defaultCoin = coins[0];
      if(defaultCoin === undefined) throw new Error(`No coins for vault '${vault.name}'`);
      var accounts = await this.getVaultAccounts(vault, defaultCoin);
      var defaultAccount = accounts[0];
      if(defaultAccount === undefined) throw new Error(`No accounts for vault '${vault.name}' coin ${defaultCoin}`);
      return this.getLastBip44VaultPublicKey(vault, defaultCoin, defaultAccount.index, 'external');
    }
  }

  _vaultBaseUrl(vaultOrVaultId) {
    var vaultId = vaultOrVaultId.id || vaultOrVaultId.vaultId || vaultOrVaultId;
    return `${this.caspBaseUrl}/vaults/${encode(vaultId)}`;
  }

  _accountUrl() {
    return `${this.caspBaseUrl}/accounts/${encode(this.activeAccountId)}`;
  }

  /**
   * Returns the single public key for non HD vaults
   * If the vault supports HD (hierarchy != NONE) this will return an error
   * The returned key is in DER format
   */
  getVaultPublicKey(vaultOrVaultId) {
    return this.superagent.get(`${this._vaultBaseUrl(vaultOrVaultId)}/publickey`)
      .then(res => res.body.publicKey);
  }

  getLastBip44VaultPublicKey(vaultOrVaultId, coinType, accountIndex, chain) {
    return this.superagent.get(`${this._vaultBaseUrl(vaultOrVaultId)}/coins/${encode(coinType)}/accounts/${encode(accountIndex)}/chains/${encode(chain)}/addresses?limit=1&sort=lastUpdatedAt:desc`)
      .then(res => res.body.items && res.body.items[0])
  }

  getVaultCoins(vaultOrVaultId) {
    return this.superagent.get(`${this._vaultBaseUrl(vaultOrVaultId)}/coins`)
      .then(res => res.body && res.body.coins)
  }

  getVaultAccounts(vaultOrVaultId, coinType) {
    return this.superagent.get(`${this._vaultBaseUrl(vaultOrVaultId)}/coins/${encode(coinType)}/accounts`)
      .then(res => res.body)
  }

  async createSignOperation(vaultId, signRequest) {
    var opId = (await this.superagent.post(`${this._vaultBaseUrl(vaultId)}/sign`)
      .send(signRequest)).body.operationID;
    return this.getSignOperation(opId);
  }

  async getSignOperation(opId) {
    opId = opId.id || opId.operationID || opId;
    return this.superagent.get(`${this.caspBaseUrl}/operations/sign/${opId}`)
      .then(res => res.body);
  }

  async getOperation(opId) {
    opId = opId.id || opId.operationID || opId;
    return this.superagent.get(`${this.caspBaseUrl}/operations/${encode(opId)}`)
      .then(res => res.body);
  }

  async getPendingOps() {
    var opRes = await this.superagent.get(`${this._accountUrl()}/operations?limit=10&status=PENDING&sort=createdAt:desc`)
    return opRes.body.items || [];
  }

  /**
   * Searches for the last pending vault-join operation
   * This is used for waiting until the vault approved by all members
   */
  async getPendingJoinOp(vault) {
    var ops = await this.getPendingOps();
    var joinOps = ops.filter(o => (o.vaultID === vault.id && o.kind === 'JOIN_VAULT'));
    var op = joinOps[0];
    if(op) return await this.getOperation(op);
  }

  /**
   * Initializes the payload for a new vault from the vault data in the config file
   * This includes finding participants by name and verifying their status
   */
  getNewVaultPayload(vaultData) {
    var payload = {
      cryptoKind: "ECDSA"
    }
    // init new vault properties from config
    var copyAttrs = "name cryptoKind providerKind hierarchy description";
    if(vaultData.hierarchy === "BIP44") {
      // these properties are relevant only for vaults with BIP44 hd wallet support
      copyAttrs = copyAttrs + " firstAccountName coinType";
    }
    copyAttrs.split(' ').forEach(attr => payload[attr] = vaultData[attr] || payload[attr]);

    // init new vault groups
    payload.groups = Object.keys(vaultData.groups).map(groupName => {
      var groupConfig = vaultData.groups[groupName];
      var members = groupConfig.members.map(memberName => {
        // find member id
        var member = this.participantsByName[memberName.toUpperCase()];
        if(!member) throw new Error(`Can't create vault '${vaultData.name}': Member '${memberName}' does not exist`)
        if(!member.isActive) throw new Error(`Can't create vault '${vaultData.name}': Member '${memberName}' is not active (${member.status})`);
        return {
          id: member.id
        }
      });
      return {
        name: groupName,
        requiredApprovals: groupConfig.requiredApprovals,
        members: members
      }
    })
    return payload;
  }

}

function encode(str) {
  return encodeURIComponent(String(str));
}

module.exports = CaspClient;
