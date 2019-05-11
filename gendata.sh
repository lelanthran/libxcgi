#!/bin/bash

export stdin_data="\
   01234567890123456789012345678901234567890123456789012345678901\n\
   0         1         2         3         4         5         6\n\
   The quick brown jumped over the lazy dog.         |         |\n\
      The quick brown jumped over the lazy dog.      |         |\n\
         The quick brown jumped over the lazy dog.   |         |\n\
            The quick brown jumped over the lazy dog.          |\n\
               The quick brown jumped over the lazy dog.       |\n\
                  The quick brown jumped over the lazy dog.    |\n\
                     The quick brown jumped over the lazy dog. |\n\
                        The quick brown jumped over the lazy dog.\n"

export CONTENT_LENGTH=652
export CONTENT_TYPE=application/x-www-form-urlencoded
export CONTEXT_DOCUMENT_ROOT=/home/lelanthran/public_html
export CONTEXT_PREFIX=/~lelanthran
export DOCUMENT_ROOT=/var/www/html
export GATEWAY_INTERFACE=CGI/1.1
export HOSTNAME=lelanthran-desktop
export HOSTTYPE=x86_64
export HTTP_ACCEPT='*/*'
export HTTP_HOST=localhost
export HTTP_USER_AGENT=curl/7.58.0
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
export PWD=/home/lelanthran/public_html/cgi-bin
export QUERY_STRING=
export REMOTE_ADDR=127.0.0.1
export REMOTE_PORT=48134
export REQUEST_METHOD=POST
export REQUEST_SCHEME=http
export REQUEST_URI=/~lelanthran/cgi-bin/test.sh
export SCRIPT_FILENAME=/home/lelanthran/public_html/cgi-bin/test.sh
export SCRIPT_NAME=/~lelanthran/cgi-bin/test.sh
export SERVER_ADDR=127.0.0.1
export SERVER_ADMIN=webmaster@localhost
export SERVER_NAME=localhost
export SERVER_PORT=80
export SERVER_PROTOCOL=HTTP/1.1
export SERVER_SIGNATURE=$'<address>Apache/2.4.29 (Ubuntu) Server at localhost Port 80</address>\n'
export SERVER_SOFTWARE='Apache/2.4.29 (Ubuntu)'

echo -ne "$stdin_data" | valgrind --leak-check=yes --show-leak-kinds=all ./xcgi_gendata.elf
