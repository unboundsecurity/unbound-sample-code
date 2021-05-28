# Encrypt with key rotation using Java
## Overview
This demo shows how to use Unbound technology with Java for performing high throughput data encryption with the following features:  
* Client key caching for increased performance.
* Key rotation for increased security.
* Ephemeral clients - short lived task oriented containerized clients with short-time authorization.

## Instructions
### Setting up UKC
1. Install and run the Unbound-NextGen-VHSM demo https://github.com/unboundsecurity/Unbound-NextGen-vHSM-Interactive-Demo
1. Open Google Chrome and browse to the VHSM demo web-admin page at "http://localhost:8081/webadmin"
1. Click on *Log into UKC Admin as partition SO*, A new browser tab will be opened with the login page for UKC Web UI.
1. Click on *Login*
1. Go to the *Clients* tab
1. Click the *Create* button to create a new client
    1. In the *Client name* field enter: "template_1"
    2. In the *Registration mode* field select "Ephemeral client template"
    3. Change the *Activation code validity period in minutes* to 20000
    4. Click the *Add ephemeral client template* button
    5. A dialog with the activation code should appear. **Copy the activation code**

### Building and running the demo
1. Clone this repository to your computer:   
  `git clone https://github.com/unboundsecurity/unbound-sample-code.git`
3. Open a terminal console and go to the docker folder:  
   `cd ./unbound-sample-code/crypto-ops/encrypt/java/cryptoki/docker`
5. Edit the file `env`, paste the activation code copied in the previous step to the CLIENT_TEMPLATE_ACTIVATION_CODE line, 
   for example `CLIENT_TEMPLATE_ACTIVATION_CODE=4767527256228252`
1. Save the `env` file
1. Make sure you have the UKC client installation file from Unbound, this is an rpm file for example: `ekm-client-2.0.2010.38445-el7+el8.x86_64.rpm`
1. Copy the rpm into `crypto-ops/encrypt/java/cryptoki/.devcontainer/data`
1. Edit the `Dockerfile` file and replace the rpm file name with the name of the rpm file you got from Unbound
1. Build the demo container by running  
   `./build.sh`
1. A docker image named `unboundukc/ukc-client:demo-java-encrypt` should be built
1. During the demo its recommended to look at the UKC log to better understand whats happening behind the scenes
    1. To show the UKC log, browse to "http://localhost:8081/logs"
    2. Alternatively, you can show the log by opening a terminal and running `docker exec -it ukc-ep-vhsm tail -f /opt/ekm/logs/ekm.log`
3. Start the demo by running  
   `./run.sh`
1. Follow the instructions on the screen 
