
#include <stdio.h>
#include <stdlib.h>

#include "xcgi.h"

#define CT_QSTRING1  ("applicatiON/X-www-form-urlencoded")
#define CT_QSTRING2  ("applicatiON/X-unknown")

int main (void)
{
   int ret = EXIT_FAILURE;
   char *tmp = NULL;

   size_t nqstrings = 0;

   if (!(xcgi_init ())) {
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

   fprintf (stderr, "Test: libxcgi v%s\n", XCGI_VERSION);

   for (size_t i=0; i<nvars; i++) {
      vars[i].value = (*vars[i].variable);
      fprintf (stderr, "[%25s] [%p] [%s]\n", vars[i].name,
                                             *(vars[i].variable),
                                             vars[i].value);
   }

   if (!(xcgi_qstrings_accept_content_type (CT_QSTRING2))) {
      fprintf (stderr, "Failed to add [%s] to content types\n", CT_QSTRING2);
      goto errorexit;
   }

   if (!(xcgi_qstrings_accept_content_type (CT_QSTRING1))) {
      fprintf (stderr, "Failed to add [%s] to content types\n", CT_QSTRING1);
      goto errorexit;
   }

   if (!xcgi_qstrings_content_types) {
      fprintf (stderr, "Failed to set content-types\n");
      goto errorexit;
   }

   for (size_t i=0; xcgi_qstrings_content_types[i]; i++) {
      fprintf (stderr, "Accepting POST content-type: [%s]\n",
               xcgi_qstrings_content_types[i]);
   }

   if (!(xcgi_qstrings_reject_content_type (CT_QSTRING2))) {
      fprintf (stderr, "Failed to reject [%s] content type\n", CT_QSTRING2);
      goto errorexit;
   }

   for (size_t i=0; xcgi_qstrings_content_types[i]; i++) {
      fprintf (stderr, "Final POST content-type: [%s]\n",
               xcgi_qstrings_content_types[i]);
   }

   if (!(xcgi_qstrings_parse ())) {
      fprintf (stderr, "Failed to parse the query strings\n");
      goto errorexit;
   }

   nqstrings = xcgi_qstrings_count ();
   if (!nqstrings) {
      fprintf (stderr, "No query strings, was expecting query strings\n");
      goto errorexit;
   }

   if (!xcgi_qstrings) {
      fprintf (stderr, "Could not retrieve query strings\n");
      goto errorexit;
   }

   for (size_t i=0; xcgi_qstrings[i]; i++) {
      fprintf (stderr, "qs [%s:%s]\n", xcgi_qstrings[i][0], xcgi_qstrings[i][1]);
   }

   fprintf (stderr, "Path-ID [%s]\n", xcgi_path_id);
   for (size_t i=0; xcgi_path_info[i]; i++) {
      fprintf (stderr, "Path[%zu] [%s]\n", i, xcgi_path_info[i]);
   }

   fprintf (stderr, "Path info [%zu]:\n", xcgi_path_info_count ());
   for (size_t i=0; xcgi_path_info[i]; i++) {
      fprintf (stderr, "Path[%zu] [%s]\n", i, xcgi_path_info[i]);
   }
   fprintf (stderr, "/Path info\n");

   if (!(xcgi_headers_value_set ("HEADer-1", "My Value; With options"))) {
      fprintf (stderr, "Failed to set header 1\n");
      goto errorexit;
   }

   if (!(xcgi_headers_value_set ("HeADer-1", "New Value; With new options"))) {
      fprintf (stderr, "Failed to set header 2\n");
      goto errorexit;
   }

   if (!(xcgi_headers_value_set ("hEADer-2", "Value"))) {
      fprintf (stderr, "Failed to set header 3\n");
      goto errorexit;
   }

   fprintf (stderr, "Headers [%zu]:\n", xcgi_headers_count ());
   for (size_t i=0; xcgi_response_headers[i]; i++) {
      fprintf (stderr, "   [%s]\n", xcgi_response_headers[i]);
   }
   fprintf (stderr, "/Headers\n");

   xcgi_headers_clear ("Header-2");
   fprintf (stderr, "Headers [%zu]:\n", xcgi_headers_count ());
   for (size_t i=0; xcgi_response_headers[i]; i++) {
      fprintf (stderr, "   [%s]\n", xcgi_response_headers[i]);
   }
   fprintf (stderr, "/Headers\n");

   xcgi_headers_write ();

   printf ("--");
   while (!feof (xcgi_stdin) && !ferror (xcgi_stdin)) {
      int c = fgetc (xcgi_stdin);
      if (c!=EOF) {
         fputc (c, stdout);
      }
   }
   fprintf (stderr, "--\n");

   tmp = get_current_dir_name ();
   fprintf (stderr, "[%s] [%s]\n", getenv ("PWD"), tmp);

   ret = EXIT_SUCCESS;

errorexit:

   free (tmp);
   xcgi_shutdown ();

   return ret;
}

