
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "xcgi.h"

#include "ds_array.h"
#include "ds_str.h"

const char *xcgi_CONTENT_LENGTH;
const char *xcgi_CONTENT_TYPE;
const char *xcgi_CONTEXT_DOCUMENT_ROOT;
const char *xcgi_CONTENT_PREFIX;
const char *xcgi_DOCUMENT_ROOT;
const char *xcgi_GATEWAY_INTERFACE;
const char *xcgi_HOSTNAME;
const char *xcgi_HOSTTYPE;
const char *xcgi_HTTP_ACCEPT;
const char *xcgi_HTTP_COOKIE;
const char *xcgi_HTTP_HOST;
const char *xcgi_HTTP_REFERER;
const char *xcgi_HTTP_USER_AGENT;
const char *xcgi_HTTPS;
const char *xcgi_PATH;
const char *xcgi_PATH_INFO;
const char *xcgi_PWD;
const char *xcgi_QUERY_STRING;
const char *xcgi_REMOTE_ADDR;
const char *xcgi_REMOTE_HOST;
const char *xcgi_REMOTE_PORT;
const char *xcgi_REMOTE_USER;
const char *xcgi_REQUEST_METHOD;
const char *xcgi_REQUEST_SCHEME;
const char *xcgi_REQUEST_URI;
const char *xcgi_SCRIPT_FILENAME;
const char *xcgi_SCRIPT_NAME;
const char *xcgi_SERVER_ADDR;
const char *xcgi_SERVER_ADMIN;
const char *xcgi_SERVER_NAME;
const char *xcgi_SERVER_PORT;
const char *xcgi_SERVER_PROTOCOL;
const char *xcgi_SERVER_SIGNATURE;
const char *xcgi_SERVER_SOFTWARE;

FILE *xcgi_stdin;

const char **xcgi_path_info;
const char **xcgi_qstrings_content_types;
const char ***xcgi_qstrings;
const char **xcgi_response_headers;

/* ************************************************************************
 * All the cookie-related storage. This is all private to this module.
 */
typedef struct cookie_t cookie_t;
struct cookie_t {
   char       *name;
   char       *value;
   time_t      expires;
   uint32_t    flags;
};

static cookie_t **xcgi_cookielist;

static bool cookielist_init (void)
{
   return (xcgi_cookielist = (cookie_t **)ds_array_new ()) ? true : false;
}

static const char *cookie_time (time_t expires)
{
   static char ret[30];

   ret[0] = ';';
   ret[1] = ' ';
   strcpy (&ret[2], ctime (&expires));
   char *tmp = strchr (ret, '\n');
   if (tmp)
      *tmp = 0;

   return ret;
}

static const char *cookie_samesite (uint32_t flags)
{
   static char ret[20];
   memset (ret, 0, sizeof ret);
   if (flags & XCGI_COOKIE_SAMESITE_LAX) {
      strcpy (ret, "; SameSite=Lax");
   }
   if (flags & XCGI_COOKIE_SAMESITE_STRICT) {
      strcpy (ret, "; SameSite=Strict");
   }
   return ret;
}

static bool cookielist_write (void)
{
   for (size_t i=0; xcgi_cookielist[i]; i++) {
      cookie_t *cookie = xcgi_cookielist[i];
      fprintf (stdout, "Set-Cookie: %s=%s%s%s%s%s\r\n",
               cookie->name,
               cookie->value,
               cookie->expires ? cookie_time (cookie->expires) : "",
               cookie->flags & XCGI_COOKIE_SECURE ? "; Secure" : "",
               cookie->flags & XCGI_COOKIE_HTTPONLY ? "; HttpOnly" : "",
               cookie_samesite (cookie->flags));
   }
   return true;
}

static void cookie_del (cookie_t *cookie)
{
   if (!cookie)
      return;
   free (cookie->name);
   free (cookie->value);
   free (cookie);
}

static cookie_t *cookie_new (const char *name, const char *value,
                             time_t expires, uint32_t flags)
{
   bool error = true;
   cookie_t *ret = NULL;

   if (!name || !value)
      goto errorexit;

   if (!(ret = malloc (sizeof *ret)))
      goto errorexit;

   memset (ret, 0, sizeof *ret);

   name = ds_str_dup (name);
   value = ds_str_dup (value);
   ret->expires = expires;
   ret->flags = flags;

   if (!ret->name || !ret->value)
      goto errorexit;

   error = false;

errorexit:
   if (error) {
      cookie_del (ret);
      ret = NULL;
   }

   return ret;
}
static void cookielist_shutdown (void)
{
   for (size_t i=0; xcgi_cookielist[i]; i++) {
      cookie_del (xcgi_cookielist[i]);
   }
   ds_array_del ((void **)xcgi_cookielist);
   xcgi_cookielist = NULL;
}

bool xcgi_header_cookie_set (const char *name, const char *value,
                             time_t    expires,
                             uint32_t  flags)
{
   bool error = true;
   cookie_t *newcookie = NULL;

   if (!(newcookie = cookie_new (name, value, expires, flags)))
      goto errorexit;

   if (!(ds_array_ins_tail ((void ***)&xcgi_cookielist, newcookie)))
      goto errorexit;

   error = false;

errorexit:
   if (error) {
      cookie_del (newcookie);
   }
   return !error;
}

void xcgi_header_cookie_clear (const char *name)
{
   for (size_t i=0; xcgi_cookielist[i]; i++) {
      cookie_t *cookie = xcgi_cookielist[i];
      if ((strcmp (cookie->name, name))==0) {
         cookie_del (cookie);
         ds_array_remove ((void ***)&xcgi_cookielist, i);
      }
   }
}


/* ************************************************************************
 */
static bool qs_content_types_init (void)
{
   return (xcgi_qstrings_content_types = (const char **)ds_array_new ())
            ? true : false;
}

static void qs_content_types_shutdown (void)
{
   if (!xcgi_qstrings_content_types)
      return;

   for (size_t i=0; xcgi_qstrings_content_types[i]; i++) {
      free ((char *)xcgi_qstrings_content_types[i]);
   }
   ds_array_del ((void **)xcgi_qstrings_content_types);
   xcgi_qstrings_content_types = NULL;
}

static char *qs_content_types_add (const char *ct)
{
   bool error = true;
   char *ret = NULL;
   size_t len = 0;

   if (!ct)
      return NULL;

   len = strlen (ct) + 1;

   if (!(ret = malloc (len)))
      goto errorexit;

   for (size_t i=0; ct[i]; i++)
      ret[i] = toupper (ct[i]);

   ret[len-1] = 0;

   if (!(ds_array_ins_tail ((void ***)&xcgi_qstrings_content_types, ret)))
      goto errorexit;

   error = false;

errorexit:
   if (error) {
      free (ret);
      ret = NULL;
   }

   return ret;
}

static bool qs_content_types_remove (const char *ct)
{
   if (!xcgi_qstrings_content_types)
      return true;

   bool found = false;
   char *tmp = ds_str_dup (ct);
   if (!tmp)
      return false;

   for (size_t i=0; tmp[i]; i++)
      tmp[i] = toupper (tmp[i]);


   for (size_t i=0; xcgi_qstrings_content_types[i]; i++) {
      if ((strcmp (tmp, xcgi_qstrings_content_types[i]))==0) {
         char *old = ds_array_remove ((void ***)&xcgi_qstrings_content_types, i);
         free (old);
         found = true;
         // DO NOT BREAK HERE! We want to remove duplicates as well.
      }
   }

   free (tmp);
   return found;
}

static bool qs_content_types_check (const char *ct)
{
   if (!xcgi_qstrings_content_types)
      return false;

   char *tmp = ds_str_dup (ct);
   if (!tmp)
      return false;

   for (size_t i=0; tmp[i]; i++)
      tmp[i] = toupper (tmp[i]);

   for (size_t i=0; xcgi_qstrings_content_types[i]; i++) {
      if ((strcmp (tmp, xcgi_qstrings_content_types[i]))==0) {
         free (tmp);
         return true;
      }
   }

   free (tmp);

   return false;
}


/* ************************************************************************
 */
static bool qstrings_init (void)
{
   return (xcgi_qstrings = (const char ***)ds_array_new ()) ? true : false;
}

static void qstrings_shutdown (void)
{
   for (size_t i=0; xcgi_qstrings && xcgi_qstrings[i]; i++) {
      free ((void *)xcgi_qstrings[i][0]);
      free ((void *)xcgi_qstrings[i][1]);
      free ((void *)xcgi_qstrings[i]);
   }
   ds_array_del ((void **)xcgi_qstrings);
   xcgi_qstrings = NULL;
}

static char **qstrings_add (const char *name, const char *value)
{
   bool error = true;
   char **ret = NULL;
   if (!name || !value)
      return NULL;

   if (!(ret = malloc (sizeof *ret * 2)))
      goto errorexit;

   ret[0] = ds_str_dup (name);
   ret[1] = ds_str_dup (value);
   if (!ret[0] || !ret[1])
      goto errorexit;

   if (!(ds_array_ins_tail ((void ***)&xcgi_qstrings, ret)))
      goto errorexit;

   error = false;

errorexit:
   if (error) {
      free (ret[0]);
      free (ret[1]);
      free (ret);
      ret = NULL;
   }

   return ret;
}

/* ************************************************************************
 */
static bool parse_path_info (void)
{
   if (!(xcgi_path_info = (const char **)ds_array_new ()))
      return false;

   char *tmp = ds_str_dup (xcgi_PATH_INFO);
   if (!tmp)
      return true;

   char *pathf = strtok (tmp, "/");
   while (pathf) {
      char *e = ds_str_dup (pathf);
      if (!e || !ds_array_ins_tail ((void ***)&xcgi_path_info, e)) {
         free (tmp);
         return false;
      }
      pathf = strtok (NULL, "/");
   }
   free (tmp);

   return true;
}

static void path_info_shutdown (void)
{
   for (size_t i=0; xcgi_path_info && xcgi_path_info[i]; i++) {
      free ((void *)xcgi_path_info[i]);
   }
   ds_array_del ((void **)xcgi_path_info);
   xcgi_path_info = NULL;
}

/* ************************************************************************
 */
static bool response_headers_init (void)
{
   if (!(cookielist_init ()))
      return false;

   return (xcgi_response_headers = (const char **)ds_array_new ())
               ? true : false;
}

static void response_headers_shutdown (void)
{
   cookielist_shutdown ();

   if (!xcgi_response_headers)
      return;

   for (size_t i=0; xcgi_response_headers[i]; i++) {
      free ((char *)xcgi_response_headers[i]);
   }
   ds_array_del ((void **)xcgi_response_headers);
   xcgi_response_headers = NULL;
}

static size_t response_headers_find (const char *name)
{
   if (!xcgi_response_headers)
      if (!(response_headers_init ()))
         return (size_t)-1;

   if (!name)
      return (size_t)-1;

   size_t len = strlen (name);

   for (size_t i=0; xcgi_response_headers[i]; i++) {
      if ((strncasecmp (xcgi_response_headers[i], name, len))==0)
         return i;
   }

   return (size_t)-1;
}


/* ************************************************************************
 */
struct {
   const char *name;
   const char **variable;
} g_vars[] = {
      { "CONTENT_LENGTH",           &xcgi_CONTENT_LENGTH          },
      { "CONTENT_TYPE",             &xcgi_CONTENT_TYPE            },
      { "CONTEXT_DOCUMENT_ROOT",    &xcgi_CONTEXT_DOCUMENT_ROOT   },
      { "CONTEXT_PREFIX",           &xcgi_CONTENT_PREFIX          },
      { "DOCUMENT_ROOT",            &xcgi_DOCUMENT_ROOT           },
      { "GATEWAY_INTERFACE",        &xcgi_GATEWAY_INTERFACE       },
      { "HOSTNAME",                 &xcgi_HOSTNAME                },
      { "HOSTTYPE",                 &xcgi_HOSTTYPE                },
      { "HTTP_ACCEPT",              &xcgi_HTTP_ACCEPT             },
      { "HTTP_COOKIE",              &xcgi_HTTP_COOKIE             },
      { "HTTP_HOST",                &xcgi_HTTP_HOST               },
      { "HTTP_REFERER",             &xcgi_HTTP_REFERER            },
      { "HTTP_USER_AGENT",          &xcgi_HTTP_USER_AGENT         },
      { "HTTPS",                    &xcgi_HTTPS                   },
      { "PATH",                     &xcgi_PATH                    },
      { "PATH_INFO",                &xcgi_PATH_INFO               },
      { "PWD",                      &xcgi_PWD                     },
      { "QUERY_STRING",             &xcgi_QUERY_STRING            },
      { "REMOTE_ADDR",              &xcgi_REMOTE_ADDR             },
      { "REMOTE_HOST",              &xcgi_REMOTE_HOST             },
      { "REMOTE_PORT",              &xcgi_REMOTE_PORT             },
      { "REMOTE_USER",              &xcgi_REMOTE_USER             },
      { "REQUEST_METHOD",           &xcgi_REQUEST_METHOD          },
      { "REQUEST_SCHEME",           &xcgi_REQUEST_SCHEME          },
      { "REQUEST_URI",              &xcgi_REQUEST_URI             },
      { "SCRIPT_FILENAME",          &xcgi_SCRIPT_FILENAME         },
      { "SCRIPT_NAME",              &xcgi_SCRIPT_NAME             },
      { "SERVER_ADDR",              &xcgi_SERVER_ADDR             },
      { "SERVER_ADMIN",             &xcgi_SERVER_ADMIN            },
      { "SERVER_NAME",              &xcgi_SERVER_NAME             },
      { "SERVER_PORT",              &xcgi_SERVER_PORT             },
      { "SERVER_PROTOCOL",          &xcgi_SERVER_PROTOCOL         },
      { "SERVER_SIGNATURE",         &xcgi_SERVER_SIGNATURE        },
      { "SERVER_SOFTWARE",          &xcgi_SERVER_SOFTWARE         },
};

bool xcgi_init (void)
{
   bool error = true;

   for (size_t i=0; i<sizeof g_vars/sizeof g_vars[0]; i++) {
      char *tmp = getenv (g_vars[i].name);
      *(g_vars[i].variable) = tmp ? tmp : "";
   }
   xcgi_stdin = stdin;

   if (!(qstrings_init ())) {
      fprintf (stderr, "Failed to allocate storage for the qstrings\n");
      goto errorexit;
   }

   if (!(qs_content_types_init ())) {
      fprintf (stderr, "Failed to allocate storage for the content types\n");
      goto errorexit;
   }

   if (!(parse_path_info ())) {
      fprintf (stderr, "Failed to parse the path info [%s]\n",
               xcgi_PATH_INFO);
      goto errorexit;
   }

   if (!(response_headers_init ())) {
      fprintf (stderr, "Failed to allocate storage for response headers\n");
      goto errorexit;
   }

   error = false;

errorexit:

   if (error) {
      xcgi_shutdown ();
   }

   return !error;
}

void xcgi_shutdown (void)
{
   if (xcgi_stdin)
      fclose (xcgi_stdin);

   qstrings_shutdown ();
   qs_content_types_shutdown ();
   path_info_shutdown ();
   response_headers_shutdown ();
}

#define MARKER_EOV      ("MARKER-END-OF-VARS")

static char g_line[1024  * 16];

bool xcgi_save (const char *fname)
{
   bool error = true;
   FILE *outf = NULL;
   size_t clen = 0;

   if (!(outf = fopen (fname, "w"))) {
      fprintf (stderr, "Failed to open [%s] for writing: %m\n", fname);
      goto errorexit;
   }

   for (size_t i=0; i<sizeof g_vars/sizeof g_vars[0]; i++) {
      char *tmp = xcgi_string_escape (*(g_vars[i].variable));
      fprintf (outf, "%s\x01%s\n", g_vars[i].name, tmp);
      free (tmp);
   }
   fprintf (outf, "%s\n", MARKER_EOV);

   if ((sscanf (xcgi_getenv ("CONTENT_LENGTH"), "%zu", &clen))==1) {
      fprintf (outf, "%zu\n", clen);
      while (!ferror (stdin) && !feof (stdin) && clen>0) {
         size_t must_read = clen < sizeof g_line ? clen : sizeof g_line;
         size_t nbytes = fread (g_line, 1, must_read, stdin);
         size_t written = fwrite (g_line, 1, nbytes, outf);
         if (written != nbytes) {
            fprintf (stderr, "Wrote only [%zu/%zu] bytes to file\n",
                     written, nbytes);
            goto errorexit;
         }
         clen -= nbytes;
      }
   }

   error = false;

errorexit:
   if (outf) {
      fclose (outf);
   }

   return !error;
}

bool xcgi_load (const char *fname)
{
   bool error = true;
   FILE *inf = NULL;
   char *tmp = NULL;
   size_t nlines = 0;
   size_t clen = 0;

   if (!(inf = fopen (fname, "r"))) {
      fprintf (stderr, "Failed to open [%s] for reading: %m\n", fname);
      goto errorexit;
   }

   while (!(feof (inf) && !ferror (inf))) {
      char *ltmp = fgets (g_line, sizeof g_line - 1, inf);
      if (!ltmp) {
         break;
      }

      tmp = strchr (g_line, '\n');
      if (tmp)
         *tmp = 0;

      nlines++;
      g_line[sizeof g_line - 1] = 0;
      if ((memcmp (g_line, MARKER_EOV, strlen (MARKER_EOV)))==0)
         break;

      tmp = strchr (g_line, 0x01);
      if (!tmp) {
         fprintf (stderr, "%zu Failed parsing variable [%s]. Aborting\n",
                           nlines, g_line);
         goto errorexit;
      }
      *tmp++ = 0;

      if (!(ltmp = xcgi_string_unescape (tmp))) {
         fprintf (stderr, "Failed to unescape string, aborting\n");
         goto errorexit;
      }

      // TODO: For Windows must use putenv_s()
      setenv (g_line, ltmp, 1);
      free (ltmp);
   }

   qstrings_shutdown ();
   qs_content_types_shutdown ();
   path_info_shutdown ();
   response_headers_shutdown ();

   xcgi_init ();

   if (!(fgets (g_line, sizeof g_line - 1, inf))) {
      error = false;
      goto errorexit;
   }

   tmp = strchr (g_line, '\n');
   if (tmp)
      *tmp = 0;

   if ((strcmp (xcgi_getenv ("CONTENT_LENGTH"), g_line))!=0) {
      fprintf (stderr, "Content length differs: [%s:%s]\n",
                  xcgi_getenv ("CONTENT_LENGTH"), g_line);
      goto errorexit;
   }

   if ((sscanf (g_line, "%zu\n", &clen))!=1)
      goto errorexit;

   xcgi_stdin = inf;

   error = false;

errorexit:
   if (error && inf) {
      fclose (inf);
   }

   return !error;
}

const char *xcgi_getenv (const char *name)
{
   for (size_t i=0; i<sizeof g_vars / sizeof g_vars[0]; i++) {
      if ((strcmp (g_vars[i].name, name))==0)
         return *(g_vars[i].variable);
   }

   return "";
}

char *xcgi_string_escape (const char *src)
{
   bool error = true;
   char *ret = NULL;

   static const char *allowed =
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "0123456789"
      "$-_.+!*'(),";

   if (!src)
      return NULL;

   size_t dst_size = 0;
   size_t dst_index = 0;

   for (size_t i=0; src[i]; i++) {
      if (!(strchr (allowed, src[i])))
         dst_size += 2;
      dst_size++;
   }

   if (!(ret = malloc (dst_size + 1))) {
      fprintf (stderr, "OOM error allocating escaped string\n");
      goto errorexit;
   }

   memset (ret, 0, dst_size + 1);

   for (size_t i=0; src[i]; i++) {
      if (!(strchr (allowed, src[i]))) {
         ret[dst_index++] = '%';
         sprintf (&ret[dst_index], "%02x", src[i]);
         dst_index += 2;
      } else {
         ret[dst_index++] = src[i];
      }
   }

   error = false;
errorexit:

   if (error) {
      free (ret);
      ret = NULL;
   }

   return ret;

}

char *xcgi_string_unescape (const char *src)
{
   bool error = true;
   char *ret = NULL;
   size_t len = 0,
          index = 0;

   if (!src)
      return NULL;

   len = strlen (src) + 1;

   if (!(ret = malloc (len)))
      goto errorexit;

   for (size_t i=0; src[i] && i<len; i++) {

      if (src[i]!='%') {
         ret[index++] = src[i];
         continue;
      }

      int tmp;
      char c;
      i++;
      if ((sscanf (&src[i], "%02x", &tmp))!=1) {
         fprintf (stderr, "Failed to scan escaped character at %s\n",
                  &src[i]);
         goto errorexit;
      }
      c = tmp;
      ret[index++] = c;
      i++;
   }

   ret[index] = 0;

   error = false;

errorexit:

   if (error) {
      free (ret);
      ret = NULL;
   }

   return ret;
}

bool xcgi_qstrings_accept_content_type (const char *content_type)
{
   return (qs_content_types_add (content_type)==NULL) ? false : true;
}

bool xcgi_qstrings_reject_content_type (const char *content_type)
{
   return qs_content_types_remove (content_type);
}

static bool xcgi_parse_query_string (void)
{
   bool error = true;
   char *uestring = xcgi_string_unescape (xcgi_QUERY_STRING);

   char *pair = NULL;

   pair = strtok (uestring, "&");
   while (pair) {
      char *sep = strchr (pair, '=');
      if (!sep) {
         fprintf (stderr, "Failed to find delimiter in [%s]\n", pair);
         goto errorexit;
      }
      *sep = 0;
      if (!(qstrings_add (pair, &sep[1]))) {
         fprintf (stderr, "Failed to add qstrings [%s:%s]\n", pair, sep);
         goto errorexit;
      }
      *sep = '=';
      pair = strtok (NULL, "&");
   }

   error = false;

errorexit:

   free (uestring);
   return !error;
}

static char *read_next_pair (FILE *inf)
{
   bool error = true;
   char *ret = NULL;

   char tmp[2];
   tmp[0] = 0;
   tmp[1] = 0;

   while (!feof (inf) && !ferror (inf) && ((tmp[0] = fgetc (inf))!=EOF)) {
      if (tmp[0]=='&')
         break;

      if (!(ds_str_append (&ret, tmp, NULL))) {
         fprintf (stderr, "OOM error reading POST pairs\n");
         goto errorexit;
      }
   }

   error = false;

errorexit:
   if (error) {
      free (ret);
      ret = NULL;
   }
   return ret;
}

static bool xcgi_parse_POST_query_string (void)
{
   bool error = true;
   char *pair = NULL;
   char *tmp = NULL;

   while ((pair = read_next_pair (xcgi_stdin))) {
      free (tmp);
      tmp = xcgi_string_unescape (pair);
      free (pair); pair = NULL;

      char *sep = strchr (tmp, '=');
      if (!sep) {
         fprintf (stderr, "Failed to find delimiter in [%s]\n", tmp);
         goto errorexit;
      }

      *sep++ = 0;

      if (!(qstrings_add (tmp, sep))) {
         fprintf (stderr, "Failed to add qstrings [%s:%s]\n", tmp, sep);
         goto errorexit;
      }

   }

   error = false;

errorexit:

   free (tmp);

   return !error;
}

bool xcgi_qstrings_parse (void)
{
   bool error = true;

   if (!(xcgi_parse_query_string ()))
      goto errorexit;

   if ((qs_content_types_check (xcgi_CONTENT_TYPE))) {
      if (!(xcgi_parse_POST_query_string ()))
         goto errorexit;
   }

   error = false;

errorexit:
   return !error;
}

size_t xcgi_qstrings_count (void)
{
   return ds_array_length ((void **)xcgi_qstrings);
}

bool xcgi_headers_value_set (const char *header, const char *value)
{
   size_t index = response_headers_find (header);
   char *tmp = NULL;

   if (index == (size_t)-1) {

      if (!(ds_str_printf (&tmp, "%s:%s", header, value)))
         return false;

      bool ret = ds_array_ins_tail ((void ***)&xcgi_response_headers, tmp);
      if (!ret)
         free (tmp);
      return ret;

   }

   if (!(ds_str_printf (&tmp, "%s, %s", xcgi_response_headers[index], value)))
      return false;

   free ((void *)xcgi_response_headers[index]);
   xcgi_response_headers[index] = tmp;
   return true;
}

void xcgi_headers_clear (const char *header)
{
   size_t index = response_headers_find (header);

   if (index != (size_t)-1) {
      free ((void *)xcgi_response_headers[index]);
      ds_array_remove ((void ***)&xcgi_response_headers, index);
   }
}

bool xcgi_headers_write (void)
{
   if (!xcgi_response_headers)
      return true;

   if (!(cookielist_write ()))
      return false;

   for (size_t i=0; xcgi_response_headers[i]; i++) {
      fprintf (stdout, "%s\r\n", xcgi_response_headers[i]);
   }

   return true;
}

size_t xcgi_path_info_count (void)
{
   return ds_array_length ((void **)xcgi_path_info);
}

size_t xcgi_headers_count (void)
{
   return ds_array_length ((void **)xcgi_response_headers);
}


