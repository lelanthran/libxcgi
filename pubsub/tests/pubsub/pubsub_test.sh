#!/bin/bash

# The first thing to do is to install the executables for libsqldb into
# your path. When this is done, create and initialise a database for the
# test as follows:
#     $ sqldb_auth_cli create ./localdb.sqlite
#     $ sqldb_auth_cli init sqlite ./localdb.sqlite
#     $ sqldb_auth_cli user_create one@example.com user-ONE 123456\
#                      --database=./localdb.sqlite  --database-type=sqlite

# The library path I use while testing. This should probably be cleaned
# up for different targets.
export LD_LIBRARY_PATH=/home/lelanthran/lib:/home/lelanthran/opensource/libxcgi/library/debug/lib/x86_64-pc-linux-gnu:/home/lelanthran/opensource/libxcgi/pubsub/debug/lib/x86_64-pc-linux-gnu

# These variables cause the cgi program to think that it has been invoked
# by a web server.
export CONTEXT_DOCUMENT_ROOT=/home/lelanthran/public_html
export CONTENT_TYPE=application/json
export DOCUMENT_ROOT=/var/www/html
export GATEWAY_INTERFACE=CGI/1.1
export HOSTNAME=lelanthran-desktop
export HOSTTYPE=x86_64
export HTTP_ACCEPT='*/*'
export HTTP_HOST=localhost
export HTTP_USER_AGENT=curl/7.58.0
# export PATH_INFO=/one/.two/three
export PATH_TRANSLATED=/var/www/html/one/.two/three
export PWD=/home/lelanthran/public_html/cgi-bin
export QUERY_STRING=
export REMOTE_ADDR=127.0.0.1
export REMOTE_PORT=52096
export REMOTE_USER=lelanthran
export REQUEST_METHOD=GET
export REQUEST_SCHEME=http
export REQUEST_URI=/~lelanthran/cgi-bin/test.sh/one/.two/three
export SCRIPT_FILENAME=/home/lelanthran/public_html/cgi-bin/test.sh
export SCRIPT_NAME=/~lelanthran/cgi-bin/test.sh
export SERVER_ADDR=127.0.0.1
export SERVER_ADMIN=webmaster@localhost
export SERVER_NAME=localhost
export SERVER_PORT=80
export SERVER_PROTOCOL=HTTP/1.1
export SERVER_SIGNATURE=$'<address>Apache/2.4.29 (Ubuntu) Server at localhost Port 80</address>\n'
export SERVER_SOFTWARE='Apache/2.4.29 (Ubuntu)'

export PUBSUB_WORKING_DIR="./"

function display_file () {
   cat $2 | while read LINE; do
      echo $1 $LINE
   done
}

# For each of the endpoints we test we set PATH_INFO and call the cgi
# program
function call_cgi () {
   export PATH_INFO=$1
   export CONTENT_LENGTH=`echo -ne $3 | wc -c`
   echo $3 > tmp.input
   echo "Calling '$PATH_INFO'"
# I uncomment this snippet when I need to debug a particular test.
#  if [ "$2" == "login.results" ]; then
#     gdb pubsub.elf
#     exit 0;
#  fi
   valgrind  --error-exitcode=127 --leak-check=full \
      ./pubsub.elf < tmp.input >$2
   if [ "$?" -ne 0 ]; then
      echo "Error calling '$PATH_INFO', executable returned: "
      display_file "✘" $2
      exit 127
   else
      echo "Success calling '$PATH_INFO', executable returned: "
      display_file "✔" $2
   fi
}

###############################################

call_cgi /login login.results '{
   "email":    "one@example.com",
   "password": "123456"
}'

export HTTP_COOKIE="SessionID=0123456789"
###############################################

call_cgi /user-new user-new.results '{
   "email": "example@email.com",
   "nick": "Example1",
   "password": "12345"
}'

call_cgi /user-new user-new-1.results '{
   "email": "todelete1@email.com",
   "nick": "ToDelete1",
   "password": "12345"
}'


call_cgi /user-new user-new-2.results '{
   "email": "todelete2@email.com",
   "nick": "ToDelete2",
   "password": "12345"
}'

call_cgi /user-new user-new-3.results '{
   "email": "todelete3@email.com",
   "nick": "ToDelete3",
   "password": "12345"
}'

###############################################

call_cgi /user-mod user-mod.results '{
   "old-email":   "todelete2@email.com",
   "new-email":   "todelete4@email.com",
   "nick":        "Nickname to use",
   "password":    "cleartext password"
}'

###############################################

call_cgi /user-list user-list.results '{
   "emailPattern":      "*",
   "nickPattern":       "*",
   "idPattern":         "*"
}'

###############################################

call_cgi /group-new group-new-1.results '{
   "name": "Group1",
}'

call_cgi /group-new group-new-2.results '{
   "name": "Group2",
}'

call_cgi /group-new group-new-3.results '{
   "name": "Group3",
}'

###############################################

call_cgi /group-list group-list-1.results '{
   "pattern":         "*"
}'

###############################################

call_cgi /group-adduser group-adduser-1.results '{
   "name":         "Group1",
   "email":        "todelete1@example.com"
}'

call_cgi /group-adduser group-adduser-2.results '{
   "name":         "Group2",
   "email":        "todelete2@example.com"
}'

call_cgi /group-adduser group-adduser-3.results '{
   "name":         "Group3",
   "email":        "todelete3@example.com"
}'

call_cgi /group-adduser group-adduser-4.results '{
   "name":         "Group1",
   "email":        "todelete2@example.com"
}'

call_cgi /group-adduser group-adduser-5.results '{
   "name":         "Group1",
   "email":        "todelete3@example.com"
}'

###############################################

call_cgi /group-members group-members-1.results '{
   "name":         "Group1",
}'

call_cgi /group-members group-members-2.results '{
   "name":         "Group2",
}'

call_cgi /group-members group-members-3.results '{
   "name":         "Group3",
}'

###############################################

call_cgi /group-rmuser '{
   "name":     "Group1",
   "email":    "todelete2@example.com"
}'

###############################################

call_cgi /group-members '{
   "name":         "Group1",
}'

###############################################

call_cgi /perms-grant-user perms-grant-user-1.results '{
   "email":       "todelete1@example.com",
   "perms":       "put,get,list,del",
   "resource":    "Resource-1"
}'

call_cgi /perms-grant-user perms-grant-user-2.results '{
   "email":       "todelete1@example.com",
   "perms":       "get,list,del",
   "resource":    "Resource-2"
}'

call_cgi /perms-grant-user perms-grant-user-3.results '{
   "email":       "todelete1@example.com",
   "perms":       "put,list,del",
   "resource":    "Resource-3"
}'

call_cgi /perms-grant-user perms-grant-user-4.results '{
   "email":       "todelete2@example.com",
   "perms":       "put,get,del",
   "resource":    "Resource-1"
}'

call_cgi /perms-grant-user perms-grant-user-5.results '{
   "email":       "todelete2@example.com",
   "perms":       "put,get,list",
   "resource":    "Resource-2"
}'

call_cgi /perms-grant-user perms-grant-user-6.results '{
   "email":       "todelete2@example.com",
   "perms":       "put,get,list",
   "resource":    "Resource-3"
}'

call_cgi /perms-grant-user perms-grant-user-7.results '{
   "email":       "todelete3@example.com",
   "perms":       "get,list,del",
   "resource":    "Resource-1"
}'

call_cgi /perms-grant-user perms-grant-user-8.results '{
   "email":       "todelete3@example.com",
   "perms":       "put,list,del",
   "resource":    "Resource-2"
}'

call_cgi /perms-grant-user perms-grant-user-9.results '{
   "email":       "todelete3@example.com",
   "perms":       "put,get,del",
   "resource":    "Resource-3"
}'

###############################################

call_cgi /perms-grant-group perms-grant-group-1.results '{
   "name":        "Group1",
   "perms":       "put,get,list,del",
   "resource":    "Resource-1"
}'

call_cgi /perms-grant-group perms-grant-group-2.results '{
   "name":        "Group1",
   "perms":       "get,list,del",
   "resource":    "Resource-2"
}'

call_cgi /perms-grant-group perms-grant-group-3.results '{
   "name":        "Group1",
   "perms":       "put,list,del",
   "resource":    "Resource-3"
}'

call_cgi /perms-grant-group perms-grant-group-4.results '{
   "name":        "Group2",
   "perms":       "put,get,del",
   "resource":    "Resource-1"
}'

call_cgi /perms-grant-group perms-grant-group-5.results '{
   "name":        "Group2",
   "perms":       "put,get,list",
   "resource":    "Resource-2"
}'

call_cgi /perms-grant-group perms-grant-group-6.results '{
   "name":        "Group2",
   "perms":       "get,list,del",
   "resource":    "Resource-3"
}'

call_cgi /perms-grant-group perms-grant-group-7.results '{
   "name":        "Group3",
   "perms":       "put,list,del",
   "resource":    "Resource-1"
}'

call_cgi /perms-grant-group perms-grant-group-8.results '{
   "name":        "Group3",
   "perms":       "put,get,del",
   "resource":    "Resource-2"
}'

call_cgi /perms-grant-group perms-grant-group-9.results '{
   "name":        "Group3",
   "perms":       "put,get,list",
   "resource":    "Resource-3"
}'

###############################################

call_cgi /perms-resource-user perms-resource-user-1.results '{
   "email":       "todelete1@example.com",
   "resource":    "Resource-1"
}'

call_cgi /perms-resource-user perms-resource-user-2.results '{
   "email":       "todelete1@example.com",
   "resource":    "Resource-2"
}'

call_cgi /perms-resource-user perms-resource-user-3.results '{
   "email":       "todelete1@example.com",
   "resource":    "Resource-3"
}'

call_cgi /perms-resource-user perms-resource-user-4.results '{
   "email":       "todelete2@example.com",
   "resource":    "Resource-1"
}'

call_cgi /perms-resource-user perms-resource-user-5.results '{
   "email":       "todelete2@example.com",
   "resource":    "Resource-2"
}'

call_cgi /perms-resource-user perms-resource-user-6.results '{
   "email":       "todelete2@example.com",
   "resource":    "Resource-3"
}'

call_cgi /perms-resource-user perms-resource-user-7.results '{
   "email":       "todelete3@example.com",
   "resource":    "Resource-1"
}'

call_cgi /perms-resource-user perms-resource-user-8.results '{
   "email":       "todelete3@example.com",
   "resource":    "Resource-2"
}'

call_cgi /perms-resource-user perms-resource-user-9.results '{
   "email":       "todelete3@example.com",
   "resource":    "Resource-3"
}'

###############################################

call_cgi /perms-resource-group perms-resource-group-1.results '{
   "name":        "Group1",
   "resource":    "Resource-1"
}'

call_cgi /perms-resource-group perms-resource-group-2.results '{
   "name":        "Group1",
   "resource":    "Resource-2"
}'

call_cgi /perms-resource-group perms-resource-group-3.results '{
   "name":        "Group1",
   "resource":    "Resource-3"
}'

call_cgi /perms-resource-group perms-resource-group-4.results '{
   "name":        "Group2",
   "resource":    "Resource-1"
}'

call_cgi /perms-resource-group perms-resource-group-5.results '{
   "name":        "Group2",
   "resource":    "Resource-2"
}'

call_cgi /perms-resource-group perms-resource-group-6.results '{
   "name":        "Group2",
   "resource":    "Resource-3"
}'

call_cgi /perms-resource-group perms-resource-group-7.results '{
   "name":        "Group3",
   "resource":    "Resource-1"
}'

call_cgi /perms-resource-group perms-resource-group-8.results '{
   "name":        "Group3",
   "resource":    "Resource-2"
}'

call_cgi /perms-resource-group perms-resource-group-9.results '{
   "name":        "Group3",
   "resource":    "Resource-3"
}'

###############################################

call_cgi /perms-revoke-user perms-revoke-user-1.results '{
   "email":       "todelete1@example.com",
   "perms":       "put",
   "resource":    "Resource-1"
}'

call_cgi /perms-revoke-user perms-revoke-user-2.results '{
   "email":       "todelete2@example.com",
   "perms":       "get",
   "resource":    "Resource-2"
}'

###############################################

call_cgi /perms-resource-user perms-resource-user-1.results '{
   "email":       "todelete1@example.com",
   "resource":    "Resource-1"
}'

call_cgi /perms-resource-user perms-resource-user-2.results '{
   "email":       "todelete2@example.com",
   "resource":    "Resource-2"
}'

###############################################

call_cgi /perms-revoke-group perms-revoke-group-1.results '{
   "name":        "Group1",
   "perms":       "put",
   "resource":    "Resource-1"
}'

call_cgi /perms-revoke-group perms-revoke-group-2.results '{
   "name":        "Group2",
   "perms":       "get",
   "resource":    "Resource-2"
}'

###############################################

call_cgi /perms-resource-group perms-resource-group-1.results '{
   "name":        "Group1",
   "resource":    "Resource-1"
}'

call_cgi /perms-resource-group perms-resource-group-2.results '{
   "name":        "Group2",
   "resource":    "Resource-2"
}'

###############################################
###############################################
###############################################

call_cgi /group-rm group-rm-1.results '{
   "name":    "Group2"
}'

###############################################

call_cgi /user-rm user-rm-1.results '{
   "email":    "ToDelete1"
}'

call_cgi /user-rm user-rm-2.results '{
   "email":    "ToDelete2"
}'

call_cgi /user-rm user-rm-3.results '{
   "email":    "ToDelete3"
}'

###############################################

call_cgi /group-mod group-mod-1.results '{
   "old-name":         "Group1",
   "new-name":         "p1-Renamed"
}'

###############################################

call_cgi /group-list group-list-1.results '{
   "pattern":         "*"
}'

###############################################

call_cgi /user-list user-list-1.results '{
   "emailPattern":      "*",
   "nickPattern":       "*",
   "idPattern":         "*"
}'

###############################################




# export PATH_INFO=/logout
# echo -ne '{ }'




