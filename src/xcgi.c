
#include <stdlib.h>

#include "xcgi.h"

const char *xcgi_document_root;
const char *xcgi_http_cookie;
const char *xcgi_http_host;
const char *xcgi_http_referer;
const char *xcgi_http_user_agent;
const char *xcgi_https;
const char *xcgi_path;
const char *xcgi_query_string;
const char *xcgi_remote_addr;
const char *xcgi_remote_host;
const char *xcgi_remote_port;
const char *xcgi_remote_user;
const char *xcgi_request_method;
const char *xcgi_request_uri;
const char *xcgi_script_filename;
const char *xcgi_script_name;
const char *xcgi_server_admin;
const char *xcgi_server_name;
const char *xcgi_server_port;
const char *xcgi_server_software;

struct {
   const char *name;
   const char **variable;
} g_vars[] = {
      { "DOCUMENT_ROOT",    &xcgi_document_root     },
      { "HTTP_COOKIE",      &xcgi_http_cookie       },
      { "HTTP_HOST",        &xcgi_http_host         },
      { "HTTP_REFERER",     &xcgi_http_referer      },
      { "HTTP_USER_AGENT",  &xcgi_http_user_agent   },
      { "HTTPS",            &xcgi_https             },
      { "PATH",             &xcgi_path              },
      { "QUERY_STRING",     &xcgi_query_string      },
      { "REMOTE_ADDR",      &xcgi_remote_addr       },
      { "REMOTE_HOST",      &xcgi_remote_host       },
      { "REMOTE_PORT",      &xcgi_remote_port       },
      { "REMOTE_USER",      &xcgi_remote_user       },
      { "REQUEST_METHOD",   &xcgi_request_method    },
      { "REQUEST_URI",      &xcgi_request_uri       },
      { "SCRIPT_FILENAME",  &xcgi_script_filename   },
      { "SCRIPT_NAME",      &xcgi_script_name       },
      { "SERVER_ADMIN",     &xcgi_server_admin      },
      { "SERVER_NAME",      &xcgi_server_name       },
      { "SERVER_PORT",      &xcgi_server_port       },
      { "SERVER_SOFTWARE",  &xcgi_server_software   },
};

bool xcgi_init (void)
{
   for (size_t i=0; i<sizeof g_vars/sizeof g_vars[0]; i++) {
      char *tmp = getenv (g_vars[i].name);
      *(g_vars[i].variable) = tmp ? tmp : "";
   }
   return true;
}

