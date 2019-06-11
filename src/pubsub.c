
#include <stdio.h>
#include <stdlib.h>

#include "xcgi.h"

#include "pubsub_error.h"

#include "ds_hmap.h"
#include "ds_str.h"


/* ******************************************************************
 * The field names as defined in the API spec document. When adding
 * elements to these lists ensure that the index (IDX) #define matches the
 * entry in the array g_incoming below it.
 */
#define FIELD_STR_EMAIL                 ("email")
#define FIELD_STR_PASSWORD              ("password")
#define FIELD_STR_SESSION               ("session")
#define FIELD_STR_NICK                  ("nick")
#define FIELD_STR_USER_ID               ("user-id")
#define FIELD_STR_EMAIL_PATTERN         ("email-pattern")
#define FIELD_STR_NICK_PATTERN          ("nick-pattern")
#define FIELD_STR_ID_PATTERN            ("id-pattern")
#define FIELD_STR_RESULTSET_COUNT       ("resultset-count")
#define FIELD_STR_RESULTSET             ("resultset")
#define FIELD_STR_OLD_EMAIL             ("old-email")
#define FIELD_STR_NEW_EMAIL             ("new-email")
#define FIELD_STR_GROUP_NAME            ("group-name")
#define FIELD_STR_GROUP_DESCRIPTION     ("group-description")
#define FIELD_STR_GROUP_ID              ("group-id")
#define FIELD_STR_OLD_GROUP_NAME        ("old-group-name")
#define FIELD_STR_NEW_GROUP_NAME        ("new-group-name")
#define FIELD_STR_GROUP_PATTERN         ("group-pattern")
#define FIELD_STR_PERMS                 ("perms")
#define FIELD_STR_RESOURCE              ("resource")
#define FIELD_STR_QUEUE_NAME            ("queue-name")
#define FIELD_STR_QUEUE_DESCRIPTION     ("queue-description")
#define FIELD_STR_QUEUE_ID              ("queue-id")
#define FIELD_STR_MESSAGE_ID            ("message-id")
#define FIELD_STR_MESSAGE_IDS           ("message-ids")

#define FIELD_IDX_EMAIL                 0
#define FIELD_IDX_PASSWORD              1
#define FIELD_IDX_SESSION               2
#define FIELD_IDX_NICK                  3
#define FIELD_IDX_USER_ID               4
#define FIELD_IDX_EMAIL_PATTERN         5
#define FIELD_IDX_NICK_PATTERN          6
#define FIELD_IDX_ID_PATTERN            7
#define FIELD_IDX_RESULTSET_COUNT       8
#define FIELD_IDX_RESULTSET             9
#define FIELD_IDX_OLD_EMAIL             10
#define FIELD_IDX_NEW_EMAIL             11
#define FIELD_IDX_GROUP_NAME            12
#define FIELD_IDX_GROUP_DESCRIPTION     13
#define FIELD_IDX_GROUP_ID              14
#define FIELD_IDX_OLD_GROUP_NAME        15
#define FIELD_IDX_NEW_GROUP_NAME        16
#define FIELD_IDX_GROUP_PATTERN         17
#define FIELD_IDX_PERMS                 18
#define FIELD_IDX_RESOURCE              19
#define FIELD_IDX_QUEUE_NAME            20
#define FIELD_IDX_QUEUE_DESCRIPTION     21
#define FIELD_IDX_QUEUE_ID              22
#define FIELD_IDX_MESSAGE_ID            23
#define FIELD_IDX_MESSAGE_IDS           24

static struct {
   const char *name;
   const char *value;
   size_t      len;
} g_incoming[] = {
   { FIELD_STR_EMAIL,               NULL, 0 },
   { FIELD_STR_PASSWORD,            NULL, 0 },
   { FIELD_STR_SESSION,             NULL, 0 },
   { FIELD_STR_NICK,                NULL, 0 },
   { FIELD_STR_USER_ID,             NULL, 0 },
   { FIELD_STR_EMAIL_PATTERN,       NULL, 0 },
   { FIELD_STR_NICK_PATTERN,        NULL, 0 },
   { FIELD_STR_ID_PATTERN,          NULL, 0 },
   { FIELD_STR_RESULTSET_COUNT,     NULL, 0 },
   { FIELD_STR_RESULTSET,           NULL, 0 },
   { FIELD_STR_OLD_EMAIL,           NULL, 0 },
   { FIELD_STR_NEW_EMAIL,           NULL, 0 },
   { FIELD_STR_GROUP_NAME,          NULL, 0 },
   { FIELD_STR_GROUP_DESCRIPTION,   NULL, 0 },
   { FIELD_STR_GROUP_ID,            NULL, 0 },
   { FIELD_STR_OLD_GROUP_NAME,      NULL, 0 },
   { FIELD_STR_NEW_GROUP_NAME,      NULL, 0 },
   { FIELD_STR_GROUP_PATTERN,       NULL, 0 },
   { FIELD_STR_PERMS,               NULL, 0 },
   { FIELD_STR_RESOURCE,            NULL, 0 },
   { FIELD_STR_QUEUE_NAME,          NULL, 0 },
   { FIELD_STR_QUEUE_DESCRIPTION,   NULL, 0 },
   { FIELD_STR_QUEUE_ID,            NULL, 0 },
   { FIELD_STR_MESSAGE_ID,          NULL, 0 },
   { FIELD_STR_MESSAGE_IDS,         NULL, 0 },
};

static bool init_incoming (void)
{
   size_t content_length = 0;
   char *input = NULL;

   // We return true because having no POST data is not an error
   if ((sscanf (xcgi_CONTENT_LENGTH, "%zu", &content_length))!=1)
      return true;

   // We return true because having POST which is not json is not an error
   if (!(strstr (xcgi_CONTENT_TYPE, "application/json")))
      return true;

   if (!(input = malloc (content_length + 1)))
      return false;

   memset (input, 0, content_length + 1);

   size_t nbytes = fread (input, 1, content_length, xcgi_stdin);
   if (nbytes!=content_length) {
      free (input);
      return false;
   }

   bool error = false;
   for (size_t i=0; i<sizeof g_incoming/sizeof g_incoming[0]; i++) {
      const char *tmp = xcgi_json (input, g_incoming[i].name, NULL);
      size_t len = xcgi_json_length (tmp);
      if (!(g_incoming[i].value = malloc (len + 1))) {
         error = true;
         break;
      }
      strncpy (g_incoming[i].value, tmp, len);
   }

   free (input);

   if (error) {
      for (size_t i=0; i<sizeof g_incoming/sizeof g_incoming[0]; i++) {
         free (g_incoming[i].value);
         g_incoming[i].value = NULL;
         g_incoming[i].len = 0;
      }
   }

   return !error;
}

/* ******************************************************************
 * Setting fields and generating the JSON for all the fields.
 */
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

/* ******************************************************************
 * All the endpoint handlers.
 */
typedef bool (endpoint_func_t) (ds_hmap_t *, int *, int *);

static bool endpoint_ERROR (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_ENDPOINT;
   *status_code = 200;
   return true;
}

static bool endpoint_LOGIN (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   if (!(set_sfield (jfields, FIELD_STR_SESSION, "0123456789")))
      return false;

   *error_code = 0;
   *status_code = 200;

   return true;
}

static bool endpoint_LOGOUT (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = 0;
   status_code = 200;
   return true;
}

static bool endpoint_USER_NEW (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_RM (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_LIST (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_MOD (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_NEW (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_RM (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_MOD (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_ADDUSER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_RMUSER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_LIST (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_MEMBERS (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_GRANT_USER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_REVOKE_USER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_RESOURCE_USER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_GRANT_GROUP (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_REVOKE_GROUP (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_RESOURCE_GROUP (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_NEW (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_RM (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_MOD (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_PUT (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_GET (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_DEL (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   error_code = error_code;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_LIST (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
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

   if (!(init_incoming ())) {
      fprintf (stderr, "Failed to read incoming json fields\n");
      goto errorexit;
   }

   endpoint = endpoint_parse (xcgi_path_info[0]);

   if (endpoint!=endpoint_LOGIN && !(xcgi_HTTP_COOKIE[0])) {
      errorCode = EPUBSUB_AUTH;
      statusCode = 200;
      goto errorexit;
   }

   if (!(endpoint (jfields, &errorCode, &statusCode)))
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

   if (endpoint!=endpoint_QUEUE_GET) {
      print_json (jfields);
   }
   free_json (jfields);

   xcgi_shutdown ();

   return ret;
}

