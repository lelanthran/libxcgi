#!/bin/bash

# Small script to create the database that we will run the pubsub tests
# on.

set -e

export DBFILE=./localdb.sqlite
export DBARGS="--database=$DBFILE --database-type=sqlite"
export FPERMS=`echo {0..63}`
export ENDPOINTS='
   login
   logout

   user-new
   user-rm
   user-list
   user-mod

   group-new
   group-rm
   group-mod
   group-adduser
   group-rmuser
   group-list
   group-members

   grant-user
   grant-group
   grant-to-user-over-user
   grant-to-user-over-group
   grant-to-group-over-user
   grant-to-group-over-group

   revoke
   revoke-from-user-over-user
   revoke-from-user-over-group
   revoke-from-group-over-user
   revoke-from-group-over-group

   queue-new
   queue-rm
   queue-mod
   queue-put
   queue-get
   queue-del
   queue-list'

rm -rfv ./localdb.sqlite
sqldb_auth_cli.elf create $DBFILE
sqldb_auth_cli.elf init sqlite $DBFILE
sqldb_auth_cli.elf $DBARGS user_create admin@example.com admin 123456

for X in $ENDPOINTS; do
   echo Granting all permissions for $X to admin@example.com
   sqldb_auth_cli.elf $DBARGS grant_user admin@example.com $X $FPERMS
done


