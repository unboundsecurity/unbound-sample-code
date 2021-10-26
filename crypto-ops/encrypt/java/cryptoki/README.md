# Encrypt with key rotation using Java

This demo shows how to use Unbound technology with Java for performing high throughput data encryption. It has the following features:  
* Client key caching for increased performance.
* Key rotation for increased security.
* Ephemeral clients - short lived, task oriented, containerized clients with short-time authorization.

## Instructions

### Set up UKC
1. Install and run the Unbound-NextGen-VHSM demo found [here](https://github.com/unboundsecurity/Unbound-NextGen-vHSM-Interactive-Demo).
1. Open Google Chrome and browse to the VHSM demo web-admin page at [http://localhost:8081/webadmin](http://localhost:8081/webadmin).
1. Click *Log into UKC Admin as partition SO*. A new browser tab opens with the login page for UKC Web UI.
1. Click *Login*.
1. Select the *Clients* tab.
1. Click *Create* to create a new client.
    1. In the *Client name* field enter: "template_1".
    1. In the *Registration mode* field select "Ephemeral client template".
    1. Change the *Activation code validity period in minutes* to "20000".
    1. Click *Add ephemeral client template*.
    1. A dialog with the activation code appears. **Copy the activation code**.

### Build and run the demo
1. Clone this repository to your computer:   
  `git clone https://github.com/unboundsecurity/unbound-sample-code.git`
1. Open a terminal console and go to the docker folder:  
   `cd ./unbound-sample-code/crypto-ops/encrypt/java/cryptoki/docker`
1. Edit the file `env`. Paste the activation code copied in the previous section to the CLIENT_TEMPLATE_ACTIVATION_CODE line.
   For example:
   
   `CLIENT_TEMPLATE_ACTIVATION_CODE=4767527256228252`
1. Save the `env` file.
1. Make sure you have the UKC client installation file from Unbound, which is an RPM file. For example: 
   
   `ekm-client-2.0.2010.38445-el7+el8.x86_64.rpm`
1. Copy the RPM file to `crypto-ops/encrypt/java/cryptoki/.devcontainer/data`.
1. Edit the file `Dockerfile` and replace the RPM filename with the one you got from Unbound.
1. Build the demo container by running:  
   `./build.sh`
   
   A docker image named `unboundukc/ukc-client:demo-java-encrypt` should be built.
1. During the demo it is recommended to look at the UKC log to better understand what is happening.
    1. To show the UKC log, browse to "http://localhost:8081/logs".
    1. Alternatively, you can show the log by opening a terminal and running: 
        `docker exec -it ukc-ep-vhsm tail -f /opt/ekm/logs/ekm.log`
1. Start the demo by running  
   `./run.sh`
1. Follow the instructions on the screen.
