
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

   //////////////////////////////////////////////////////////////////


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


   //////////////////////////////////////////////////////////////////


   // Load/save the cgi environment for later playback.
   bool xcgi_load (const char *fname);
   bool xcgi_save (const char *fname);

   // Return the value of the http variable specified, never NULL. Caller
   // must not free the result. Caller may optionally use the variables
   // themselves directly; see the list of xcgi_[A-Z]* variables below.
   const char *xcgi_getenv (const char *name);


   //////////////////////////////////////////////////////////////////


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


   //////////////////////////////////////////////////////////////////


   // When parsing query strings, the library will refuse to parse POST
   // request query strings that do not match an acceptable content-type.
   // These two functions add and remove acceptable content-type
   // specifications from the whitelist used by the parser.
   //
   // The list is initially empty, so all POST query strings will be
   // ignored.
   //
   // The function xcgi_qstrings_accept_content_type() adds a content-type
   // specification to the list. This function can be called multiple
   // times, even with the same content-type, with no ill-effect. This
   // function must be called at least once for each content-type that
   // can be parsed as a POST request query string.
   //
   // The function xcgi_qstrings_reject_content_type() removes content type
   // specifications from the list of acceptable content types for query
   // strings. This function can be called multiple times, even with the
   // same content-type, with no ill effects.
   //
   // On success true is returned. On error false is returned. Adding
   // duplicated content-types is not an error (returns true) and removing
   // non-existing content-types is not an error (returns true).
   //
   // NOTE: The list of acceptable content-types is ignored when parsing
   // query strings that are passed in the URI.
   bool xcgi_qstrings_accept_content_type (const char *content_type);
   bool xcgi_qstrings_reject_content_type (const char *content_type);

   // Parse the query strings into memory. Returns true on success and
   // false on error (for example, running out of memory). Both GET and
   // POST query strings are checked, with an added check on the
   // CONTENT_TYPE for ensuring that POST query strings are of the correct
   // content-type.
   //
   // Set acceptable query strings content-types using
   // xcgi_qstrings_accept_content_type().
   //
   // The caller can iterate across the query strings by using
   // xcgi_qstrings_count() to get a count of the query strings and
   // then using the xcgi_qstrings const array of name/value pairs
   // to access the names and values.
   //
   // The usage of xcgi_qstrings_count is optional as the const array of
   // name/value pairs is terminated with a NULL and can be looped over
   // until the NULL is encountered.
   bool xcgi_qstrings_parse (void);

   // Return the number of query strings found. Must be called only after
   // a successful call to xcgi_qstrings_parse().
   size_t xcgi_qstrings_count (void);


   //////////////////////////////////////////////////////////////////

   // Returns the number of strings in the xcgi_path_info array.
   size_t xcgi_path_info_count (void);

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
extern const char *xcgi_PATH_INFO;
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

// All output must be read from this stream, because stdin is not
// guaranteed to the the source of POST data.
extern FILE *xcgi_stdin;

// These variables are all available after certain parsing is performed
// and not necessarily after xcgi_init(). An indication of when each
// variable is available is given in the comments.

// Available after xcgi_init(). Contains an array of strings, terminated
// with a NULL, that consists of each of the path elements passed to this
// script via PATH_INFO. Use the function xcgi_path_info_count() to get
// the number of strings in the array for iteration purposes.
extern const char **xcgi_path_info;

// Available after xcgi_qstrings_parse(). Contains an array of the
// content-types that will be chekced to determine if POST data must be
// passed as name=value query strings. See the functions:
//    xcgi_qstrings_accept_content_type() => content-type to accept
//    xcgi_qstrings_reject_content_type() => content-type to reject
// The caller usually does not need to read this variable; the caller must
// set the acceptable content-types using the two functions above.
extern const char **xcgi_qstrings_content_types;

// Available after xcgi_qstrings_parse(). Contains an array of name=value
// pairs for all the query strings passed to this script. This includes the
// POST data if content-type matches a content-type specified in the list
// of acceptable content_types `xcgi_qstrings_content_types`.
extern const char ***xcgi_qstrings;

#endif

