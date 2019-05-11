
#include <stdio.h>
#include <stdlib.h>

#include "xcgi.h"

int main (int argc, char ** argv)
{
   int ret = EXIT_FAILURE;
   printf ("Fake a cgi execution.\n");

   if (argc < 2) {
      fprintf (stderr, "Specify a filename to load the cgi env from\n");
      goto errorexit;
   }

   if (!(xcgi_load (argv[1]))) {
      fprintf (stderr, "Failed to load cgi environment from [%s]\n", argv[1]);
      goto errorexit;
   }

   ret = EXIT_SUCCESS;

errorexit:

   return ret;
}

