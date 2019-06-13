
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "xcgi.h"
#include "xcgi_json.h"

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
#define FIELD_STR_SESSION               ("session-id")
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

#define BIT_EMAIL                ((uint64_t)(1 << FIELD_IDX_EMAIL))
#define BIT_PASSWORD             ((uint64_t)(1 << FIELD_IDX_PASSWORD))
#define BIT_SESSION              ((uint64_t)(1 << FIELD_IDX_SESSION))
#define BIT_NICK                 ((uint64_t)(1 << FIELD_IDX_NICK))
#define BIT_USER_ID              ((uint64_t)(1 << FIELD_IDX_USER_ID))
#define BIT_EMAIL_PATTERN        ((uint64_t)(1 << FIELD_IDX_EMAIL_PATTERN))
#define BIT_NICK_PATTERN         ((uint64_t)(1 << FIELD_IDX_NICK_PATTERN))
#define BIT_ID_PATTERN           ((uint64_t)(1 << FIELD_IDX_ID_PATTERN))
#define BIT_RESULTSET_COUNT      ((uint64_t)(1 << FIELD_IDX_RESULTSET_COUNT))
#define BIT_RESULTSET            ((uint64_t)(1 << FIELD_IDX_RESULTSET))
#define BIT_OLD_EMAIL            ((uint64_t)(1 << FIELD_IDX_OLD_EMAIL))
#define BIT_NEW_EMAIL            ((uint64_t)(1 << FIELD_IDX_NEW_EMAIL))
#define BIT_GROUP_NAME           ((uint64_t)(1 << FIELD_IDX_GROUP_NAME))
#define BIT_GROUP_DESCRIPTION    ((uint64_t)(1 << FIELD_IDX_GROUP_DESCRIPTION))
#define BIT_GROUP_ID             ((uint64_t)(1 << FIELD_IDX_GROUP_ID))
#define BIT_OLD_GROUP_NAME       ((uint64_t)(1 << FIELD_IDX_OLD_GROUP_NAME))
#define BIT_NEW_GROUP_NAME       ((uint64_t)(1 << FIELD_IDX_NEW_GROUP_NAME))
#define BIT_GROUP_PATTERN        ((uint64_t)(1 << FIELD_IDX_GROUP_PATTERN))
#define BIT_PERMS                ((uint64_t)(1 << FIELD_IDX_PERMS))
#define BIT_RESOURCE             ((uint64_t)(1 << FIELD_IDX_RESOURCE))
#define BIT_QUEUE_NAME           ((uint64_t)(1 << FIELD_IDX_QUEUE_NAME))
#define BIT_QUEUE_DESCRIPTION    ((uint64_t)(1 << FIELD_IDX_QUEUE_DESCRIPTION))
#define BIT_QUEUE_ID             ((uint64_t)(1 << FIELD_IDX_QUEUE_ID))
#define BIT_MESSAGE_ID           ((uint64_t)(1 << FIELD_IDX_MESSAGE_ID))
#define BIT_MESSAGE_IDS          ((uint64_t)(1 << FIELD_IDX_MESSAGE_IDS))


#define P_ERROR                  (0)
#define P_LOGIN                  (BIT_EMAIL | BIT_PASSWORD)
#define P_LOGOUT                 (0)

#define P_USER_NEW               (BIT_EMAIL | BIT_NICK | BIT_PASSWORD)
#define P_USER_RM                (BIT_EMAIL)
#define P_USER_LIST              (BIT_EMAIL_PATTERN | BIT_NICK_PATTERN | BIT_ID_PATTERN)
#define P_USER_MOD               (BIT_OLD_EMAIL | BIT_NEW_EMAIL | BIT_NICK | BIT_PASSWORD)

#define P_GROUP_NEW              (BIT_GROUP_NAME | BIT_GROUP_DESCRIPTION)
#define P_GROUP_RM               (BIT_GROUP_NAME)
#define P_GROUP_MOD              (BIT_OLD_GROUP_NAME | BIT_NEW_GROUP_NAME)
#define P_GROUP_ADDUSER          (BIT_GROUP_NAME | BIT_EMAIL)
#define P_GROUP_RMUSER           (BIT_GROUP_NAME | BIT_EMAIL)
#define P_GROUP_LIST             (BIT_GROUP_PATTERN)
#define P_GROUP_MEMBERS          (BIT_GROUP_NAME)

#define P_PERMS_GRANT_USER       (BIT_EMAIL | BIT_PERMS | BIT_RESOURCE)
#define P_PERMS_REVOKE_USER      (BIT_EMAIL | BIT_PERMS | BIT_RESOURCE)
#define P_PERMS_RESOURCE_USER    (BIT_EMAIL | BIT_RESOURCE)
#define P_PERMS_GRANT_GROUP      (BIT_GROUP_NAME | BIT_PERMS | BIT_RESOURCE)
#define P_PERMS_REVOKE_GROUP     (BIT_GROUP_NAME | BIT_PERMS | BIT_RESOURCE)
#define P_PERMS_RESOURCE_GROUP   (BIT_GROUP_NAME | BIT_RESOURCE)

#define P_QUEUE_NEW              (BIT_QUEUE_NAME | BIT_QUEUE_DESCRIPTION)
#define P_QUEUE_RM               (BIT_QUEUE_ID)
#define P_QUEUE_MOD              (BIT_QUEUE_ID) // TODO
#define P_QUEUE_PUT              0
#define P_QUEUE_GET              0
#define P_QUEUE_DEL              0
#define P_QUEUE_LIST             0

static struct {
   const char *name;
   char       *value;
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

static void incoming_shutdown (void)
{
   for (size_t i=0; i<sizeof g_incoming/sizeof g_incoming[0]; i++) {
      free (g_incoming[i].value);
      g_incoming[i].value = NULL;
      g_incoming[i].len = 0;
   }
}

static bool incoming_valid (uint64_t mask)
{
   for (size_t i=0; i<sizeof mask; i++) {
      if (((uint64_t)1 << i) & mask) {
         if (!g_incoming[i].value)
            return false;
      }
   }
   return true;
}

static bool incoming_init (void)
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
      incoming_shutdown ();
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

   xcgi_header_cookie_clear (FIELD_STR_SESSION);
   if (!(xcgi_header_cookie_set (FIELD_STR_SESSION, "0123456789", 0, 0)))
      return false;

   *error_code = 0;
   *status_code = 200;

   return true;
}

static bool endpoint_LOGOUT (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   *status_code = 200;
   return true;
}

static bool endpoint_USER_NEW (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_RM (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_LIST (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_USER_MOD (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_NEW (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_RM (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_MOD (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_ADDUSER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_RMUSER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_LIST (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_GROUP_MEMBERS (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_GRANT_USER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_REVOKE_USER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_RESOURCE_USER (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_GRANT_GROUP (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_REVOKE_GROUP (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_PERMS_RESOURCE_GROUP (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_NEW (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_RM (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_MOD (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_PUT (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_GET (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_DEL (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static bool endpoint_QUEUE_LIST (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   jfields = jfields;
   *error_code = EPUBSUB_UNIMPLEMENTED;
   status_code = status_code;
   return false;
}

static const struct {
   endpoint_func_t  *fptr;
   const char       *str;
   uint64_t          params;
} g_endpts[] = {
{ endpoint_ERROR,                  "",                     P_ERROR                  },
{ endpoint_LOGIN,                  "login",                P_LOGIN                  },
{ endpoint_LOGOUT,                 "logout",               P_LOGOUT                 },

{ endpoint_USER_NEW,               "user-new",             P_USER_NEW               },
{ endpoint_USER_RM,                "user-rm",              P_USER_RM                },
{ endpoint_USER_LIST,              "user-list",            P_USER_LIST              },
{ endpoint_USER_MOD,               "user-mod",             P_USER_MOD               },

{ endpoint_GROUP_NEW,              "group-new",            P_GROUP_NEW              },
{ endpoint_GROUP_RM,               "group-rm",             P_GROUP_RM               },
{ endpoint_GROUP_MOD,              "group-mod",            P_GROUP_MOD              },
{ endpoint_GROUP_ADDUSER,          "group-adduser",        P_GROUP_ADDUSER          },
{ endpoint_GROUP_RMUSER,           "group-rmuser",         P_GROUP_RMUSER           },
{ endpoint_GROUP_LIST,             "group-list",           P_GROUP_LIST             },
{ endpoint_GROUP_MEMBERS,          "group-members",        P_GROUP_MEMBERS          },

{ endpoint_PERMS_GRANT_USER,       "perms-grant-user",     P_PERMS_GRANT_USER       },
{ endpoint_PERMS_REVOKE_USER,      "perms-revoke-user",    P_PERMS_REVOKE_USER      },
{ endpoint_PERMS_RESOURCE_USER,    "perms-resource-user",  P_PERMS_RESOURCE_USER    },
{ endpoint_PERMS_GRANT_GROUP,      "perms-grant-group",    P_PERMS_GRANT_GROUP      },
{ endpoint_PERMS_REVOKE_GROUP,     "perms-revoke-group",   P_PERMS_REVOKE_GROUP     },
{ endpoint_PERMS_RESOURCE_GROUP,   "perms-resource-group", P_PERMS_RESOURCE_GROUP   },

{ endpoint_QUEUE_NEW,              "queue-new",            P_QUEUE_NEW              },
{ endpoint_QUEUE_RM,               "queue-rm",             P_QUEUE_RM               },
{ endpoint_QUEUE_MOD,              "queue-mod",            P_QUEUE_MOD              },
{ endpoint_QUEUE_PUT,              "queue-put",            P_QUEUE_PUT              },
{ endpoint_QUEUE_GET,              "queue-get",            P_QUEUE_GET              },
{ endpoint_QUEUE_DEL,              "queue-del",            P_QUEUE_DEL              },
{ endpoint_QUEUE_LIST,             "queue-list",           P_QUEUE_LIST             },
   };

static endpoint_func_t *endpoint_parse (const char *srcstr)
{
   if (!srcstr)
      return endpoint_ERROR;

   for (size_t i=0; i<sizeof g_endpts/sizeof g_endpts[0]; i++) {
      if ((strcmp (srcstr, g_endpts[i].str))==0)
         return g_endpts[i].fptr;
   }
   return endpoint_ERROR;
}

static bool endpoint_valid_params (endpoint_func_t *fptr)
{
   if (!fptr)
      return false;

   for (size_t i=0; i<sizeof g_endpts/sizeof g_endpts[0]; i++) {
      if (g_endpts[i].fptr == fptr)
         return incoming_valid (g_endpts[i].params);
   }

   return false;
}

int main (void)
{
   int ret = EXIT_FAILURE;

   int error_code = 0;
   const char *error_message = "Success";

   int statusCode = 501;
   const char *statusMessage = "Internal Server Error";

   ds_hmap_t *jfields = NULL;
   endpoint_func_t *endpoint = endpoint_ERROR;

   if (!(jfields = ds_hmap_new (32))) {
      fprintf (stderr, "Failed to create hashmap for json fields\n");
      return EXIT_FAILURE;
   }

   if (!(xcgi_init ())) {
      fprintf (stderr, "Failed to initialise the library\n");
      goto errorexit;
   }

   if (!(incoming_init ())) {
      fprintf (stderr, "Failed to read incoming json fields\n");
      goto errorexit;
   }

   if ((endpoint = endpoint_parse (xcgi_path_info[0]))==endpoint_ERROR) {
      fprintf (stderr, "Warning: endpoint [%s] not found\n",
                        xcgi_path_info[0]);
   }

   if (endpoint!=endpoint_LOGIN && !(xcgi_HTTP_COOKIE[0])) {
      error_code = EPUBSUB_AUTH;
      statusCode = 200;
      goto errorexit;
   }

   if (!(endpoint_valid_params (endpoint))) {
      fprintf (stderr, "Endpoint [%s] missing required parameters\n",
                        xcgi_path_info[0]);
      error_code = EPUBSUB_BAD_PARAMS;
      statusCode = 200;
      goto errorexit;
   }

   if (!(endpoint (jfields, &error_code, &statusCode)))
      goto errorexit;


   ret = EXIT_SUCCESS;

errorexit:

   error_message = pubsub_error_msg (error_code);

   if (!(set_ifield (jfields, "error-code", error_code))) {
      fprintf (stderr, "Failed setting the error-code field\n");
      return EXIT_FAILURE;
   }

   if (!(set_sfield (jfields, "error-message", error_message))) {
      fprintf (stderr, "Failed setting the error-message field\n");
      return EXIT_FAILURE;
   }

   xcgi_headers_value_set ("CONTENT_TYPE", "application/json");

   statusMessage = xcgi_reason_phrase (statusCode);

   printf ("HTTP/1.1 %i %s\r\n", statusCode, statusMessage);

   xcgi_headers_write ();

   if (endpoint!=endpoint_QUEUE_GET) {
      print_json (jfields);
   }
   free_json (jfields);

   xcgi_shutdown ();
   incoming_shutdown ();

   return ret;
}

