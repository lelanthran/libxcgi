
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xcgi.h"

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
   for (size_t i=0; i<sizeof g_vars/sizeof g_vars[0]; i++) {
      char *tmp = getenv (g_vars[i].name);
      *(g_vars[i].variable) = tmp ? tmp : "";
   }
   xcgi_stdin = stdin;
   return true;
}

void xcgi_shutdown (void)
{
   if (xcgi_stdin)
      fclose (xcgi_stdin);
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
         printf ("Read [%zu] bytes, [%zu] remaining\n", nbytes, clen);
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

      printf ("Read [%s]\n", g_line);
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
      printf ("Set [%s:%s]\n", g_line, ltmp);
      free (ltmp);
   }

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

bool xcgi_qstrings_parse (void)
{
   return false;
}

size_t xcgi_qstrings_count (void)
{
   return 0;
}

const char ***xcgi_qstrings (void)
{
   return NULL;
}


