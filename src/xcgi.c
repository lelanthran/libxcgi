
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
      { "xcgi_document_root",    &xcgi_document_root     },
      { "xcgi_http_cookie",      &xcgi_http_cookie       },
      { "xcgi_http_host",        &xcgi_http_host         },
      { "xcgi_http_referer",     &xcgi_http_referer      },
      { "xcgi_http_user_agent",  &xcgi_http_user_agent   },
      { "xcgi_https",            &xcgi_https             },
      { "xcgi_path",             &xcgi_path              },
      { "xcgi_query_string",     &xcgi_query_string      },
      { "xcgi_remote_addr",      &xcgi_remote_addr       },
      { "xcgi_remote_host",      &xcgi_remote_host       },
      { "xcgi_remote_port",      &xcgi_remote_port       },
      { "xcgi_remote_user",      &xcgi_remote_user       },
      { "xcgi_request_method",   &xcgi_request_method    },
      { "xcgi_request_uri",      &xcgi_request_uri       },
      { "xcgi_script_filename",  &xcgi_script_filename   },
      { "xcgi_script_name",      &xcgi_script_name       },
      { "xcgi_server_admin",     &xcgi_server_admin      },
      { "xcgi_server_name",      &xcgi_server_name       },
      { "xcgi_server_port",      &xcgi_server_port       },
      { "xcgi_server_software",  &xcgi_server_software   },
};

bool xcgi_init (void)
{
   for (size_t i=0; i<sizeof g_vars/sizeof g_vars[0]; i++) {
      char *tmp = getenv (g_vars[i].name);
      *(g_vars[i].variable) = tmp ? tmp : "";
   }
   return true;
}

