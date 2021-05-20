#!/bin/bash
set -e

echo "UKC ready registering client"

PARTITION=$UKC_PARTITION
CLIENT="${UKC_CLIENT_NAME:-ephemeral}"
EP_HOST_NAME="${EP_HOST_NAME:-ep}"

ACTIVATION_CODE=5265731171522490

ucl register --template ephemeral -p $PARTITION  -c $ACTIVATION_CODE

echo "Client registered"
