#!/bin/bash
docker rm -f ukc-client
docker run  --env EP_HOST_NAME=ep --name ukc-client --add-host ep:18.159.194.214 -i unboundukc/ukc-client:demo-java-encrypt

# docker run --env EP_HOST_NAME=ukc-ep --name ukc-client --network=casp_default -i unboundukc/ukc-client:demo-java-encrypt