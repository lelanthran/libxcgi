#!/bin/bash

# The first thing to do is to install the executables for libsqldb into
# your path. When this is done, create and initialise a database for the
# test using the create_testdb.sh script in this directory.



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

echo Removing existing results...
rm *.results*
echo Done.

# For each of the endpoints we test we set PATH_INFO and call the cgi
# program
# function call_cgi () {
#    export PATH_INFO=$1
#    export CONTENT_LENGTH=`echo -ne $3 | wc -c`
#    echo $3 > tmp.input
#    echo "Calling '$PATH_INFO'"
#    if [ -f "$2" ]; then
#       echo "File '$2' already exists - duplicate test?"
#       exit 119
#    fi
# # I uncomment this snippet when I need to debug a particular test.
# #  if [ "$2" == "grant-to-user-1.results" ]; then
# #     cat tmp.input
# #     gdb pubsub.elf
# #     exit 0;
# #  fi
#    if [ -z "$VGOPTS" ]; then
#       ./pubsub.elf < tmp.input >$2
#    else
#       valgrind $VGOPTS --error-exitcode=127 ./pubsub.elf < tmp.input >$2
#    fi
# 
#    if [ "$?" -ne 0 ]; then
#       echo "Error calling '$PATH_INFO', executable returned: "
#       display_file "✘" $2
#       exit 127
#    else
#       echo "Success calling '$PATH_INFO', executable returned: "
#       display_file "✔" $2
#    fi
# }


function call_cgi () {
   echo $3 > tmp.input
   curl -X POST 'http://localhost/~lelanthran/cgi-bin/pubsub.sh'$1 \
      --cookie "session-id=$HTTP_COOKIE" \
      -H "Content-type: application/json" \
      -d @tmp.input\
      -o $2.curl
   ERRCODE=`grep error-code $2.curl | cut -f 2 -d : | cut -f 1 -d ,`
   if [ "$ERRCODE" -ne 0 ]; then
      echo Failed on request \"$1\"
      cat tmp.input
      echo ======================
      display_file "✘" $2.curl
      exit -1
   fi
}

export NAMES='
   one
   two
   three
   four
   five
   six
   seven
   eight
   nine
   ten'

export RMNAMES='
   zero
   two
   three
   four
   five
   seven
   ten'

export LGROUPS='
   Group-One
   Group-Two
   Group-Three
   Group-Four
   Group-Five
   Group-Six
   Group-Seven
   Group-Eight
   Group-Nine
   Group-Ten
'

###############################################

# call_cgi "" null_endpoint.results '{
#     "ignored":  "values",
#     "null":     "endpoint"
# }'

###############################################

call_cgi /login login.results '{
   "email":    "admin@example.com",
   "password": "123456"
}'

export HTTP_COOKIE=`cat login.results | grep Set-Cookie | grep session-id | cut -f 2 -d \  `

if [ -z "$HTTP_COOKIE" ]; then
   export HTTP_COOKIE=`cat login.results.curl | grep session-id | cut -f 4 -d \"  `
fi

echo "Session = [$HTTP_COOKIE]"

###############################################

for X in $NAMES; do
   export EMAIL="m$X@example.com"
   export NICK="User-m$X"
   export OUTFILE="user-new-m$X.results"

   call_cgi /user-new $OUTFILE '{
      "email": "'$EMAIL'",
      "nick": "'$NICK'",
      "password": "123456"
   }'
done

###############################################

for X in $NAMES; do
   export OLDEMAIL="m$X@example.com"
   export NEWEMAIL="$X@example.com"
   export NICK="User-$X"
   export OUTFILE="user-mod-$X.results"

   call_cgi /user-mod $OUTFILE '{
      "old-email":   "'$OLDEMAIL'",
      "new-email":   "'$NEWEMAIL'",
      "nick": "'$NICK'",
      "password": "12345"
   }'
done

###############################################

call_cgi /user-list user-list-1.results '{
   "email-pattern":  "*",
   "nick-pattern":   "*",
   "id-pattern":     "*",
   "resultset-emails":  "true",
   "resultset-nicks":   "true",
   "resultset-flags":   "true",
   "resultset-ids":     "true"
}'

###############################################

for X in $LGROUPS; do
   call_cgi /group-new group-new-$X.results '{
      "group-name": "m'$X'",
      "group-description": "m'$X' description.",
   }'
done

###############################################

for X in $LGROUPS; do
   call_cgi /group-mod group-mod-$X.results '{
      "old-group-name": "m'$X'",
      "new-group-name": "'$X'",
      "group-description": "'$X' description.",
   }'
done

###############################################

call_cgi /group-list group-list-1.results '{
   "name-pattern":               "*-t*",
   "description-pattern":        "",
   "resultset-names":            "true",
   "resultset-descriptions":     "true",
   "resultset-ids":              "true"
}'

###############################################

for X in $LGROUPS; do
   for Y in $NAMES; do
      call_cgi /group-adduser group-adduser-$X-$Y.results '{
         "group-name":   "'$X'",
         "email":        "'$Y'@example.com"
      }'
   done
done

###############################################

for X in $LGROUPS; do
   call_cgi /group-members group-members-1-$X.results '{
      "group-name":         "'$X'",
      "resultset-emails":  "true",
      "resultset-nicks":   "true",
      "resultset-flags":   "true",
      "resultset-ids":     "true"
   }'
done

###############################################

for X in $RMNAMES; do
   call_cgi /group-rmuser group-rmuser-$X.results '{
      "group-name":     "Group-One",
      "email":          "'$X'@example.com",
   }'
done

###############################################

call_cgi /grant-create-to-user grant-create-to-user-1.results '{
   "email": "ten@example.com",
   "perms": "create-user,create-group"
}'

call_cgi /grant-create-to-group grant-create-to-group-1.results '{
   "group-name": "Group-Ten",
   "perms":      "create-user,create-group"
}'

call_cgi /revoke-create-from-user revoke-create-from-user-1.results '{
   "email": "ten@example.com",
   "perms": "create-user"
}'

call_cgi /revoke-create-from-group revoke-create-from-group-1.results '{
   "group-name": "Group-Ten",
   "perms":      "create-group"
}'

###############################################

call_cgi /grant-to-user-over-user grant-to-user-over-user-1.results '{
   "email":       "two@example.com",
   "target-user": "three@example.com",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

call_cgi /grant-to-user-over-group grant-to-user-over-group-1.results '{
   "email":          "three@example.com",
   "target-group":   "Group-One",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

call_cgi /grant-to-group-over-user grant-to-group-over-user-1.results '{
   "group-name":  "Group-Two",
   "target-user": "four@example.com",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

call_cgi /grant-to-group-over-group grant-to-group-over-group-1.results '{
   "group-name":     "Group-Three",
   "target-group":   "Group-Four",
   "perms":          "modify-user,delete-user,list-members,read-user"
}'

##########################

call_cgi /grant-to-user-over-user grant-to-user-over-user-2.results '{
   "email":       "five@example.com",
   "target-user": "six@example.com",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

call_cgi /grant-to-user-over-group grant-to-user-over-group-2.results '{
   "email":          "seven@example.com",
   "target-group":   "Group-Five",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

call_cgi /grant-to-group-over-user grant-to-group-over-user-2.results '{
   "group-name":  "Group-Six",
   "target-user": "eight@example.com",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

call_cgi /grant-to-group-over-group grant-to-group-over-group-2.results '{
   "group-name":     "Group-Seven",
   "target-group":   "Group-Eight",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

##########################

call_cgi /revoke-from-user-over-user revoke-from-user-over-user-1.results '{
   "email":       "two@example.com",
   "target-user": "three@example.com",
   "perms":       "modify-user"
}'

call_cgi /revoke-from-user-over-group revoke-from-user-over-group-1.results '{
   "email":          "three@example.com",
   "target-group":   "Group-One",
   "perms":       "delete-user"
}'

call_cgi /revoke-from-group-over-user revoke-from-group-over-user-1.results '{
   "group-name":  "Group-Two",
   "target-user": "four@example.com",
   "perms":       "list-members"
}'

call_cgi /revoke-from-group-over-group revoke-from-group-over-group-1.results '{
   "group-name":     "Group-Three",
   "target-group":   "Group-Four",
   "perms":          "read-user"
}'

###############################################

call_cgi /perms-create-user perms-create-user-1.results '{
   "email":    "ten@example.com"
}'

call_cgi /perms-create-group perms-create-group-1.results '{
   "group-name":    "Group-Ten"
}'

call_cgi /perms-user-over-user perms-user-over-user-1.results '{
   "email":       "two@example.com",
   "target-user": "three@example.com"
}'

call_cgi /perms-user-over-group perms-user-over-group-1.results '{
   "email":        "three@example.com",
   "target-group": "Group-One"
}'


call_cgi /perms-group-over-user perms-group-over-user-1.results '{
   "group-name":  "Group-Two",
   "target-user": "four@example.com"
}'

call_cgi /perms-group-over-group perms-group-over-group-1.results '{
   "group-name":        "Group-Three",
   "target-group":      "Group-Four"
}'

 ##############

call_cgi /perms-user-over-user perms-user-over-user-2.results '{
   "email":       "five@example.com",
   "target-user": "six@example.com"
}'

call_cgi /perms-user-over-group perms-user-over-group-2.results '{
   "email":        "seven@example.com",
   "target-group": "Group-Five"
}'


call_cgi /perms-group-over-user perms-group-over-user-2.results '{
   "group-name":  "Group-Six",
   "target-user": "eight@example.com"
}'

call_cgi /perms-group-over-group perms-group-over-group-2.results '{
   "group-name":        "Group-Seven",
   "target-group":      "Group-Eight"
}'

###############################################

call_cgi /grant-to-user grant-to-user-1.results '{
   "email":      "four@example.com",
   "resource":   "MyTempResource",
   "perms":      "0,3,4,6,7"
}'

call_cgi /grant-to-user grant-to-user-2.results '{
   "email":      "five@example.com",
   "resource":   "MyTempResource",
   "perms":      "0,3,4,6,7"
}'

call_cgi /grant-to-group grant-to-group-1.results '{
   "group-name":  "Group-Four",
   "resource":    "MyTempResource",
   "perms":       "0,3,4,6,7"
}'

call_cgi /grant-to-group grant-to-group-2.results '{
   "group-name":  "Group-Five",
   "resource":    "MyTempResource",
   "perms":       "0,3,4,6,7"
}'

call_cgi /revoke-from-user revoke-from-user-1.results '{
   "email":       "four@example.com",
   "resource":    "MyTempResource",
   "perms":       "0,3,6,"
}'

call_cgi /revoke-from-group revoke-from-group-1.results '{
   "group-name":  "Group-Four",
   "resource":    "MyTempResource",
   "perms":       "0,3,6,"
}'

call_cgi /perms-for-user perms-for-user-1.results '{
   "email":       "four@example.com",
   "resource":    "MyTempResource",
}'

call_cgi /perms-for-user perms-for-user-2.results '{
   "email":       "five@example.com",
   "resource":    "MyTempResource",
}'

call_cgi /perms-for-group perms-for-group-1.results '{
   "group-name":  "Group-Four",
   "resource":    "MyTempResource",
}'

call_cgi /perms-for-group perms-for-group-2.results '{
   "group-name":  "Group-Five",
   "resource":    "MyTempResource",
}'


###############################################

call_cgi /flags-set flags-set-1.results '{
   "email":      "one@example.com",
   "flags":      "lockout"
}'

call_cgi /user-info user-info-1.results '{
   "email":      "one@example.com",
}'

call_cgi /flags-clear flags-clear-1.results '{
   "email":      "one@example.com",
   "flags":      "lockout"
}'

call_cgi /user-info user-info-2.results '{
   "email":      "one@example.com",
}'

###############################################

call_cgi /grant-to-user-over-user grant-to-user-over-user-3.results '{
   "email":       "nine@example.com",
   "target-user": "ten@example.com",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

call_cgi /grant-to-user-over-group grant-to-user-over-group-3.results '{
   "email":          "nine@example.com",
   "target-group":   "Group-One",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

call_cgi /grant-to-group-over-user grant-to-group-over-user-3.results '{
   "group-name":  "Group-Two",
   "target-user": "four@example.com",
   "perms":       "modify-user,delete-user,list-members,read-user"
}'

call_cgi /grant-to-group-over-group grant-to-group-over-group-3.results '{
   "group-name":     "Group-Two",
   "target-group":   "Group-Four",
   "perms":          "modify-user,delete-user,list-members,read-user"
}'

###############################################

call_cgi /user-rm user-rm-1.results '{
   "email":    "nine@example.com"
}'

call_cgi /group-rm group-rm-1.results '{
   "group-name":    "Group-Two"
}'

echo "Ending test (117)"
exit 117

###############################################


