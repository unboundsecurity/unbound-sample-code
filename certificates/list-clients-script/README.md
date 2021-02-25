# List Certificate Expiration Dates for UKC Clients

The *ukc-list-clients.sh* script was created by Unbound to list the UKC clients along with their certificate expiration dates.

The script runs on all partitions in UKC that are inherited.

## Usage

`ukc-list-clients.sh <UKC_URL> <ROOT_SO_PASSWORD> [EXPIRY_DAYS]`

If *EXPIRY_DAYS* is not provided, all clients are listed.
  
## Example

`./ukc-list-clients.sh https://<EP URL> <Password> 2000`

## Sample response
```
Partition, Client name, Client certificate expiration date
----------------------------------------------------------
part1,ep,2023-12-09T07:15:50Z
part10,ep,2023-12-09T07:17:16Z
part100,client211,2023-12-09T08:31:05Z
part100,client212,2023-12-09T08:31:06Z
```
