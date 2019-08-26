#!/bin/bash

# Small script to create the database that we will run the pubsub tests
# on.

set -e

export DBFILE=./localdb.sqlite
export DBARGS="--database=$DBFILE --database-type=sqlite"
export FPERMS=`echo {0..63}`
export ENDPOINTS='

   _ALL_
   all
   '

rm -rfv ./localdb.sqlite
sqldb_auth_cli.elf create $DBFILE sqlite
sqldb_auth_cli.elf init sqlite $DBFILE
sqldb_auth_cli.elf $DBARGS user_create admin@example.com admin 123456

for X in $ENDPOINTS; do
   echo Granting all permissions for $X to admin@example.com
   sqldb_auth_cli.elf $DBARGS grant_user admin@example.com $X $FPERMS
done

chmod a+w $DBFILE
