
#ifndef H_XCGI
#define H_XCGI

#include <stdbool.h>
#include <stdio.h>

// This is a global non-thread-safe library. A CGI program runs once and
// then exits. Memory used by this module is potentially never freed. The
// caller MUST call xcgi_init() before calling any other function. Before
// program return the caller can call xcgi_shutdown() to ensure that all
// files are closed, although this is not necessary.

#ifdef __cplusplus
extern "C" {
#endif

   // Initialises the cgi variables. Returns true on success, false on error.
   //
   // NOTE: This function does not load the query strings nor does it read
   // any POST data. For that the caller must explicitly call
   // xcgi_qstrings_parse().
   //
   // The reason for this is because the POST data may not be query
   // strings; it might be binary data that is not encoded in a manner
   // compatible with query strings, such as json.
   //
   // If we unconditionally consume all of the stdin input trying to find
   // query strings then we won't later be able to read it when we
   // discover that the POST data was not query strings.
   bool xcgi_init (void);

   // Frees and/or closes all resources allocated or opened during the
   // course of execution of this library.
   void xcgi_shutdown (void);

   // Load/save the cgi environment for later playback.
   bool xcgi_load (const char *fname);
   bool xcgi_save (const char *fname);

   // Return the value of the http variable specified, never NULL. Caller
   // must not free the result.
   const char *xcgi_getenv (const char *name);

   // Returns a copy of the specified string with all the non-ascii
   // characters replaced with their hex equivalent using %xx as the
   // format. The replacement is performed for any characters not in the
   // regex [a-zA-Z0-9$-_.+!*'(),].
   //
   // On success a new string allocated with malloc() is returned. On
   // failure NULL is returned. The caller must free the result.
   char *xcgi_string_escape (const char *src);

   // Returns a copy of the specified string with all the escaped
   // characters replaced with their ascii equivalent using %xx as the
   // escape format. This function undoes the escaping performed by
   // xcgi_string_escape().
   //
   // On success a new string allocated with malloc() is returned. On
   // failure NULL is returned. The caller must free the result.
   char *xcgi_string_unescape (const char *src);

   // Parse the query strings into memory. Returns true on success and
   // false on error (for example, running out of memory).
   //
   // The caller can iterate across the query strings by using
   // xcgi_qstrings_count() to get a count of the query strings and
   // then using xcgi_qstrings() to get a const array of name/value pairs
   // that contain the parsed query strings.
   //
   // The usage of xcgi_qstrings_count is optional as the const array of
   // name/value pairs is terminated with a NULL and can be looped over
   // until the NULL is encountered.
   bool xcgi_qstrings_parse (void);
   size_t xcgi_qstrings_count (void);
   const char ***xcgi_qstrings (void);

#ifdef __cplusplus
};
#endif

// All of these variables are non-NULL after a successful xcgi_init(). The
// caller MUST NOT modify the variables.

extern const char *xcgi_CONTENT_LENGTH;
extern const char *xcgi_CONTENT_TYPE;
extern const char *xcgi_CONTEXT_DOCUMENT_ROOT;
extern const char *xcgi_CONTENT_PREFIX;
extern const char *xcgi_DOCUMENT_ROOT;
extern const char *xcgi_GATEWAY_INTERFACE;
extern const char *xcgi_HOSTNAME;
extern const char *xcgi_HOSTTYPE;
extern const char *xcgi_HTTP_ACCEPT;
extern const char *xcgi_HTTP_COOKIE;
extern const char *xcgi_HTTP_HOST;
extern const char *xcgi_HTTP_REFERER;
extern const char *xcgi_HTTP_USER_AGENT;
extern const char *xcgi_HTTPS;
extern const char *xcgi_PATH;
extern const char *xcgi_PWD;
extern const char *xcgi_QUERY_STRING;
extern const char *xcgi_REMOTE_ADDR;
extern const char *xcgi_REMOTE_HOST;
extern const char *xcgi_REMOTE_PORT;
extern const char *xcgi_REMOTE_USER;
extern const char *xcgi_REQUEST_METHOD;
extern const char *xcgi_REQUEST_SCHEME;
extern const char *xcgi_REQUEST_URI;
extern const char *xcgi_SCRIPT_FILENAME;
extern const char *xcgi_SCRIPT_NAME;
extern const char *xcgi_SERVER_ADDR;
extern const char *xcgi_SERVER_ADMIN;
extern const char *xcgi_SERVER_NAME;
extern const char *xcgi_SERVER_PORT;
extern const char *xcgi_SERVER_PROTOCOL;
extern const char *xcgi_SERVER_SIGNATURE;
extern const char *xcgi_SERVER_SOFTWARE;

extern FILE *xcgi_stdin;

#endif

