#!/bin/bash
#
# This script was created by Unbound Tech to list the clients along with their certificate expiration dates for UKC.
# The script runs on all partitions in UKC that are inherited.
#

# Must install jq 'yum -y install jq'

if [ $# -lt 2 ]; then
	echo 'Usage: ukc-list-clients.sh <UKC_URL> <ROOT_SO_PASSWORD> [EXPIRY_DAYS] '
	echo 'If EXPIRY_DAYS is not provided, all clients are listed'
	exit
fi

UKC_URL=$1
ROOT_SO_PASSWORD=$2
# Filter clients with expiration of maximum expiration days from now
EXPIRY_DAYS=$3

mapfile -t parts < <(curl --silent -k -u so@root:$ROOT_SO_PASSWORD $UKC_URL/api/v1/partitions | jq  -r '.[] | select(.supportPartitionInheritance==true) | .name ');

echo "Partition, Client name, Client certificate expiration date"
echo "----------------------------------------------------------"

for part in "${parts[@]}"; do 
    info=( `curl --silent -k -u so@root:$ROOT_SO_PASSWORD -G -d partitionId=${part} $UKC_URL/api/v1/clients \
        | jq -r '.[] | .name + " " + .expiresAt ' ` ); 
	name=${info[0]}
	exp=${info[1]}
	if [ -z $exp ] ; then continue ; fi
	expSec=`date -d $exp "+%s"`
	nowSec=`date +%s`
	expDays=$(( (expSec-nowSec)/(60*60*24) ))
	if [ ! -z $EXPIRY_DAYS ]; then
		if [ $expDays -lt $EXPIRY_DAYS ]; then
			continue
		fi
	fi
	echo $part,$name,$exp 
done
