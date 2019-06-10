
#include <stdio.h>
#include <stdlib.h>

#include "xcgi.h"

#include "pubsub_error.h"

#include "ds_hmap.h"
#include "ds_str.h"

#define TYPE_STRING        (1)
#define TYPE_INT           (2)

static bool set_field (ds_hmap_t *hm, const char *name, const void *value,
                       int type)
{
   if (!hm || !name)
      return false;

   char *tmp = NULL;
   size_t nbytes = 0;

   if (type==TYPE_STRING)
      nbytes = ds_str_printf (&tmp, "\"%s\"", value);

   if (type==TYPE_INT)
      nbytes = ds_str_printf (&tmp, "%i", (int)((intptr_t)value));

   if (nbytes==0)
      return false;

   if (!(ds_hmap_set_str_str (hm, name, tmp))) {
      free (tmp);
      return false;
   }

   return true;
}

static bool set_sfield (ds_hmap_t *hm, const char *name, const char *value)
{
   return set_field (hm, name, value, TYPE_STRING);
}

static bool set_ifield (ds_hmap_t *hm, const char *name, int value)
{
   return set_field (hm, name, (void *)(intptr_t)value, TYPE_INT);
}

static void print_json (ds_hmap_t *hm)
{
   char **keys = NULL;
   size_t nkeys = ds_hmap_keys (hm, (void ***)&keys, NULL);

   printf ("{");
   for (size_t i=0; i<nkeys; i++) {
      char *value = NULL;
      if (!(ds_hmap_get_str_str (hm, keys[i], &value))) {
         fprintf (stderr, "Failed to retrieve key [%s]\n", keys[i]);
         goto errorexit;
      }
      printf ("%s\n\"%s\": %s", i ? "," : "", keys[i], value);
   }
   printf ("\n}\n");

errorexit:
   free (keys);
}

static void free_json (ds_hmap_t *hm)
{
   char **keys = NULL;
   size_t nkeys = ds_hmap_keys (hm, (void ***)&keys, NULL);

   for (size_t i=0; i<nkeys; i++) {
      char *value = NULL;
      if (!(ds_hmap_get_str_str (hm, keys[i], &value))) {
         fprintf (stderr, "Failed to retrieve key [%s]\n", keys[i]);
         goto errorexit;
      }
      free (value);
   }

   ds_hmap_del (hm);

errorexit:
   free (keys);
}

typedef bool (endpoint_func_t) (int *, int *);

static bool endpoint_ERROR (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_LOGIN (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_LOGOUT (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_NEW (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_RM (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_LIST (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_MOD (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_NEW (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_RM (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_MOD (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_ADDUSER (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_RMUSER (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_LIST (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_MEMBERS (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_GRANT_USER (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_REVOKE_USER (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_RESOURCE_USER (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_GRANT_GROUP (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_REVOKE_GROUP (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_RESOURCE_GROUP (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_NEW (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_RM (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_MOD (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_PUT (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_GET (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_DEL (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_LIST (int *error_code, int *status_code)
{
   error_code = error_code;
   status_code = status_code;
   return false;
}

static endpoint_func_t *endpoint_parse (const char *srcstr)
{
   static const struct {
      endpoint_func_t  *code;
      const char       *str;
   } endpts[] = {
      {  endpoint_ERROR,                  ""                      },
      {  endpoint_LOGIN,                  "login"                 },
      {  endpoint_LOGOUT,                 "logout"                },

      {  endpoint_USER_NEW,               "user-new"              },
      {  endpoint_USER_RM,                "user-rm"               },
      {  endpoint_USER_LIST,              "user-list"             },
      {  endpoint_USER_MOD,               "user-mod"              },

      {  endpoint_GROUP_NEW,              "group-new"             },
      {  endpoint_GROUP_RM,               "group-rm"              },
      {  endpoint_GROUP_MOD,              "group-mod"             },
      {  endpoint_GROUP_ADDUSER,          "group-adduser"         },
      {  endpoint_GROUP_RMUSER,           "group-rmuser"          },
      {  endpoint_GROUP_LIST,             "group-list"            },
      {  endpoint_GROUP_MEMBERS,          "group-members"         },

      {  endpoint_PERMS_GRANT_USER,       "perms-grant-user"      },
      {  endpoint_PERMS_REVOKE_USER,      "perms-revoke-user"     },
      {  endpoint_PERMS_RESOURCE_USER,    "perms-resource-user"   },
      {  endpoint_PERMS_GRANT_GROUP,      "perms-grant-group"     },
      {  endpoint_PERMS_REVOKE_GROUP,     "perms-revoke-group"    },
      {  endpoint_PERMS_RESOURCE_GROUP,   "perms-resource-group"  },

      {  endpoint_QUEUE_NEW,              "queue-new"             },
      {  endpoint_QUEUE_RM,               "queue-rm"              },
      {  endpoint_QUEUE_MOD,              "queue-mod"             },
      {  endpoint_QUEUE_PUT,              "queue-put"             },
      {  endpoint_QUEUE_GET,              "queue-get"             },
      {  endpoint_QUEUE_DEL,              "queue-del"             },
      {  endpoint_QUEUE_LIST,             "queue-list"            },
   };

   if (!srcstr)
      return endpoint_ERROR;

   for (size_t i=0; i<sizeof endpts/sizeof endpts[0]; i++) {
      if ((strcmp (srcstr, endpts[i].str))==0)
         return endpts[i].code;
   }
   return endpoint_ERROR;
}

int main (void)
{
   int ret = EXIT_FAILURE;

   int errorCode = 0;
   const char *errorMessage = "Success";

   int statusCode = 501;
   const char *statusMessage = "Internal Server Error";

   ds_hmap_t *jfields = NULL;
   endpoint_func_t *endpoint = endpoint_ERROR;

   if (!(jfields = ds_hmap_new (32))) {
      fprintf (stderr, "Failed to create hashmap for json fields\n");
      return EXIT_FAILURE;
   }

   if (!errorMessage) {
      fprintf (stderr, "Failed to allocate memory for error message\n");
      goto errorexit;
   }

   if (!(xcgi_init ())) {
      fprintf (stderr, "Failed to initialise the library\n");
      goto errorexit;
   }

   endpoint = endpoint_parse (xcgi_path_info[0]);

   if (!(xcgi_HTTP_COOKIE[0]) && endpoint!=endpoint_LOGIN) {
      errorCode = EPUBSUB_AUTH;
      goto errorexit;
   }

   if (!(endpoint (&errorCode, &statusCode)))
      goto errorexit;

   /*
   printf ("Path info [%zu]:\n", xcgi_path_info_count ());
   for (size_t i=0; xcgi_path_info[i]; i++) {
      printf ("   [%s]: %i\n", xcgi_path_info[i],
                               endpoint_parse (xcgi_path_info[i]));
   }
   printf ("/Path info\n");

   while (!feof (xcgi_stdin) && !ferror (xcgi_stdin)) {
      int c = fgetc (xcgi_stdin);
      if (c!=EOF) {
         fputc (c, stdout);
      }
   }
   */

   ret = EXIT_SUCCESS;

errorexit:

   errorMessage = pubsub_error_msg (errorCode);

   if (!(set_ifield (jfields, "errorCode", errorCode))) {
      fprintf (stderr, "Failed setting the errorCode field\n");
      return EXIT_FAILURE;
   }

   if (!(set_sfield (jfields, "errorMessage", errorMessage))) {
      fprintf (stderr, "Failed setting the errorMessage field\n");
      return EXIT_FAILURE;
   }

   statusMessage = xcgi_reason_phrase (statusCode);

   printf ("HTTP/1.1 %i %s\r\n", statusCode, statusMessage);

   xcgi_headers_write ();

   print_json (jfields);
   free_json (jfields);

   xcgi_shutdown ();

   return ret;
}

