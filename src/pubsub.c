
#include <stdio.h>
#include <stdlib.h>

#include "xcgi.h"

int main (void)
{
   int ret = EXIT_FAILURE;

   if (!(xcgi_init ())) {
      fprintf (stderr, "Failed to initialise the library\n");
      goto errorexit;
   }

   printf ("Path info [%zu]:\n", xcgi_path_info_count ());
   for (size_t i=0; xcgi_path_info[i]; i++) {
      printf ("   [%s]\n", xcgi_path_info[i]);
   }
   printf ("/Path info\n");

   xcgi_headers_write ();

   printf ("--");
   while (!feof (xcgi_stdin) && !ferror (xcgi_stdin)) {
      int c = fgetc (xcgi_stdin);
      if (c!=EOF) {
         fputc (c, stdout);
      }
   }
   printf ("--\n");

   ret = EXIT_SUCCESS;

errorexit:

   xcgi_shutdown ();

   return ret;
}

