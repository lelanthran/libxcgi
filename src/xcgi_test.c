
#include <stdio.h>
#include <stdlib.h>

#include "xcgi.h"

int main (void)
{
   int ret = EXIT_FAILURE;

   struct {
      const char *name;
      const char *variable;
      const char *value;
   } vars[] = {
      { "xcgi_document_root",     xcgi_document_root,      NULL },
      { "xcgi_http_cookie",       xcgi_http_cookie,        NULL },
      { "xcgi_http_host",         xcgi_http_host,          NULL },
      { "xcgi_http_referer",      xcgi_http_referer,       NULL },
      { "xcgi_http_user_agent",   xcgi_http_user_agent,    NULL },
      { "xcgi_https",             xcgi_https,              NULL },
      { "xcgi_path",              xcgi_path,               NULL },
      { "xcgi_query_string",      xcgi_query_string,       NULL },
      { "xcgi_remote_addr",       xcgi_remote_addr,        NULL },
      { "xcgi_remote_host",       xcgi_remote_host,        NULL },
      { "xcgi_remote_port",       xcgi_remote_port,        NULL },
      { "xcgi_remote_user",       xcgi_remote_user,        NULL },
      { "xcgi_request_method",    xcgi_request_method,     NULL },
      { "xcgi_request_uri",       xcgi_request_uri,        NULL },
      { "xcgi_script_filename",   xcgi_script_filename,    NULL },
      { "xcgi_script_name",       xcgi_script_name,        NULL },
      { "xcgi_server_admin",      xcgi_server_admin,       NULL },
      { "xcgi_server_name",       xcgi_server_name,        NULL },
      { "xcgi_server_port",       xcgi_server_port,        NULL },
      { "xcgi_server_software",   xcgi_server_software,    NULL },
   };

   static const size_t nvars = sizeof vars / sizeof vars[0];

   printf ("Test: libxcgi v%s\n", XCGI_VERSION);

   if (!(xcgi_init ())) {
      fprintf (stderr, "Failed to initialise the library\n");
      goto errorexit;
   }

   for (size_t i=0; i<nvars; i++) {
      fprintf (stderr, "[%25s] [%p] [%s]\n", vars[i].name,
                                             vars[i].variable,
                                             vars[i].value);
   }

   ret = EXIT_SUCCESS;

errorexit:

   return ret;
}

