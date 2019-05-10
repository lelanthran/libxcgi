
#ifndef H_XCGI
#define H_XCGI

#include <stdbool.h>

// This is a global non-thread-safe file. A CGI program runs once, then
// exits.

#ifdef __cplusplus
extern "C" {
#endif

   // Returns false on error.
   bool xcgi_init (void);

#ifdef __cplusplus
};
#endif

// All of these variables are non-NULL after a successful xcgi_init(). The
// caller MUST NOT modify the variables.

extern const char *xcgi_document_root;
extern const char *xcgi_http_cookie;
extern const char *xcgi_http_host;
extern const char *xcgi_http_referer;
extern const char *xcgi_http_user_agent;
extern const char *xcgi_https;
extern const char *xcgi_path;
extern const char *xcgi_query_string;
extern const char *xcgi_remote_addr;
extern const char *xcgi_remote_host;
extern const char *xcgi_remote_port;
extern const char *xcgi_remote_user;
extern const char *xcgi_request_method;
extern const char *xcgi_request_uri;
extern const char *xcgi_script_filename;
extern const char *xcgi_script_name;
extern const char *xcgi_server_admin;
extern const char *xcgi_server_name;
extern const char *xcgi_server_port;
extern const char *xcgi_server_software;

#endif

