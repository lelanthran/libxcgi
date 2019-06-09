#!/bin/bash

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

