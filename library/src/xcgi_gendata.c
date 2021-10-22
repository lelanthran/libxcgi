
#include <stdio.h>
#include <stdlib.h>

#include "xcgi.h"

int main (void)
{
   int ret = EXIT_FAILURE;

   if (!(xcgi_init ("./", XCGI_INIT_ALL))) {
      fprintf (stderr, "Failed to initialise the library\n");
      goto errorexit;
   }

   struct {
      const char *name;
      const char **variable;
      const char *value;
   } vars[] = {
      { "content_length",           &xcgi_CONTENT_LENGTH,         NULL},
      { "content_type",             &xcgi_CONTENT_TYPE,           NULL},
      { "context_document_root",    &xcgi_CONTEXT_DOCUMENT_ROOT,  NULL},
      { "context_prefix",           &xcgi_CONTENT_PREFIX,         NULL},
      { "document_root",            &xcgi_DOCUMENT_ROOT,          NULL},
      { "gateway_interface",        &xcgi_GATEWAY_INTERFACE,      NULL},
      { "hostname",                 &xcgi_HOSTNAME,               NULL},
      { "hosttype",                 &xcgi_HOSTTYPE,               NULL},
      { "http_accept",              &xcgi_HTTP_ACCEPT,            NULL},
      { "http_cookie",              &xcgi_HTTP_COOKIE,            NULL},
      { "http_host",                &xcgi_HTTP_HOST,              NULL},
      { "http_referer",             &xcgi_HTTP_REFERER,           NULL},
      { "http_user_agent",          &xcgi_HTTP_USER_AGENT,        NULL},
      { "https",                    &xcgi_HTTPS,                  NULL},
      { "path",                     &xcgi_PATH,                   NULL},
      { "path_info",                  &xcgi_PATH_INFO,              NULL},
      { "pwd",                      &xcgi_PWD,                    NULL},
      { "query_string",             &xcgi_QUERY_STRING,           NULL},
      { "remote_addr",              &xcgi_REMOTE_ADDR,            NULL},
      { "remote_host",              &xcgi_REMOTE_HOST,            NULL},
      { "remote_port",              &xcgi_REMOTE_PORT,            NULL},
      { "remote_user",              &xcgi_REMOTE_USER,            NULL},
      { "request_method",           &xcgi_REQUEST_METHOD,         NULL},
      { "request_scheme",           &xcgi_REQUEST_SCHEME,         NULL},
      { "request_uri",              &xcgi_REQUEST_URI,            NULL},
      { "script_filename",          &xcgi_SCRIPT_FILENAME,        NULL},
      { "script_name",              &xcgi_SCRIPT_NAME,            NULL},
      { "server_addr",              &xcgi_SERVER_ADDR,            NULL},
      { "server_admin",             &xcgi_SERVER_ADMIN,           NULL},
      { "server_name",              &xcgi_SERVER_NAME,            NULL},
      { "server_port",              &xcgi_SERVER_PORT,            NULL},
      { "server_protocol",          &xcgi_SERVER_PROTOCOL,        NULL},
      { "server_signature",         &xcgi_SERVER_SIGNATURE,       NULL},
      { "server_software",          &xcgi_SERVER_SOFTWARE,        NULL},
   };

   static const size_t nvars = sizeof vars / sizeof vars[0];

   printf ("Test: libxcgi v%s\n", xcgi_version);

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

   xcgi_shutdown ();

   return ret;
}

