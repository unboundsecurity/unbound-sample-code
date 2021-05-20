#!/bin/bash
docker rm -f ukc-client
# docker run  --env EP_HOST_NAME=ep --name ukc-client --add-host ep:18.159.194.214 -i unboundukc/ukc-client:demo-java-encrypt

docker run --name ukc-client \
        --network=casp_default \
        --env EP_HOST_NAME=ukc-ep \
        --env UKC_CRYPTO_USER=encrypter \
        --env UKC_CRYPTO_USER_PASSWORD=Password1! \
        -i unboundukc/ukc-client:demo-java-encrypt