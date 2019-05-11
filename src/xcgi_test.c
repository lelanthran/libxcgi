
#include <stdio.h>
#include <stdlib.h>

#include "xcgi.h"

int main (void)
{
   int ret = EXIT_FAILURE;

   if (!(xcgi_init ())) {
      fprintf (stderr, "Failed to initialise the library\n");
      goto errorexit;
   }

   struct {
      const char *name;
      const char **variable;
      const char *value;
   } vars[] = {
      { "content_length",           &xcgi_content_length,         NULL},
      { "content_type",             &xcgi_content_type,           NULL},
      { "context_document_root",    &xcgi_context_document_root,  NULL},
      { "context_prefix",           &xcgi_content_prefix,         NULL},
      { "document_root",            &xcgi_document_root,          NULL},
      { "gateway_interface",        &xcgi_gateway_interface,      NULL},
      { "hostname",                 &xcgi_hostname,               NULL},
      { "hosttype",                 &xcgi_hosttype,               NULL},
      { "http_accept",              &xcgi_http_accept,            NULL},
      { "http_cookie",              &xcgi_http_cookie,            NULL},
      { "http_host",                &xcgi_http_host,              NULL},
      { "http_referer",             &xcgi_http_referer,           NULL},
      { "http_user_agent",          &xcgi_http_user_agent,        NULL},
      { "https",                    &xcgi_https,                  NULL},
      { "path",                     &xcgi_path,                   NULL},
      { "pwd",                      &xcgi_pwd,                    NULL},
      { "query_string",             &xcgi_query_string,           NULL},
      { "remote_addr",              &xcgi_remote_addr,            NULL},
      { "remote_host",              &xcgi_remote_host,            NULL},
      { "remote_port",              &xcgi_remote_port,            NULL},
      { "remote_user",              &xcgi_remote_user,            NULL},
      { "request_method",           &xcgi_request_method,         NULL},
      { "request_scheme",           &xcgi_request_scheme,         NULL},
      { "request_uri",              &xcgi_request_uri,            NULL},
      { "script_filename",          &xcgi_script_filename,        NULL},
      { "script_name",              &xcgi_script_name,            NULL},
      { "server_addr",              &xcgi_server_addr,            NULL},
      { "server_admin",             &xcgi_server_admin,           NULL},
      { "server_name",              &xcgi_server_name,            NULL},
      { "server_port",              &xcgi_server_port,            NULL},
      { "server_protocol",          &xcgi_server_protocol,        NULL},
      { "server_signature",         &xcgi_server_signature,       NULL},
      { "server_software",          &xcgi_server_software,        NULL},
   };

   static const size_t nvars = sizeof vars / sizeof vars[0];

   printf ("Test: libxcgi v%s\n", XCGI_VERSION);

   for (size_t i=0; i<nvars; i++) {
      vars[i].value = (*vars[i].variable);
      fprintf (stderr, "[%25s] [%p] [%s]\n", vars[i].name,
                                             *(vars[i].variable),
                                             vars[i].value);
   }

   if (!(xcgi_save ("test-vars.dat"))) {
      fprintf (stderr, "Failed to save the vars to file\n");
      goto errorexit;
   }

   ret = EXIT_SUCCESS;

errorexit:

   return ret;
}

