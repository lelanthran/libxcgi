#!/bin/bash

# Exit on first error
set -e

# These variables cause the cgi program to think that it has been invoked
# by a web server.
export CONTEXT_DOCUMENT_ROOT=/home/lelanthran/public_html
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


# For each of the endpoints we test we set PATH_INFO and call the cgi
# program
export PATH_INFO=/login
echo -ne '{"email": "example@email.com" }' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/user-new
echo -ne '{
   "email": "example@email.com",
   "nick": "Example1",
   "password": "12345"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email": "todelete1@email.com",
   "nick": "ToDelete1",
   "password": "12345"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email": "todelete2@email.com",
   "nick": "ToDelete2",
   "password": "12345"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email": "todelete3@email.com",
   "nick": "ToDelete3",
   "password": "12345"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/user-mod
echo -ne '{
   "old-email":   "todelete2@email.com",
   "new-email":   "todelete4@email.com",
   "nick":        "Nickname to use",
   "password":    "cleartext password"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/user-list
echo -ne '{
   "emailPattern":      "*",
   "nickPattern":       "*",
   "idPattern":         "*"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/group-new
echo -ne '{
   "name": "Group1",
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name": "Group2",
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name": "Group3",
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/group-list
echo -ne '{
   "pattern":         "*"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/group-adduser
echo -ne '{
   "name":         "Group1",
   "email":        "todelete1@example.com"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":         "Group2",
   "email":        "todelete2@example.com"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":         "Group3",
   "email":        "todelete3@example.com"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":         "Group1",
   "email":        "todelete2@example.com"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":         "Group1",
   "email":        "todelete3@example.com"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/group-members
echo -ne '{
   "name":         "Group1",
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":         "Group2",
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":         "Group3",
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/group-rmuser
echo -ne '{
   "name":     "Group1",
   "email":    "todelete2@example.com"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/group-members
echo -ne '{
   "name":         "Group1",
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/perms-grant-user
echo -ne '{
   "email":       "todelete1@example.com",
   "perms":       "put,get,list,del",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete1@example.com",
   "perms":       "get,list,del",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete1@example.com",
   "perms":       "put,list,del",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete2@example.com",
   "perms":       "put,get,del",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete2@example.com",
   "perms":       "put,get,list",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete2@example.com",
   "perms":       "put,get,list",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete3@example.com",
   "perms":       "get,list,del",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete3@example.com",
   "perms":       "put,list,del",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete3@example.com",
   "perms":       "put,get,del",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/perms-grant-group
echo -ne '{
   "name":        "Group1",
   "perms":       "put,get,list,del",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group1",
   "perms":       "get,list,del",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group1",
   "perms":       "put,list,del",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group2",
   "perms":       "put,get,del",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group2",
   "perms":       "put,get,list",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group2",
   "perms":       "get,list,del",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group3",
   "perms":       "put,list,del",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group3",
   "perms":       "put,get,del",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group3",
   "perms":       "put,get,list",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/perms-resource-user
echo -ne '{
   "email":       "todelete1@example.com",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete1@example.com",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete1@example.com",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete2@example.com",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete2@example.com",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete2@example.com",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete3@example.com",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete3@example.com",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete3@example.com",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/perms-resource-group
echo -ne '{
   "name":        "Group1",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group1",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group1",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group2",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group2",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group2",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group3",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group3",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group3",
   "resource":    "Resource-3"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/perms-revoke-user
echo -ne '{
   "email":       "todelete1@example.com",
   "perms":       "put",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete2@example.com",
   "perms":       "get",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/perms-resource-user
echo -ne '{
   "email":       "todelete1@example.com",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":       "todelete2@example.com",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/perms-revoke-group
echo -ne '{
   "name":        "Group1",
   "perms":       "put",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group2",
   "perms":       "get",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/perms-resource-group
echo -ne '{
   "name":        "Group1",
   "resource":    "Resource-1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "name":        "Group2",
   "resource":    "Resource-2"
}' | valgrind ./pubsub.elf > results

###############################################
###############################################
###############################################

export PATH_INFO=/group-rm
echo -ne '{
   "name":    "Group2"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/user-rm
echo -ne '{
   "email":    "ToDelete1"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":    "ToDelete2"
}' | valgrind ./pubsub.elf > results

echo -ne '{
   "email":    "ToDelete3"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/group-list
echo -ne '{
   "pattern":         "*"
}' | valgrind ./pubsub.elf > results

###############################################

export PATH_INFO=/user-list
echo -ne '{
   "emailPattern":      "*",
   "nickPattern":       "*",
   "idPattern":         "*"
}' | valgrind ./pubsub.elf > results

###############################################




# export PATH_INFO=/logout
# echo -ne '{ }' | valgrind ./pubsub.elf > results




