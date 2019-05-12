
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "xcgi.h"

void sigh (int n)
{
   fprintf (stderr, "Rxed signal %i, ignoring\n", n);
}

int main (int argc, char ** argv)
{
   int ret = EXIT_FAILURE;
   static char input[1024 * 16];
   FILE *childf = NULL;
   char *child_cmd = NULL;

   printf ("Fake a cgi execution.\n");

   signal (SIGPIPE, sigh);

   if (argc < 2) {
      fprintf (stderr, "Specify a filename to load the cgi env from\n");
      goto errorexit;
   }

   if (argc < 3) {
      fprintf (stderr, "Specify the cgi executable to run\n");
      goto errorexit;
   }

   if (!(xcgi_init())) {
      fprintf (stderr, "Failed to initialise the xcgi library\n");
      goto errorexit;
   }

   if (!(xcgi_load (argv[1]))) {
      fprintf (stderr, "Failed to load cgi environment from [%s]\n", argv[1]);
      goto errorexit;
   }

   if (!(ds_str_append (&child_cmd,
                        "valgrind ",
                        "--leak-check=full ",
                        "--track-origins=yes ",
                        "--show-leak-kinds=all ",
                        argv[2], NULL))) {
      fprintf (stderr, "Failed to construct command\n");
      goto errorexit;
   }

   if (!(childf = popen (child_cmd, "w"))) {
      fprintf (stderr, "Failed to execute [%s]\n", argv[2]);
      goto errorexit;
   }

   while (!feof (xcgi_stdin) && !ferror (xcgi_stdin)) {
      size_t nbytes = fread (input, 1, sizeof input, xcgi_stdin);
      size_t written = fwrite (input, 1, nbytes, childf);
      if (nbytes != written) {
         fprintf (stderr, "Faker: Wrote only [%zu/%zu] bytes to child\n",
                  written, nbytes);
         goto errorexit;
      }
   }

   ret = EXIT_SUCCESS;

errorexit:

   free (child_cmd);

   xcgi_shutdown ();
   if (childf)
      pclose (childf);

   return ret;
}

