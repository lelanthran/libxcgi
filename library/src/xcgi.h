
#ifndef H_XCGI
#define H_XCGI

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "sqldb.h"

// Overview
// This is a global non-thread-safe library. A CGI program runs once and
// then exits. Memory used by this module is potentially never freed. The
// caller MUST call xcgi_init() before calling any other function. Before
// program return the caller can call xcgi_shutdown() to ensure that all
// files are closed, although this is not necessary.
//
// On startup the xcgi library attempts to switch to a directory specified
// by the caller. Failure to switch directory is fatal. All data storage
// and filesystem access will be relative to this directory.
//
// The 'xcgi.ini' file is a file in the new direcotry containing name=value
// pairs that serve as the configuration for the application. The contents
// of this file is *mostly* up to the caller; some of the modules included
// with libxcgi, such as the database access module, rely on an entry in
// this file to establish a connection to the database.
//
// Failure to open 'xcgi.ini' is not fatal; in this case the configuration
// and database modules used and provided by xcgi will be unavailable.
//
// As a rule, do not attempt to use or reuse any name in the name=value
// record that starts with 'xcgi-'; These are reserved for libxcgi itself.
//
// To read/write name/value pairs, open xcgi_cfg.h, and see the functions:
//    xcgi_cfg_set()
//    xcgi_cfg_get()
//    xcgi_cfg_get_int()
//    xcgi_cfg_get_flt()

#define XCGI_COOKIE_SECURE             (1 << 0)
#define XCGI_COOKIE_HTTPONLY           (1 << 1)
#define XCGI_COOKIE_SAMESITE_STRICT    (1 << 2)
#define XCGI_COOKIE_SAMESITE_LAX       (1 << 3)


#ifdef __cplusplus
extern "C" {
#endif

   //////////////////////////////////////////////////////////////////
   // Initialisation/shutdown functions

   // Initialises the cgi variables. Returns true on success, false on error.
   // The specified path must exist and is used as the working directory:
   // xcgi_init() will switch to the specified directory.
   //
   // See the explanation of the 'xcgi.ini' above.
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
   bool xcgi_init (const char *path);

   // Frees and/or closes all resources allocated or opened during the
   // course of execution of this library.
   void xcgi_shutdown (void);


   //////////////////////////////////////////////////////////////////
   // Environment functions

   // Load/save the cgi environment for later playback.
   bool xcgi_load (const char *path, const char *fname);
   bool xcgi_save (const char *fname);

   // Return the value of the http variable specified, never NULL. Caller
   // must not free the result. Caller may optionally use the variables
   // themselves directly; see the list of xcgi_[A-Z]* variables below.
   const char *xcgi_getenv (const char *name);


   //////////////////////////////////////////////////////////////////
   // String functions

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
   // Query strings functions
   //
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
   // Header functions
   //
   // Each header, with the exception of SetCookie, may have multiple
   // values. The value passed to xcgi_header_value_set() must be complete,
   // together with any of the optional data that the caller wants to
   // store in the header value field (the data which is present after the
   // semicolon in the transmitted response header).
   //
   // SetCookie MUST NOT be set by this function. Use the cookie functions
   // specifically. In general the caller should only use these header
   // functions for setting response headers that cannot be set using a
   // specific response-header function (such as xcgi_header_SetCookie(),
   // or xcgi_header_Accept(), etc.
   //
   // Prior to transmission, headers can be examined by iterating over
   // them using the xcgi_response_headers array. This excludes the
   // headers set by calling specific response-header functions such as
   // xcgi_header_SetCookie(), xcgi_header_Accept(), etc.
   //

   // Append a new value to the named header. If the header does not exist
   // it will be created. Returns true on success and false on failure.
   //
   bool xcgi_headers_value_set (const char *header, const char *value);

   // Clear the entire response header field for the named response header.
   // Does nothing if the header does not exist.
   void xcgi_headers_clear (const char *header);

   // Writes the headers out
   bool xcgi_headers_write (void);

   // ///////////////////////////////////////////////////////////////
   // Set specific headers

   // Set and clear cookies. An expiry of '0' causes a session cookie to
   // be set (no expiry is specified in the response header).
   //
   // Setting a cookie multiple times is allowed - it will be duplicated
   // in the response headers. Clearing a cookie removes all instances of
   // it.
   bool xcgi_header_cookie_set (const char *name, const char *value,
                                time_t    expires,
                                uint32_t  flags);

   void xcgi_header_cookie_clear (const char *name);



   //////////////////////////////////////////////////////////////////
   // Misc functions

   // Returns the number of strings in the xcgi_cookies array.
   size_t xcgi_cookies_count (void);

   // Returns the number of strings in the xcgi_path_info array.
   size_t xcgi_path_info_count (void);

   // Returns the number of response headers ready for transmission
   size_t xcgi_headers_count (void);

   // Returns the reason phrase for the specified http status code. If the
   // code is unknown then the string "Internal Server Error" with the
   // code embedded into it is returned.
   const char *xcgi_reason_phrase (int status_code);

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
// guaranteed to be the the source of POST data.
extern FILE *xcgi_stdin;

// These variables are all available after certain parsing is performed
// and not necessarily after xcgi_init(). An indication of when each
// variable is available is given in the comments.

// Available after xcgi_init(). Contains an array of strings, terminated
// with a NULL, that consists of each of the path elements passed to this
// script via PATH_INFO. Use the function xcgi_path_info_count() to get
// the number of strings in the array for iteration purposes.
//
extern const char **xcgi_path_info;

// Available after xcgi_init(). Contains an array of strings, terminated
// with a NULL, that consists of each of the cookies found in the
// xcgi_HTTP_COOKIE environment variable. Use the function
// xcgi_cookies_count() to get the number of cookies found.
//
// Each cookie is stored as a single string of name=value.
extern const char **xcgi_cookies;

// Available after xcgi_qstrings_parse(). Contains an array of the
// content-types that will be checked to determine if POST data must be
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

// Available after setting any/all response headers. Contains an array of
// strings that each represent a single response header to be transmitted
// verbatim.
extern const char **xcgi_response_headers;


/* The following variables are all non-const and may be modified by the
 * caller as specified.
 */

// Available after xcgi_init(). Must be used to get/set name/value
// pairs used for program configuration. See the functions in xcgi_cfg.h
// for more information.
extern char **xcgi_config;

// Available after xcgi_init(). This is the default database, if it
// exists. To specify the database and the credentials see the explanation
// of the 'xcgi.ini' file at the beginning of this file.
//
// This database handle is intended to be used with all the functions in
// list in xcgi_db.h; the caller *MUST* *NOT* close this handle, it will
// be automatically closed when xcgi_shutdown() is called.
//
// The caller may pass this handle to all of the xcgi_auth module's
// functions.
extern sqldb_t *xcgi_db;

#endif

