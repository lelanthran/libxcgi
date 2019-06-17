
#ifndef H_XCGI
#define H_XCGI

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

// Overview
// This is a global non-thread-safe library. A CGI program runs once and
// then exits. Memory used by this module is potentially never freed. The
// caller MUST call xcgi_init() before calling any other function. Before
// program return the caller can call xcgi_shutdown() to ensure that all
// files are closed, although this is not necessary.
//
//
// Single binary/Multiple instances
// The first element in the PATH_INFO is *always* interpreted as an
// application name. For example, calling your xcgi program with
// PATH_INFO=/path1/path2/path3 results in path1 being stored as the
// xcgi_path_id variable, path2 and path3 being stored as elements of the
// xcgi_path_info variable.
//
// The path_id is simply a name that you can use to differentiate multiple
// instances of your script even if you only install a single binary. For
// example, you can use the url "[...]/script/v1/.../.../..." and
// "[...]/script/v2/.../..." to serve different content using the saem
// executable "script".
//
// On startup the xcgi library looks for a file called xcgi_paths.ini in
// its current directory. This file contains name=value pairs mapping each
// path_id to a point on the filesystem:
//    v1=/home/script-user/apps/v1
//    v2=/home/script-user/apps/v2
// After resolving the path_id to a full filesystem path, xcgi switches
// the current working directory to that path and then attempts to load a
// configuration file in that path which also has name=value pairs. The
// contents of this file is up to you.

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
   // Environment functions

   // Load/save the cgi environment for later playback.
   bool xcgi_load (const char *fname);
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
// Note that the first element is reserved (see below, xcgi_path_id) and
// will not appear in this list of paths.
extern const char **xcgi_path_info;

// Available after xcgi_init(). Contains the first path element found in
// PATH_INFO. The first path element is used as an identifier for the
// host's directory which contains the persistent files xcgi uses
// (database, created files/dirs, etc).
//
// This allows the xcgi program to reside in the cgi-bin directory while
// serving content out of multiple different directories.
extern const char *xcgi_path_id;

// Available after xcgi_init(). Contains an array of strings, terminated
// with a NULL, that consists of each of the cookies found in the
// xcgi_HTTP_COOKIE environment variable. Use the function
// xcg_cookies_count() to get the number of cookies found.
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



#endif

