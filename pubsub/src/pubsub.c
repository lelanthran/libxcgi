
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "xcgi.h"
#include "xcgi_json.h"

#include "sqldb.h"
#include "sqldb_auth.h"

#include "pubsub_error.h"

#include "ds_hmap.h"
#include "ds_str.h"

// Used to marshall json fields.
#define TYPE_STRING        (1)
#define TYPE_INT           (2)
#define TYPE_ARRAY         (3)

static uint64_t perms_decode (const char *str);

// ErrorReportingMadeEasy(tm)
#define PROG_ERR(...)      do {\
      fprintf (stderr, "%s:%d: Fatal error: ", __FILE__, __LINE__);\
      fprintf (stderr, __VA_ARGS__);\
      fprintf (stderr, " [Aborting]\n");\
} while (0)

/* ******************************************************************
 * Globals. Ugly but necessary.
 */
char       *g_session_id = NULL;
char       *g_email = NULL;
char       *g_nick = NULL;
uint64_t    g_flags = 0;
uint64_t    g_id = 0;
uint64_t    g_perms = 0;

/* ******************************************************************
 * The field names as defined in the API spec document. When adding
 * elements to these lists ensure that the index (IDX) #define matches the
 * entry in the array g_incoming below it.
 */
#define FIELD_STR_EMAIL                    ("email")
#define FIELD_STR_PASSWORD                 ("password")
#define FIELD_STR_SESSION                  ("session-id")
#define FIELD_STR_NICK                     ("nick")
#define FIELD_STR_USER_ID                  ("user-id")
#define FIELD_STR_EMAIL_PATTERN            ("email-pattern")
#define FIELD_STR_NICK_PATTERN             ("nick-pattern")
#define FIELD_STR_ID_PATTERN               ("id-pattern")
#define FIELD_STR_RESULTSET_COUNT          ("resultset-count")
#define FIELD_STR_RESULTSET_EMAILS         ("resultset-emails")
#define FIELD_STR_RESULTSET_NICKS          ("resultset-nicks")
#define FIELD_STR_RESULTSET_FLAGS          ("resultset-flags")
#define FIELD_STR_RESULTSET_IDS            ("resultset-ids")
#define FIELD_STR_OLD_EMAIL                ("old-email")
#define FIELD_STR_NEW_EMAIL                ("new-email")
#define FIELD_STR_GROUP_NAME               ("group-name")
#define FIELD_STR_GROUP_DESCRIPTION        ("group-description")
#define FIELD_STR_GROUP_ID                 ("group-id")
#define FIELD_STR_OLD_GROUP_NAME           ("old-group-name")
#define FIELD_STR_NEW_GROUP_NAME           ("new-group-name")
#define FIELD_STR_NAME_PATTERN             ("name-pattern")
#define FIELD_STR_DESCRIPTION_PATTERN      ("description-pattern")
#define FIELD_STR_RESULTSET_NAMES          ("resultset-names")
#define FIELD_STR_RESULTSET_DESCRIPTIONS   ("resultset-descriptions")
#define FIELD_STR_PERMS                    ("perms")
#define FIELD_STR_TARGET_USER              ("target-user")
#define FIELD_STR_TARGET_GROUP             ("target-group")
#define FIELD_STR_RESOURCE                 ("resource")
#define FIELD_STR_QUEUE_NAME               ("queue-name")
#define FIELD_STR_QUEUE_DESCRIPTION        ("queue-description")
#define FIELD_STR_QUEUE_ID                 ("queue-id")
#define FIELD_STR_MESSAGE_ID               ("message-id")
#define FIELD_STR_MESSAGE_IDS              ("message-ids")
#define FIELD_STR_ERROR_MESSAGE            ("error-message")
#define FIELD_STR_ERROR_CODE               ("error-code")

#define BIT_EMAIL                    ((uint64_t)1 << 0 )
#define BIT_PASSWORD                 ((uint64_t)1 << 1 )
#define BIT_SESSION                  ((uint64_t)1 << 2 )
#define BIT_NICK                     ((uint64_t)1 << 3 )
#define BIT_USER_ID                  ((uint64_t)1 << 4 )
#define BIT_EMAIL_PATTERN            ((uint64_t)1 << 5 )
#define BIT_NICK_PATTERN             ((uint64_t)1 << 6 )
#define BIT_ID_PATTERN               ((uint64_t)1 << 7 )
#define BIT_RESULTSET_COUNT          ((uint64_t)1 << 8 )
#define BIT_RESULTSET_EMAILS         ((uint64_t)1 << 9 )
#define BIT_RESULTSET_NICKS          ((uint64_t)1 << 10)
#define BIT_RESULTSET_FLAGS          ((uint64_t)1 << 11)
#define BIT_RESULTSET_IDS            ((uint64_t)1 << 12)
#define BIT_OLD_EMAIL                ((uint64_t)1 << 13)
#define BIT_NEW_EMAIL                ((uint64_t)1 << 14)
#define BIT_GROUP_NAME               ((uint64_t)1 << 15)
#define BIT_GROUP_DESCRIPTION        ((uint64_t)1 << 16)
#define BIT_GROUP_ID                 ((uint64_t)1 << 17)
#define BIT_OLD_GROUP_NAME           ((uint64_t)1 << 18)
#define BIT_NEW_GROUP_NAME           ((uint64_t)1 << 19)
#define BIT_NAME_PATTERN             ((uint64_t)1 << 20)
#define BIT_DESCRIPTION_PATTERN      ((uint64_t)1 << 21)
#define BIT_RESULTSET_NAMES          ((uint64_t)1 << 22)
#define BIT_RESULTSET_DESCRIPTIONS   ((uint64_t)1 << 23)
#define BIT_PERMS                    ((uint64_t)1 << 24)
#define BIT_TARGET_USER              ((uint64_t)1 << 25)
#define BIT_TARGET_GROUP             ((uint64_t)1 << 26)
#define BIT_RESOURCE                 ((uint64_t)1 << 27)
#define BIT_QUEUE_NAME               ((uint64_t)1 << 28)
#define BIT_QUEUE_DESCRIPTION        ((uint64_t)1 << 29)
#define BIT_QUEUE_ID                 ((uint64_t)1 << 30)
#define BIT_MESSAGE_ID               ((uint64_t)1 << 31)
#define BIT_MESSAGE_IDS              ((uint64_t)1 << 32)
#define BIT_ERROR_MESSAGE            ((uint64_t)1 << 33)
#define BIT_ERROR_CODE               ((uint64_t)1 << 34)


// These are the constraints for each endpoint. It's a bitmask of which
// fields need to appear in the JSON we get from the client for each
// endpoint.
#define ARG_ERROR                  (0)
#define ARG_LOGIN                  (BIT_EMAIL | BIT_PASSWORD)
#define ARG_LOGOUT                 (0)

#define ARG_USER_NEW               (BIT_EMAIL | BIT_NICK | BIT_PASSWORD)
#define ARG_USER_RM                (BIT_EMAIL)
#define ARG_USER_LIST              (BIT_EMAIL_PATTERN | BIT_NICK_PATTERN | BIT_ID_PATTERN | BIT_RESULTSET_EMAILS | BIT_RESULTSET_NICKS | BIT_RESULTSET_FLAGS | BIT_RESULTSET_IDS)
#define ARG_USER_MOD               (BIT_OLD_EMAIL | BIT_NEW_EMAIL | BIT_NICK | BIT_PASSWORD)

#define ARG_GROUP_NEW              (BIT_GROUP_NAME | BIT_GROUP_DESCRIPTION)
#define ARG_GROUP_RM               (BIT_GROUP_NAME)
#define ARG_GROUP_MOD              (BIT_OLD_GROUP_NAME | BIT_NEW_GROUP_NAME | BIT_GROUP_DESCRIPTION)
#define ARG_GROUP_ADDUSER          (BIT_GROUP_NAME | BIT_EMAIL)
#define ARG_GROUP_RMUSER           (BIT_GROUP_NAME | BIT_EMAIL)
#define ARG_GROUP_LIST             (BIT_NAME_PATTERN | BIT_DESCRIPTION_PATTERN | BIT_RESULTSET_NAMES | BIT_RESULTSET_DESCRIPTIONS | BIT_RESULTSET_IDS)
#define ARG_GROUP_MEMBERS          (BIT_GROUP_NAME | BIT_RESULTSET_EMAILS | BIT_RESULTSET_NICKS | BIT_RESULTSET_FLAGS |  BIT_RESULTSET_IDS)

#define ARG_GRANT                  (BIT_EMAIL | BIT_PERMS)
#define ARG_GRANT_USER_O_USER      (BIT_EMAIL | BIT_PERMS | BIT_TARGET_USER)
#define ARG_GRANT_USER_O_GROUP     (BIT_EMAIL | BIT_PERMS | BIT_TARGET_GROUP)
#define ARG_GRANT_GROUP_O_USER     (BIT_GROUP_NAME | BIT_PERMS | BIT_TARGET_USER)
#define ARG_GRANT_GROUP_O_GROUP    (BIT_GROUP_NAME | BIT_PERMS | BIT_TARGET_GROUP)

#define ARG_REVOKE                  (BIT_EMAIL | BIT_PERMS)
#define ARG_REVOKE_USER_O_USER      (BIT_EMAIL | BIT_PERMS | BIT_TARGET_USER)
#define ARG_REVOKE_USER_O_GROUP     (BIT_EMAIL | BIT_PERMS | BIT_TARGET_GROUP)
#define ARG_REVOKE_GROUP_O_USER     (BIT_GROUP_NAME | BIT_PERMS | BIT_TARGET_USER)
#define ARG_REVOKE_GROUP_O_GROUP    (BIT_GROUP_NAME | BIT_PERMS | BIT_TARGET_GROUP)

#define ARG_QUEUE_NEW              (BIT_QUEUE_NAME | BIT_QUEUE_DESCRIPTION)
#define ARG_QUEUE_RM               (BIT_QUEUE_ID)
#define ARG_QUEUE_MOD              (BIT_QUEUE_ID) // TODO
#define ARG_QUEUE_PUT              0
#define ARG_QUEUE_GET              0
#define ARG_QUEUE_DEL              0
#define ARG_QUEUE_LIST             0


/* ******************************************************************
 * Setting the incoming data fields, and functions that search the
 * incoming data fields.
 */
struct incoming_value_t {
   const char *name;
   uint8_t     type;
   char       *value;
   size_t      len;
};
static struct incoming_value_t g_incoming[] = {
   { FIELD_STR_EMAIL,                  TYPE_STRING, NULL, 0 },
   { FIELD_STR_PASSWORD,               TYPE_STRING, NULL, 0 },
   { FIELD_STR_SESSION,                TYPE_STRING, NULL, 0 },
   { FIELD_STR_NICK,                   TYPE_STRING, NULL, 0 },
   { FIELD_STR_USER_ID,                TYPE_STRING, NULL, 0 },
   { FIELD_STR_EMAIL_PATTERN,          TYPE_STRING, NULL, 0 },
   { FIELD_STR_NICK_PATTERN,           TYPE_STRING, NULL, 0 },
   { FIELD_STR_ID_PATTERN,             TYPE_STRING, NULL, 0 },
   { FIELD_STR_RESULTSET_COUNT,        TYPE_STRING, NULL, 0 },
   { FIELD_STR_RESULTSET_EMAILS,       TYPE_STRING, NULL, 0 },
   { FIELD_STR_RESULTSET_NICKS,        TYPE_STRING, NULL, 0 },
   { FIELD_STR_RESULTSET_FLAGS,        TYPE_STRING, NULL, 0 },
   { FIELD_STR_RESULTSET_IDS,          TYPE_STRING, NULL, 0 },
   { FIELD_STR_OLD_EMAIL,              TYPE_STRING, NULL, 0 },
   { FIELD_STR_NEW_EMAIL,              TYPE_STRING, NULL, 0 },
   { FIELD_STR_GROUP_NAME,             TYPE_STRING, NULL, 0 },
   { FIELD_STR_GROUP_DESCRIPTION,      TYPE_STRING, NULL, 0 },
   { FIELD_STR_GROUP_ID,               TYPE_STRING, NULL, 0 },
   { FIELD_STR_OLD_GROUP_NAME,         TYPE_STRING, NULL, 0 },
   { FIELD_STR_NEW_GROUP_NAME,         TYPE_STRING, NULL, 0 },
   { FIELD_STR_NAME_PATTERN,           TYPE_STRING, NULL, 0 },
   { FIELD_STR_DESCRIPTION_PATTERN,    TYPE_STRING, NULL, 0 },
   { FIELD_STR_RESULTSET_NAMES,        TYPE_STRING, NULL, 0 },
   { FIELD_STR_RESULTSET_DESCRIPTIONS, TYPE_STRING, NULL, 0 },
   { FIELD_STR_PERMS,                  TYPE_STRING, NULL, 0 },
   { FIELD_STR_TARGET_USER,            TYPE_STRING, NULL, 0 },
   { FIELD_STR_TARGET_GROUP,           TYPE_STRING, NULL, 0 },
   { FIELD_STR_RESOURCE,               TYPE_STRING, NULL, 0 },
   { FIELD_STR_QUEUE_NAME,             TYPE_STRING, NULL, 0 },
   { FIELD_STR_QUEUE_DESCRIPTION,      TYPE_STRING, NULL, 0 },
   { FIELD_STR_QUEUE_ID,               TYPE_STRING, NULL, 0 },
   { FIELD_STR_MESSAGE_ID,             TYPE_STRING, NULL, 0 },
   { FIELD_STR_MESSAGE_IDS,            TYPE_STRING, NULL, 0 },
   { FIELD_STR_ERROR_MESSAGE,          TYPE_STRING, NULL, 0 },
   { FIELD_STR_ERROR_CODE,             TYPE_STRING, NULL, 0 },
};

static const char *incoming_find (const char *name)
{
   for (size_t i=0; i<sizeof g_incoming/sizeof g_incoming[0]; i++) {
      if ((strcmp (g_incoming[i].name, name))==0)
         return g_incoming[i].value;
   }
   return NULL;
}

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
   size_t masklen = sizeof mask * 8;
   if (masklen > (sizeof g_incoming / sizeof g_incoming[0]))
      masklen = (sizeof g_incoming / sizeof g_incoming[0]);

   for (size_t i=0; i<masklen; i++) {
      if (((uint64_t)1 << i) & mask) {
         // if (!g_incoming[i].value || !g_incoming[i].value[0])
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

   fwrite (input, 1, 1, stdout);

   if (nbytes!=content_length) {
      free (input);
      return false;
   }

   bool error = false;
   for (size_t i=0; i<sizeof g_incoming/sizeof g_incoming[0]; i++) {
      const char *tmp = xcgi_json_find (input, g_incoming[i].name, NULL);
      if (!tmp)
         continue;

      size_t len = xcgi_json_length (tmp);
      if (!(g_incoming[i].value = malloc (len + 1))) {
         error = true;
         break;
      }
      strncpy (g_incoming[i].value, tmp, len);
      g_incoming[i].value[len] = 0;

      if (g_incoming[i].value[0]=='"') {
         memmove (&g_incoming[i].value[0], &g_incoming[i].value[1], len);
         g_incoming[i].value[len-2] = 0;
      }
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
static bool set_field (ds_hmap_t *hm, const char *name, const void *value,
                       int type)
{
   if (!hm || !name)
      return true;

   char *tmp = NULL;
   size_t nbytes = 0;

   if (type==TYPE_STRING)
      nbytes = ds_str_printf (&tmp, "\"%s\"", value);

   if (type==TYPE_INT)
      nbytes = ds_str_printf (&tmp, "%i", (int)((intptr_t)value));

   if (type==TYPE_ARRAY)
      nbytes = ds_str_printf (&tmp, "%s", value);

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

static bool set_afield (ds_hmap_t *hm, const char *name, const char * value)
{
   return set_field (hm, name, value, TYPE_ARRAY);
}

static char *make_sarray (const char **src, size_t len)
{
   char *ret = NULL;

   if (!src)
      return NULL;

   size_t slen = 3;  // "[]"
   for (size_t i=0; i<len; i++) {
      slen += strlen (src[i]) + 5; // "s, "
   }

   if (!(ret = malloc (slen + 1)))
      return NULL;

   strcpy (ret, "[");

   for (size_t i=0; i<len; i++) {
      strcat (ret, "\"");
      strcat (ret, src[i]);
      strcat (ret, "\"");
      if (i<(len - 1))
         strcat (ret, ", ");
   }

   strcat (ret, "]");

   return ret;
}

static char *make_iarray (uint64_t *src, size_t len)
{
   char *ret = NULL;

   size_t slen = 3;  // "[]"
   for (size_t i=0; i<len; i++) {
      char tmp[22];
      snprintf (tmp, sizeof tmp, "%" PRIu64, src[i]);
      slen += strlen (tmp) + 3; // "s, "
   }

   if (!(ret = malloc (slen + 1)))
      return NULL;

   strcpy (ret, "[");

   for (size_t i=0; i<len; i++) {
      char tmp[22];
      snprintf (tmp, sizeof tmp, "%" PRIu64, src[i]);
      strcat (ret, tmp);
      if (i<(len - 1))
         strcat (ret, ", ");
   }

   strcat (ret, "]");

   return ret;
}

static void print_json (ds_hmap_t *hm)
{
   char **keys = NULL;
   size_t nkeys = ds_hmap_keys (hm, (void ***)&keys, NULL);

   printf ("{");
   for (size_t i=0; i<nkeys; i++) {
      char *value = NULL;
      if (!(ds_hmap_get_str_str (hm, keys[i], &value))) {
         PROG_ERR ("Failed to retrieve key [%s]\n", keys[i]);
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
         PROG_ERR ("Failed to retrieve key [%s]\n", keys[i]);
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
   char session[65];

   // No need to check for NULL, already checked in endpoint_valid_params()
   const char *in_email = incoming_find (FIELD_STR_EMAIL),
              *in_passwd = incoming_find (FIELD_STR_PASSWORD);

   memset (session, 0, sizeof session);

   if (!(sqldb_auth_session_authenticate (xcgi_db, in_email, in_passwd,
                                          session))) {
      *error_code = EPUBSUB_AUTH_FAILURE;
      *status_code = 200;
      return false;
   }

   if (!(set_sfield (jfields, FIELD_STR_SESSION, session))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      *status_code = 501;
      return false;
   }

   xcgi_header_cookie_clear (FIELD_STR_SESSION);
   if (!(xcgi_header_cookie_set (FIELD_STR_SESSION, session, 0, 0))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      *status_code = 501;
      return false;
   }

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
   const char *email = incoming_find (FIELD_STR_EMAIL),
              *nick = incoming_find (FIELD_STR_NICK),
              *password = incoming_find (FIELD_STR_PASSWORD);

   uint64_t new_id = (uint64_t)-1;

   *status_code = 200;

   if (!(strchr (email, '@'))) {
      *error_code = EPUBSUB_BAD_PARAMS;
      return false;
   }


   new_id = sqldb_auth_user_create (xcgi_db, email, nick, password);

   if (new_id==(uint64_t)-1) {
      *error_code = EPUBSUB_RESOURCE_EXISTS;
      return false;
   }

   char tmp[22];
   snprintf (tmp, sizeof tmp, "%" PRIu64, new_id);
   if (!(set_sfield (jfields, FIELD_STR_USER_ID, tmp))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   if (!(set_sfield (jfields, FIELD_STR_EMAIL, email))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   if (!(set_sfield (jfields, FIELD_STR_NICK, nick))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   *error_code = EPUBSUB_SUCCESS;
   return true;
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
   bool error = true;

   const char *email_pat = incoming_find (FIELD_STR_EMAIL_PATTERN),
              *nick_pat = incoming_find (FIELD_STR_NICK_PATTERN),
              *rqst_emails = incoming_find (FIELD_STR_RESULTSET_EMAILS),
              *rqst_nicks = incoming_find (FIELD_STR_RESULTSET_NICKS),
              *rqst_flags = incoming_find (FIELD_STR_RESULTSET_FLAGS),
              *rqst_ids = incoming_find (FIELD_STR_RESULTSET_IDS);

   char **emails = NULL,
        **nicks = NULL,
       ***ptr_emails = NULL,
       ***ptr_nicks = NULL;

   uint64_t nitems = 0,
            *flags = NULL,
            *ids = NULL,
           **ptr_flags = NULL,
           **ptr_ids = NULL;

   char *res_emails = NULL;
   char *res_nicks = NULL;
   char *res_flags = NULL;
   char *res_ids = NULL;

   char *epat = ds_str_chsubst (email_pat, /**/ '*', '%', /**/ '?', '_',   0),
        *npat = ds_str_chsubst (nick_pat,  /**/ '*', '%', /**/ '?', '_',   0);

   *status_code = 200;

   if (!epat || !npat) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

#define CHECK_TRUE(x)      (x && (((strcasecmp (x, "true"))==0) ||\
                                  (strcasecmp (x, "\"true\""))==0))

   if ((CHECK_TRUE (rqst_emails)))
      ptr_emails = &emails;

   if ((CHECK_TRUE (rqst_nicks)))
      ptr_nicks = &nicks;

   if ((CHECK_TRUE (rqst_flags)))
      ptr_flags = &flags;

   if ((CHECK_TRUE (rqst_ids)))
      ptr_ids = &ids;

#undef CHECK_TRUE

   if (!(sqldb_auth_user_find (xcgi_db, epat, npat,
                                        &nitems,
                                        ptr_emails,
                                        ptr_nicks,
                                        ptr_flags,
                                        ptr_ids))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

   res_emails = make_sarray ((const char **)emails, nitems);
   res_nicks = make_sarray ((const char **)nicks, nitems);
   res_flags = make_iarray (flags, nitems);
   res_ids = make_iarray (ids, nitems);

   if ((emails && !res_emails) ||
       (nicks && !res_nicks)   ||
       (flags && !res_flags)   ||
       (ids && !res_ids)) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

   if (!(set_ifield (jfields, FIELD_STR_RESULTSET_COUNT, nitems)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_EMAILS, res_emails)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_NICKS, res_nicks)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_FLAGS, res_flags)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_IDS, res_ids))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

   error = false;

errorexit:

   free (epat);
   free (npat);

   free (flags);
   free (ids);
   for (size_t i=0; i<nitems; i++) {
      if (emails)
         free (emails[i]);
      if (nicks)
         free (nicks[i]);
   }
   free (emails);
   free (nicks);

   free (res_emails);
   free (res_nicks);
   free (res_flags);
   free (res_ids);

   return !error;
}

static bool endpoint_USER_MOD (ds_hmap_t *jfields,
                               int *error_code, int *status_code)
{
   const char *old_email = incoming_find (FIELD_STR_OLD_EMAIL),
              *new_email = incoming_find (FIELD_STR_NEW_EMAIL),
              *nick = incoming_find (FIELD_STR_NICK),
              *password = incoming_find (FIELD_STR_PASSWORD);

   jfields = jfields;

   *status_code = 200;

   if (!(sqldb_auth_user_mod (xcgi_db, old_email, new_email, nick, password))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   *error_code = 0;

   return true;
}

static bool endpoint_GROUP_NEW (ds_hmap_t *jfields,
                                int *error_code, int *status_code)
{
   const char *group_name = incoming_find (FIELD_STR_GROUP_NAME),
              *group_description = incoming_find (FIELD_STR_GROUP_DESCRIPTION);

   uint64_t new_id = (uint64_t)-1;

   *status_code = 200;

   if (group_name[0]=='_') {
      *error_code = EPUBSUB_BAD_PARAMS;
      return false;
   }

   new_id = sqldb_auth_group_create (xcgi_db, group_name,
                                              group_description);

   if (new_id==(uint64_t)-1) {
      *error_code = EPUBSUB_RESOURCE_EXISTS;
      return false;
   }

   char tmp[22];
   snprintf (tmp, sizeof tmp, "%" PRIu64, new_id);
   if (!(set_sfield (jfields, FIELD_STR_GROUP_ID, tmp))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   if (!(set_sfield (jfields, FIELD_STR_GROUP_NAME, group_name))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   if (!(set_sfield (jfields, FIELD_STR_GROUP_DESCRIPTION, group_description))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   *error_code = EPUBSUB_SUCCESS;
   return true;
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
   const char *old_name = incoming_find (FIELD_STR_OLD_GROUP_NAME),
              *new_name = incoming_find (FIELD_STR_NEW_GROUP_NAME),
              *description = incoming_find (FIELD_STR_GROUP_DESCRIPTION);

   jfields = jfields;

   *status_code = 200;

   if (!(sqldb_auth_group_mod (xcgi_db, old_name, new_name, description))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   *error_code = 0;

   return true;
}

static bool endpoint_GROUP_ADDUSER (ds_hmap_t *jfields,
                                    int *error_code, int *status_code)
{
   const char *group_name = incoming_find (FIELD_STR_GROUP_NAME),
              *email      = incoming_find (FIELD_STR_EMAIL);

   jfields = jfields;
   *status_code = 200;

   if (!(sqldb_auth_group_adduser (xcgi_db, group_name, email))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   *error_code = 0;
   return true;
}

static bool endpoint_GROUP_RMUSER (ds_hmap_t *jfields,
                                   int *error_code, int *status_code)
{
   const char *group_name = incoming_find (FIELD_STR_GROUP_NAME),
              *email      = incoming_find (FIELD_STR_EMAIL);

   jfields = jfields;
   *status_code = 200;

   if (!(sqldb_auth_group_rmuser (xcgi_db, group_name, email))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   *error_code = 0;
   return true;
}

static bool endpoint_GROUP_LIST (ds_hmap_t *jfields,
                                 int *error_code, int *status_code)
{
   bool error = true;

   const char
      *name_pat          = incoming_find (FIELD_STR_NAME_PATTERN),
      *description_pat   = incoming_find (FIELD_STR_DESCRIPTION_PATTERN),
      *rqst_names        = incoming_find (FIELD_STR_RESULTSET_NAMES),
      *rqst_descriptions = incoming_find (FIELD_STR_RESULTSET_DESCRIPTIONS),
      *rqst_ids          = incoming_find (FIELD_STR_RESULTSET_IDS);

   char **names = NULL,
        **descriptions = NULL,
       ***ptr_names = NULL,
       ***ptr_descriptions = NULL;

   uint64_t nitems = 0,
            *ids = NULL,
           **ptr_ids = NULL;

   char *res_names = NULL;
   char *res_descriptions = NULL;
   char *res_ids = NULL;

   char
     *npat = ds_str_chsubst (name_pat,        /**/ '*', '%', /**/ '?', '_', 0),
     *dpat = ds_str_chsubst (description_pat, /**/ '*', '%', /**/ '?', '_', 0);

   *status_code = 200;

   if (!npat || !dpat) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

#define CHECK_TRUE(x)      (x && (((strcasecmp (x, "true"))==0) ||\
                                  (strcasecmp (x, "\"true\""))==0))

   if ((CHECK_TRUE (rqst_names)))
      ptr_names = &names;

   if ((CHECK_TRUE (rqst_descriptions)))
      ptr_descriptions = &descriptions;

   if ((CHECK_TRUE (rqst_ids)))
      ptr_ids = &ids;

#undef CHECK_TRUE

   if (!(sqldb_auth_group_find (xcgi_db, npat, dpat,
                                         &nitems,
                                         ptr_names,
                                         ptr_descriptions,
                                         ptr_ids))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

   res_names = make_sarray ((const char **)names, nitems);
   res_descriptions = make_sarray ((const char **)descriptions, nitems);
   res_ids = make_iarray (ids, nitems);

   if ((names && !res_names) ||
       (descriptions && !res_descriptions)   ||
       (ids && !res_ids)) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

   if (!(set_ifield (jfields, FIELD_STR_RESULTSET_COUNT, nitems)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_NAMES, res_names)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_DESCRIPTIONS, res_descriptions)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_IDS, res_ids))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

   error = false;

errorexit:

   free (npat);
   free (dpat);

   free (ids);
   for (size_t i=0; i<nitems; i++) {
      if (names)
         free (names[i]);
      if (descriptions)
         free (descriptions[i]);
   }
   free (names);
   free (descriptions);

   free (res_names);
   free (res_descriptions);
   free (res_ids);

   return !error;
}

static bool endpoint_GROUP_MEMBERS (ds_hmap_t *jfields,
                                    int *error_code, int *status_code)
{
   bool error = true;

   const char *group_name = incoming_find (FIELD_STR_GROUP_NAME),
              *rqst_emails = incoming_find (FIELD_STR_RESULTSET_EMAILS),
              *rqst_nicks = incoming_find (FIELD_STR_RESULTSET_NICKS),
              *rqst_flags = incoming_find (FIELD_STR_RESULTSET_FLAGS),
              *rqst_ids = incoming_find (FIELD_STR_RESULTSET_IDS);

   char **emails = NULL,
        **nicks = NULL,
       ***ptr_emails = NULL,
       ***ptr_nicks = NULL;

   uint64_t nitems = 0,
            *flags = NULL,
            *ids = NULL,
           **ptr_flags = NULL,
           **ptr_ids = NULL;

   char *res_emails = NULL;
   char *res_nicks = NULL;
   char *res_flags = NULL;
   char *res_ids = NULL;

   *status_code = 200;

#define CHECK_TRUE(x)      (x && (((strcasecmp (x, "true"))==0) ||\
                                  (strcasecmp (x, "\"true\""))==0))

   if ((CHECK_TRUE (rqst_emails)))
      ptr_emails = &emails;

   if ((CHECK_TRUE (rqst_nicks)))
      ptr_nicks = &nicks;

   if ((CHECK_TRUE (rqst_flags)))
      ptr_flags = &flags;

   if ((CHECK_TRUE (rqst_ids)))
      ptr_ids = &ids;

#undef CHECK_TRUE

   if (!(sqldb_auth_group_members (xcgi_db, group_name,
                                            &nitems,
                                            ptr_emails,
                                            ptr_nicks,
                                            ptr_flags,
                                            ptr_ids))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

   res_emails = make_sarray ((const char **)emails, nitems);
   res_nicks = make_sarray ((const char **)nicks, nitems);
   res_flags = make_iarray (flags, nitems);
   res_ids = make_iarray (ids, nitems);

   if ((emails && !res_emails) ||
       (nicks && !res_nicks)   ||
       (flags && !res_flags)   ||
       (ids && !res_ids)) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

   if (!(set_ifield (jfields, FIELD_STR_RESULTSET_COUNT, nitems)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_EMAILS, res_emails)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_NICKS, res_nicks)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_FLAGS, res_flags)) ||
       !(set_afield (jfields, FIELD_STR_RESULTSET_IDS, res_ids))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      goto errorexit;
   }

   error = false;

errorexit:

   free (flags);
   free (ids);
   for (size_t i=0; i<nitems; i++) {
      if (emails)
         free (emails[i]);
      if (nicks)
         free (nicks[i]);
   }
   free (emails);
   free (nicks);

   free (res_emails);
   free (res_nicks);
   free (res_flags);
   free (res_ids);

   return !error;
}

static bool endpoint_GRANT (ds_hmap_t *jfields,
                            int *error_code, int *status_code)
{
   const char *email = incoming_find (FIELD_STR_EMAIL),
              *permstr = incoming_find (FIELD_STR_PERMS);

   uint64_t perms = perms_decode (permstr);

   jfields = jfields;
   *status_code = 200;

   if (!(sqldb_auth_perms_grant_user (xcgi_db, email,
                                               SQLDB_AUTH_GLOBAL_RESOURCE,
                                               perms))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   return true;
}

static bool endpoint_REVOKE (ds_hmap_t *jfields,
                             int *error_code, int *status_code)
{
   const char *email = incoming_find (FIELD_STR_EMAIL),
              *permstr = incoming_find (FIELD_STR_PERMS);

   uint64_t perms = perms_decode (permstr);

   jfields = jfields;
   *status_code = 200;

   if (!(sqldb_auth_perms_revoke_user (xcgi_db, email,
                                       SQLDB_AUTH_GLOBAL_RESOURCE,
                                       perms))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   return true;
}

static bool
endpoint_g_r (bool (*fptr) (sqldb_t *, const char *, const char *, uint64_t),
              const char *subj, const char *target,
              ds_hmap_t *jfields,
              int *error_code, int *status_code)
{
   const char *p_subj = incoming_find (subj),
              *p_target = incoming_find (target),
              *permstr = incoming_find (FIELD_STR_PERMS);

   uint64_t perms = perms_decode (permstr);

   jfields = jfields;
   *status_code = 200;

   if (!(fptr (xcgi_db, p_subj, p_target, perms))) {
      *error_code = EPUBSUB_INTERNAL_ERROR;
      return false;
   }

   return true;
}

static bool endpoint_GRANT_USER_O_USER (ds_hmap_t *jfields,
                                        int *error_code, int *status_code)
{
   return endpoint_g_r (sqldb_auth_perms_grant_user,
                        FIELD_STR_EMAIL, FIELD_STR_TARGET_USER,
                        jfields, error_code, status_code);
}

static bool endpoint_GRANT_USER_O_GROUP (ds_hmap_t *jfields,
                                         int *error_code, int *status_code)
{
   return endpoint_g_r (sqldb_auth_perms_grant_user,
                        FIELD_STR_EMAIL, FIELD_STR_TARGET_GROUP,
                        jfields, error_code, status_code);
}

static bool endpoint_GRANT_GROUP_O_USER (ds_hmap_t *jfields,
                                         int *error_code, int *status_code)
{
   return endpoint_g_r (sqldb_auth_perms_grant_group,
                        FIELD_STR_GROUP_NAME, FIELD_STR_TARGET_USER,
                        jfields, error_code, status_code);
}

static bool endpoint_GRANT_GROUP_O_GROUP (ds_hmap_t *jfields,
                                          int *error_code, int *status_code)
{
   return endpoint_g_r (sqldb_auth_perms_grant_group,
                        FIELD_STR_GROUP_NAME, FIELD_STR_TARGET_GROUP,
                        jfields, error_code, status_code);
}

static bool endpoint_REVOKE_USER_O_USER (ds_hmap_t *jfields,
                                         int *error_code, int *status_code)
{
   return endpoint_g_r (sqldb_auth_perms_revoke_user,
                        FIELD_STR_EMAIL, FIELD_STR_TARGET_USER,
                        jfields, error_code, status_code);
}

static bool endpoint_REVOKE_USER_O_GROUP (ds_hmap_t *jfields,
                                          int *error_code, int *status_code)
{
   return endpoint_g_r (sqldb_auth_perms_revoke_user,
                        FIELD_STR_EMAIL, FIELD_STR_TARGET_GROUP,
                        jfields, error_code, status_code);
}

static bool endpoint_REVOKE_GROUP_O_USER (ds_hmap_t *jfields,
                                          int *error_code, int *status_code)
{
   return endpoint_g_r (sqldb_auth_perms_revoke_group,
                        FIELD_STR_EMAIL, FIELD_STR_TARGET_USER,
                        jfields, error_code, status_code);
}

static bool endpoint_REVOKE_GROUP_O_GROUP (ds_hmap_t *jfields,
                                           int *error_code, int *status_code)
{
   return endpoint_g_r (sqldb_auth_perms_revoke_group,
                        FIELD_STR_EMAIL, FIELD_STR_TARGET_GROUP,
                        jfields, error_code, status_code);
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
{ endpoint_ERROR,                  "",                     ARG_ERROR          },
{ endpoint_LOGIN,                  "login",                ARG_LOGIN          },
{ endpoint_LOGOUT,                 "logout",               ARG_LOGOUT         },

{ endpoint_USER_NEW,               "user-new",             ARG_USER_NEW       },
{ endpoint_USER_RM,                "user-rm",              ARG_USER_RM        },
{ endpoint_USER_LIST,              "user-list",            ARG_USER_LIST      },
{ endpoint_USER_MOD,               "user-mod",             ARG_USER_MOD       },

{ endpoint_GROUP_NEW,              "group-new",            ARG_GROUP_NEW      },
{ endpoint_GROUP_RM,               "group-rm",             ARG_GROUP_RM       },
{ endpoint_GROUP_MOD,              "group-mod",            ARG_GROUP_MOD      },
{ endpoint_GROUP_ADDUSER,          "group-adduser",        ARG_GROUP_ADDUSER  },
{ endpoint_GROUP_RMUSER,           "group-rmuser",         ARG_GROUP_RMUSER   },
{ endpoint_GROUP_LIST,             "group-list",           ARG_GROUP_LIST     },
{ endpoint_GROUP_MEMBERS,          "group-members",        ARG_GROUP_MEMBERS  },

{ endpoint_GRANT,                  "grant",                          ARG_GRANT                },
{ endpoint_GRANT_USER_O_USER,      "grant-to-user-over-user",        ARG_GRANT_USER_O_USER    },
{ endpoint_GRANT_USER_O_GROUP,     "grant-to-user-over-group",       ARG_GRANT_USER_O_GROUP   },
{ endpoint_GRANT_GROUP_O_USER,     "grant-to-group-over-user",       ARG_GRANT_GROUP_O_USER   },
{ endpoint_GRANT_GROUP_O_GROUP,    "grant-to-group-over-group",      ARG_GRANT_GROUP_O_GROUP  },

{ endpoint_REVOKE,                  "revoke",                        ARG_REVOKE               },
{ endpoint_REVOKE_USER_O_USER,      "revoke-from-user-over-user",    ARG_REVOKE_USER_O_USER   },
{ endpoint_REVOKE_USER_O_GROUP,     "revoke-from-user-over-group",   ARG_REVOKE_USER_O_GROUP  },
{ endpoint_REVOKE_GROUP_O_USER,     "revoke-from-group-over-user",   ARG_REVOKE_GROUP_O_USER  },
{ endpoint_REVOKE_GROUP_O_GROUP,    "revoke-from-group-over-group",  ARG_REVOKE_GROUP_O_GROUP },


{ endpoint_QUEUE_NEW,              "queue-new",  ARG_QUEUE_NEW  },
{ endpoint_QUEUE_RM,               "queue-rm",   ARG_QUEUE_RM   },
{ endpoint_QUEUE_MOD,              "queue-mod",  ARG_QUEUE_MOD  },
{ endpoint_QUEUE_PUT,              "queue-put",  ARG_QUEUE_PUT  },
{ endpoint_QUEUE_GET,              "queue-get",  ARG_QUEUE_GET  },
{ endpoint_QUEUE_DEL,              "queue-del",  ARG_QUEUE_DEL  },
{ endpoint_QUEUE_LIST,             "queue-list", ARG_QUEUE_LIST },
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


/* ******************************************************************
 * Manage all the permissions. A single permission is a set of 64 flags
 * for a particular resource. Each user may have multiple permissions, but
 * only the first match against a resource is considered.
 */

static uint64_t perms_get (const char *email, const char *resource)
{
   uint64_t ret = 0;
   if ((sqldb_auth_perms_get_all (xcgi_db, &ret, email, resource)))
      return ret;

   PROG_ERR ("Failed to get permissions for user [%s/%s]\n", email, resource);
   return 0;
}


#define PERM_STR_ALL                   ("all")
#define PERM_STR_CREATE_USER           ("create-user")
#define PERM_STR_CREATE_GROUP          ("create-group")
#define PERM_STR_DEL_USER              ("delete-user")
#define PERM_STR_DEL_GROUP             ("delete-group")
#define PERM_STR_READ                  ("read")
#define PERM_STR_LIST_MEMBERS          ("list-members")
#define PERM_STR_MODIFY                ("modify")
#define PERM_STR_DELETE                ("delete")
#define PERM_STR_CHANGE_PERMISSIONS    ("change-permissions")
#define PERM_STR_CHANGE_MEMBERSHIP     ("change-membership")

#define PERM_BIT_ALL                   ((uint64_t)(((uint64_t)1) << 0))
#define PERM_BIT_CREATE_USER           ((uint64_t)(((uint64_t)1) << 1))
#define PERM_BIT_CREATE_GROUP          ((uint64_t)(((uint64_t)1) << 2))
#define PERM_BIT_DEL_USER              ((uint64_t)(((uint64_t)1) << 3))
#define PERM_BIT_DEL_GROUP             ((uint64_t)(((uint64_t)1) << 4))
#define PERM_BIT_READ                  ((uint64_t)(((uint64_t)1) << 5))
#define PERM_BIT_LIST_MEMBERS          ((uint64_t)(((uint64_t)1) << 6))
#define PERM_BIT_MODIFY                ((uint64_t)(((uint64_t)1) << 6))
#define PERM_BIT_DELETE                ((uint64_t)(((uint64_t)1) << 7))
#define PERM_BIT_CHANGE_PERMISSIONS    ((uint64_t)(((uint64_t)1) << 8))
#define PERM_BIT_CHANGE_MEMBERSHIP     ((uint64_t)(((uint64_t)1) << 9))

static uint64_t perms_decode (const char *str)
{
   uint64_t ret = 0;
   bool all = (strstr (str, PERM_STR_ALL)) ? true : false;

   if (all || (strstr (str, PERM_STR_CREATE_USER)))
      ret |= PERM_BIT_CREATE_USER;

   if (all || (strstr (str, PERM_STR_CREATE_GROUP)))
      ret |= PERM_BIT_CREATE_GROUP;

   if (all || (strstr (str, PERM_STR_DEL_USER)))
      ret |= PERM_BIT_DEL_USER;

   if (all || (strstr (str, PERM_STR_DEL_GROUP)))
      ret |= PERM_BIT_DEL_GROUP;


   if (all || (strstr (str, PERM_STR_READ)))
      ret |= PERM_BIT_READ;

   if (all || (strstr (str, PERM_STR_LIST_MEMBERS)))
      ret |= PERM_BIT_LIST_MEMBERS;

   if (all || (strstr (str, PERM_STR_MODIFY)))
      ret |= PERM_BIT_MODIFY;

   if (all || (strstr (str, PERM_STR_DELETE)))
      ret |= PERM_BIT_DELETE;

   if (all || (strstr (str, PERM_STR_CHANGE_PERMISSIONS)))
      ret |= PERM_BIT_CHANGE_PERMISSIONS;

   if (all || (strstr (str, PERM_STR_CHANGE_MEMBERSHIP)))
      ret |= PERM_BIT_CHANGE_MEMBERSHIP;

   return ret;
}

/* ******************************************************************
 * Misc functions, then main
 */

static void print_help (void)
{
   static const char *msg[] = {
"Pubsub must be started as a cgi program from the webserver.",
"",
"1. Set the environment variable PUBSUB_WORKING_DIR (note case) to point to",
"   the directory that must be used as the working directory.",
"2. Update the $PUBSUB_WORKING_DIR/xcgi.ini file with the relevant",
"   information (database connection string, credentials, etc).",
"3. Use the sqldb_auth_cli program to initialise a database for use (storing",
"   the credentials and connection strings in the xcgi.ini file). When",
"   using sqlite as the database, the sqlite database file must be specified",
"   relative to $PUBSUB_WORKING_DIR.",
"",
   };

   for (size_t i=0; i<sizeof msg/sizeof msg[0]; i++) {
      printf ("%s\n", msg[i]);
   }
}

#define WORKING_DIR                 ("PUBSUB_WORKING_DIR")

int main (int argc, char **argv)
{
   int ret = EXIT_FAILURE;

   int error_code = 0;
   const char *error_message = "Success";
   const char *wdir = getenv (WORKING_DIR);

   int statusCode = 501;
   const char *statusMessage = "Internal Server Error";

   ds_hmap_t *jfields = NULL;
   endpoint_func_t *endpoint = endpoint_ERROR;

   if (argc>1 || argv[1]) {
      print_help ();
      return EXIT_FAILURE;
   }

   if (!wdir || !wdir[0]) {
      PROG_ERR ("Environment variable [%s] is not set.\n",
                 WORKING_DIR);
      return EXIT_FAILURE;
   }

   if (!(jfields = ds_hmap_new (32))) {
      PROG_ERR ("Failed to create hashmap for json fields\n");
      return EXIT_FAILURE;
   }

   if (!(xcgi_init (wdir))) {
      PROG_ERR ("Failed to initialise the xcgi library\n");
      goto errorexit;
   }

   {
      size_t ncookies = xcgi_cookies_count ();
      size_t slen = strlen (FIELD_STR_SESSION);

      g_session_id = "";

      for (size_t i=0; xcgi_cookies[i] && i<ncookies; i++) {
         if ((strncmp (FIELD_STR_SESSION, xcgi_cookies[i], slen))==0) {
            g_session_id = strchr (xcgi_cookies[i], '=');
            if (g_session_id)
               g_session_id++;
            break;
         }
      }
   }

   if (!xcgi_db) {
      PROG_ERR ("No database available\n");
      goto errorexit;
   }

   if (!(incoming_init ())) {
      PROG_ERR ("Failed to read incoming json fields\n");
      goto errorexit;
   }

   if ((endpoint = endpoint_parse (xcgi_path_info[0]))==endpoint_ERROR) {
      PROG_ERR ("Warning: endpoint [%s] not found\n", xcgi_path_info[0]);
   }

   if (endpoint!=endpoint_LOGIN && !g_session_id) {
      error_code = EPUBSUB_NOT_AUTH;
      statusCode = 200;
      goto errorexit;
   }

   if (endpoint==endpoint_LOGIN || endpoint==endpoint_LOGOUT) {
      xcgi_header_cookie_clear (FIELD_STR_SESSION);
      xcgi_header_cookie_set (FIELD_STR_SESSION, "", 0, 0);
      g_perms = 0xffffffffffffffff;
   } else {
      char session_id[65];
      strncpy (session_id, g_session_id, sizeof session_id);
      session_id[sizeof session_id - 1] = 0;
      if (!(sqldb_auth_session_valid (xcgi_db, session_id,
                                               &g_email,
                                               &g_nick,
                                               &g_flags,
                                               &g_id))) {
         PROG_ERR ("Failed to find a session for [%s]\n", g_session_id);
         xcgi_header_cookie_clear (FIELD_STR_SESSION);
         xcgi_header_cookie_set (FIELD_STR_SESSION, "", 0, 0);
         error_code = EPUBSUB_NOT_AUTH;
         statusCode = 200;
         goto errorexit;
      }
      if (!g_email || !g_nick) {
         PROG_ERR ("Corrupt session for [%s]\n", g_session_id);
         xcgi_header_cookie_clear (FIELD_STR_SESSION);
         xcgi_header_cookie_set (FIELD_STR_SESSION, "", 0, 0);
         error_code = EPUBSUB_NOT_AUTH;
         statusCode = 200;
         goto errorexit;
      }
      if (!(g_perms = perms_get (g_email, xcgi_path_info[0]))) {
         PROG_ERR ("No permissions granted to user [%s] for [%s]\n",
                    g_email, xcgi_path_info[0]);
         goto errorexit;
      }
   }

   if (!(endpoint_valid_params (endpoint))) {
      PROG_ERR ("Endpoint [%s] missing required parameters\n",
                xcgi_path_info[0]);
      error_code = EPUBSUB_MISSING_PARAMS;
      statusCode = 200;
      goto errorexit;
   }

   if (!(endpoint (jfields, &error_code, &statusCode)))
      goto errorexit;


   ret = EXIT_SUCCESS;

errorexit:

   error_message = pubsub_error_msg (error_code);

   if (!(set_ifield (jfields, FIELD_STR_ERROR_CODE, error_code))) {
      PROG_ERR ("Failed setting the error-code field\n");
      return EXIT_FAILURE;
   }

   if (!(set_sfield (jfields, FIELD_STR_ERROR_MESSAGE, error_message))) {
      PROG_ERR ("Failed setting the error-message field\n");
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

   free (g_email);
   free (g_nick);

   return ret;
}

