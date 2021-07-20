# ERC20 Contract Deployment with Unbound CASP
This demo application demonstrates how to use Unbound CORE Crypto Asset Security (also referred to as "CASP") for deploying, managing and executing transactions with a custom Ethereum ERC20 contract.

#	1. CORE Overview

Unbound CORE is a pure-software solution that runs on any endpoint, server or cloud, at a security level above and beyond all existing solutions for blockchain custody and trading of crypto assets.

The CORE solution is built on the technological foundation of secure multiparty computation (MPC). 

For more information, see:
- [Unbound Security website](https://www.unboundsecurity.com/solutions/crypto-asset-security/) - general product information
- [Unbound CORE User Guide](https://www.unboundsecurity.com/docs/CASP/CASP_User_Guide/Content/Products/Unbound_Cover_Page.htm) - overview and installation information
- [Unbound CORE Overview](https://www.unboundsecurity.com/docs/CASP/CASP_User_Guide/Content/Products/CASP/CASP_Offering_Description/Solution.htm) in the User Guide - information about the system components

# 2. Prerequisites

**CASP Express Deploy**

You need to have the [CASP Express Deploy](https://github.com/unboundsecurity/casp-express-deploy) installed and running.

**Ethereum Wallet**

Before running the demo, you need to have an Ethereum Ropsten Test wallet with ~1 Eth. During the demo, we will transfer some to the *Contract vault*.

Deploying the contract is done by sending a transaction to the Ethereum network, which requires a miner fee in Eth just like any other transaction.
    
One simple way to create an Ethereum Test wallet is to use [MetaMask](https://metamask.io/) inside of your Google Chrome browser.


## 2.1. Monitor Logfiles
Before beginning the demo, you may want to monitor the CASP and/or UKC logfiles. Monitoring them allows us to learn about the cryptographic operations that are forwarded to the remote UKC server by CASP and more details about the CASP operations.

### 2.1.1. CASP Logfile
To monitor the CASP logfile:
1. Open a terminal window on the docker running *casp*.
2. The log file is */var/log/unbound/casp.log*.

See [here](https://www.unboundsecurity.com/docs/CASP/CASP_User_Guide/Content/Products/CASP/CASP_User_Guide/Audit_and_Logging.htm) for more information about CASP log files.


### 2.1.2. UKC Logfile

To monitor the UKC logfile:
1. Open a terminal window on the docker running *ukc-ep*.
2. The log file is */opt/ekm/logs/ekm.log*.

See [here](https://www.unboundsecurity.com/docs/UKC/UKC_Maintenance/Content/Products/UKC-EKM/UKC_User_Guide/UG-Maint/AuditLogs.html) for more information about UKC log files.

## 2.2. Debug
The CASP commands can be run in a debug mode that shows all the relevant curl commands for each of the operations.

To enable debug mode, run the CASP commands as follows:
```
 DEBUG=super-* ./casp <COMMAND>
```
**Note**: It is recommended to **only** run the demo with debug mode if you specifically need to see the curl commands.


# 3. Running the Demo
Use the following steps to run the CASP demo.

1. Access the device where you have the GitHub repo downloaded.
1. Navigate to the ERC20 directory: `unbound-sample-code/blockchain/ethereum/erc20`

Note that these files were downloaded from the [GitHub repository](https://github.com/unboundsecurity/unbound-sample-code/tree/main/blockchain/ethereum/erc20).
1. Run the following command: `npm i`
3. **Reset** - This step cleans up any data from previously running the demo.
   ```
   ./casp reset
   ```
4. **Show config** - the configuration file contains all the setup information needed for CASP. It defines the acount name, vault names, the Infura token, the Ethereum gas price, and more. Modify these values as needed.
    ```
    cat ./config.json
    ```
    - If you find that transactions are too slow, you can change the Ethereum gas price.
    - Update the name of the CASP bot. It can be found in the CASP UI [Participants screen](https://localhost/caspui/#/participants).
    - Update the API key. To generate an API key, open the [System screen](https://localhost/caspui/#/system) in the CASP UI. Then click on **Generate API Key**. On the screen that opens, you must select *Super User* for the *Role* field. The *Client ID* field is the ID of the device where you are running these demo commands.
	
3. **Run the demo** - Follow the onscreen instructions. The demo uses a contract, defined in *./contracts/UnboundToken.sol*, which is compiled in this step.
    ```
    ./casp start
    ```
    Before deploying the contract, we need an Ethereum address and a private key. To accomplish this step, first we set up a CASP vault where the key is stored.
    
    The vault is created with a single participant, named **casp-bot<unique ID>**. This participant is needed to approve operations on the vault.
	
	**Note:** The following error can safely be ignored. It is a result of using self-signed certificates, which are only used for demo purposes.
	```
	(node:30960) Warning: Setting the NODE_TLS_REJECT_UNAUTHORIZED environment variable to '0' makes TLS connections and HTTPS requests insecure by disabling certificate verification.
	```
5. **Show info** - you can run this command at any time to show the status of the vaults in your account.
    ```
    ./casp info
    ```
6. **Withdraw** - you can run this command at any time to send assets from a vault to an address.
   ```
    ./casp withdraw
    ```
    Note that in the bot participant, you see the parsed transaction, such as:
    ```
    parsed ETH:
    contract address: CE152698949F45976A3F28AD696E3E1B43CA954E
    recipient address: FE32A76A5D40BEA31749439C2B39EFC0AC3346AC
    gas limit: 51498
    value: 4000000
   ```
   
	
# 4. Other Information

## 4.1. CASP Documentation
CASP has these associated documents:

- [CASP Release Notes](https://www.unboundsecurity.com/docs/CASP/CASP_Release_Notes/Content/Products/Unbound_Cover_Page.htm)
- [CASP Frequently Asked Questions](https://www.unboundsecurity.com/docs/CASP/CASP_FAQ/Content/Products/Unbound_Cover_Page.htm)
- [CASP Developers Guide with API Reference](https://www.unboundsecurity.com/docs/CASP/CASP_Developers_Guide/Content/Products/Unbound_Cover_Page.htm)
- [CASP User Guide](https://www.unboundsecurity.com/docs/CASP/CASP_User_Guide/Content/Products/Unbound_Cover_Page.htm)
- [CASP Java SDK](https://www.unboundsecurity.com/docs/CASP/CASP_Developers_Guide/Content/Products/CASP/CASP_Participant_SDK/CASP_Java_SDK.htm)

## 4.1. APIs for Common Functions
The following list provides links to CASP APIs that provide common wallet functions.
- Wallet creation - [Create new vault](https://www.unboundsecurity.com/docs/CASP/API/casp-byow.html#create-new-vault)
    - Generate address - [Create a new address](https://www.unboundsecurity.com/docs/CASP/API/casp-byow.html#create-a-new-address)
 	- Retrieve public key - [Get vault public key](https://www.unboundsecurity.com/docs/CASP/API/casp-byow.html#get-vault-public-key)
- Get value of funds of an address/account - [Get wallet balance](https://www.unboundsecurity.com/docs/CASP/API/casp-coin.html#get-wallet-balance)
- Transaction signing - Start a signing process
    - Send/Receive/Mint funds to an address/account - [Create withdrawal request](https://www.unboundsecurity.com/docs/CASP/API/casp-coin.html#create-withdrawal-request)

A full API reference is found in the [CASP Developers Guide](https://www.unboundsecurity.com/docs/CASP/CASP_Developers_Guide/Content/Products/CASP/CASP_Developers_Guide/API_Reference.htm).

# 5. Next Steps
The following steps may be useful in further utilizing CASP capabilities.

## 5.1. Approval Quorum Groups
You can change the approval quorum groups that are used to approve operations. To enable more complex groups, Unbound provided 5 bots in the customer VM. The bots are named **bot_a** through **bot_e**.

To modify the groups:
1. Edit `config.json`.
1. Locate the relevant vault in the *config.json* file.
1. Each vault has a group definition, such as:
    ```
	"groups": {
        "A": {
          "requiredApprovals": 1,
          "members": [
            "bot_a",
            "bot_b"
          ]
        }
      }
    ```
1. Modify the group definition as needed. 
    - The parameter *requiredApprovals* determines how many participants are **required** for approval. 
	- The parameter *members* determines which participants are part of the group.
	- You can add multiple groups using the format in the file (In this version it is  limited to 2 groups).
1. Save the file.
1. Reset the demo and then start it by running:
   ```
    ./casp reset
	./casp start
   ```
The relevant bots will receive join requests for vaults that they are part of.

For example, to create 2 groups, where one group has 1 out of 2 required and the other group has 2 out of 3 required, you can use this definition:
   ```
    "groups": {
        "A": {
          "requiredApprovals": 1,
          "members": [
            "bot_a",
            "bot_b"
          ]
        }
		"B": {
          "requiredApprovals": 2,
          "members": [
            "bot_c",
            "bot_d",
            "bot_e"
          ]
        }
      }
   ```


## 5.2. Mobile App
You can use the CASP mobile app to approve transactions. First, in the [Web UI](https://www.unboundsecurity.com/docs/CASP/CASP_User_Guide/Content/Products/CASP/CASP_User_Guide/Web_Interface.htm) create a [new participant](https://www.unboundsecurity.com/docs/CASP/CASP_User_Guide/Content/Products/CASP/CASP_User_Guide/Web_Interface.htm#Participants) and activate it on the mobile app. Next, add the participant to the vault approval group using the procedure described in the previous section.

See [CASP Mobile App](https://www.unboundsecurity.com/docs/CASP/CASP_User_Guide/Content/Products/CASP/CASP_User_Guide/Mobile_App.htm) for more information.
