
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xcgi.h"

const char *xcgi_content_length;
const char *xcgi_content_type;
const char *xcgi_context_document_root;
const char *xcgi_content_prefix;
const char *xcgi_document_root;
const char *xcgi_gateway_interface;
const char *xcgi_hostname;
const char *xcgi_hosttype;
const char *xcgi_http_accept;
const char *xcgi_http_host;
const char *xcgi_http_user_agent;
const char *xcgi_document_root;
const char *xcgi_http_cookie;
const char *xcgi_http_host;
const char *xcgi_http_referer;
const char *xcgi_http_user_agent;
const char *xcgi_https;
const char *xcgi_path;
const char *xcgi_pwd;
const char *xcgi_query_string;
const char *xcgi_remote_addr;
const char *xcgi_remote_host;
const char *xcgi_remote_port;
const char *xcgi_remote_user;
const char *xcgi_request_method;
const char *xcgi_request_scheme;
const char *xcgi_request_uri;
const char *xcgi_script_filename;
const char *xcgi_script_name;
const char *xcgi_server_addr;
const char *xcgi_server_admin;
const char *xcgi_server_name;
const char *xcgi_server_port;
const char *xcgi_server_protocol;
const char *xcgi_server_signature;
const char *xcgi_server_software;


struct {
   const char *name;
   const char **variable;
} g_vars[] = {
      { "CONTENT_LENGTH",           &xcgi_content_length          },
      { "CONTENT_TYPE",             &xcgi_content_type            },
      { "CONTEXT_DOCUMENT_ROOT",    &xcgi_context_document_root   },
      { "CONTEXT_PREFIX",           &xcgi_content_prefix          },
      { "DOCUMENT_ROOT",            &xcgi_document_root           },
      { "GATEWAY_INTERFACE",        &xcgi_gateway_interface       },
      { "HOSTNAME",                 &xcgi_hostname                },
      { "HOSTTYPE",                 &xcgi_hosttype                },
      { "HTTP_ACCEPT",              &xcgi_http_accept             },
      { "HTTP_HOST",                &xcgi_http_host               },
      { "HTTP_USER_AGENT",          &xcgi_http_user_agent         },
      { "DOCUMENT_ROOT",            &xcgi_document_root           },
      { "HTTP_COOKIE",              &xcgi_http_cookie             },
      { "HTTP_HOST",                &xcgi_http_host               },
      { "HTTP_REFERER",             &xcgi_http_referer            },
      { "HTTP_USER_AGENT",          &xcgi_http_user_agent         },
      { "HTTPS",                    &xcgi_https                   },
      { "PATH",                     &xcgi_path                    },
      { "PWD",                      &xcgi_pwd                     },
      { "QUERY_STRING",             &xcgi_query_string            },
      { "REMOTE_ADDR",              &xcgi_remote_addr             },
      { "REMOTE_HOST",              &xcgi_remote_host             },
      { "REMOTE_PORT",              &xcgi_remote_port             },
      { "REMOTE_USER",              &xcgi_remote_user             },
      { "REQUEST_METHOD",           &xcgi_request_method          },
      { "REQUEST_SCHEME",           &xcgi_request_scheme          },
      { "REQUEST_URI",              &xcgi_request_uri             },
      { "SCRIPT_FILENAME",          &xcgi_script_filename         },
      { "SCRIPT_NAME",              &xcgi_script_name             },
      { "SERVER_ADDR",              &xcgi_server_addr             },
      { "SERVER_ADMIN",             &xcgi_server_admin            },
      { "SERVER_NAME",              &xcgi_server_name             },
      { "SERVER_PORT",              &xcgi_server_port             },
      { "SERVER_PROTOCOL",          &xcgi_server_protocol         },
      { "SERVER_SIGNATURE",         &xcgi_server_signature        },
      { "SERVER_SOFTWARE",          &xcgi_server_software         },
};

bool xcgi_init (void)
{
   for (size_t i=0; i<sizeof g_vars/sizeof g_vars[0]; i++) {
      char *tmp = getenv (g_vars[i].name);
      *(g_vars[i].variable) = tmp ? tmp : "";
   }
   return true;
}

#define MARKER_EOV      ("MARKER-END-OF-VARS")

bool xcgi_save (const char *fname)
{
   bool error = true;
   FILE *outf = NULL;

   if (!(outf = fopen (fname, "w"))) {
      fprintf (stderr, "Failed to open [%s] for writing: %m\n", fname);
      goto errorexit;
   }

   for (size_t i=0; i<sizeof g_vars/sizeof g_vars[0]; i++) {
      fprintf (outf, "%s\x01%s\n", g_vars[i].name, *(g_vars[i].variable));
   }
   fprintf (outf, "%s", MARKER_EOV);
   // TODO: Save stdin?

   error = false;

errorexit:
   if (outf) {
      fclose (outf);
   }

   return !error;
}

bool xcgi_load (const char *fname)
{
   bool error = true;
   FILE *inf = NULL;
   static char line[1024  * 16];
   size_t nlines = 0;

   if (!(inf = fopen (fname, "r"))) {
      fprintf (stderr, "Failed to open [%s] for reading: %m\n", fname);
      goto errorexit;
   }

   while (!(feof (inf) && !ferror (inf))) {
      char *ltmp = fgets (line, sizeof line - 1, inf);
      if (!ltmp) {
         break;
      }

      nlines++;
      line[sizeof line - 1] = 0;
      if ((memcmp (line, MARKER_EOV, strlen (MARKER_EOV)+1))==0)
         break;

      char *tmp = strchr (line, '\n');
      if (tmp)
         *tmp = 0;

      tmp = strchr (line, 0x01);
      if (!tmp) {
         fprintf (stderr, "%zu Failed to parse variable line [%s]. Aborting\n",
                           nlines, line);
         goto errorexit;
      }
      *tmp++ = 0;
      // TODO: For Windows must use putenv_s()
      setenv (line, tmp, 1);
   }
   // TODO: Read stdin?

   error = false;

errorexit:
   if (inf) {
      fclose (inf);
   }

   return !error;
}

