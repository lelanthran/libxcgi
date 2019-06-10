
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
   if (!hm || !name || !value)
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

int main (void)
{
   int ret = EXIT_FAILURE;
   int errorCode = 0;
   const char *errorMessage = "Success";
   ds_hmap_t *jfields = NULL;

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

   if (!(xcgi_HTTP_COOKIE[0])) {
      errorCode = EPUBSUB_AUTH;
   }


   printf ("Path info [%zu]:\n", xcgi_path_info_count ());
   for (size_t i=0; xcgi_path_info[i]; i++) {
      printf ("   [%s]\n", xcgi_path_info[i]);
   }
   printf ("/Path info\n");

   xcgi_headers_write ();

   while (!feof (xcgi_stdin) && !ferror (xcgi_stdin)) {
      int c = fgetc (xcgi_stdin);
      if (c!=EOF) {
         fputc (c, stdout);
      }
   }

   ret = EXIT_SUCCESS;

errorexit:

   if (errorCode) {
      errorMessage = pubsub_error_msg (errorCode);
   }

   if (!(set_ifield (jfields, "errorCode", errorCode))) {
      fprintf (stderr, "Failed setting the errorCode field\n");
      return EXIT_FAILURE;
   }

   if (!(set_sfield (jfields, "errorMessage", errorMessage))) {
      fprintf (stderr, "Failed setting the errorMessage field\n");
      return EXIT_FAILURE;
   }

   print_json (jfields);
   free_json (jfields);

   xcgi_shutdown ();

   return ret;
}

