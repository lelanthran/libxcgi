
#ifndef H_XCGI
#define H_XCGI

#include <stdbool.h>

// This is a global non-thread-safe file. A CGI program runs once, then
// exits. Memory used by this module is never freed.

#ifdef __cplusplus
extern "C" {
#endif

   // Initialises the cgi variables. Returns true on success, false on error.
   bool xcgi_init (void);

   // Load/save the cgi environment for later playback.
   bool xcgi_load (const char *fname);
   bool xcgi_save (const char *fname);


#ifdef __cplusplus
};
#endif

// All of these variables are non-NULL after a successful xcgi_init(). The
// caller MUST NOT modify the variables.

extern const char *xcgi_content_length;
extern const char *xcgi_content_type;
extern const char *xcgi_context_document_root;
extern const char *xcgi_content_prefix;
extern const char *xcgi_document_root;
extern const char *xcgi_gateway_interface;
extern const char *xcgi_hostname;
extern const char *xcgi_hosttype;
extern const char *xcgi_http_accept;
extern const char *xcgi_http_cookie;
extern const char *xcgi_http_host;
extern const char *xcgi_http_referer;
extern const char *xcgi_http_user_agent;
extern const char *xcgi_https;
extern const char *xcgi_path;
extern const char *xcgi_pwd;
extern const char *xcgi_query_string;
extern const char *xcgi_remote_addr;
extern const char *xcgi_remote_host;
extern const char *xcgi_remote_port;
extern const char *xcgi_remote_user;
extern const char *xcgi_request_method;
extern const char *xcgi_request_scheme;
extern const char *xcgi_request_uri;
extern const char *xcgi_script_filename;
extern const char *xcgi_script_name;
extern const char *xcgi_server_addr;
extern const char *xcgi_server_admin;
extern const char *xcgi_server_name;
extern const char *xcgi_server_port;
extern const char *xcgi_server_protocol;
extern const char *xcgi_server_signature;
extern const char *xcgi_server_software;

#endif

