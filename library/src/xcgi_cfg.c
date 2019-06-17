#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdarg.h>


#include "xcgi_cfg.h"

#include "ds_str.h"

bool xcgi_cfg_fwrite (char **xcgi_cfg, FILE *outf)
{
   if (!xcgi_cfg || !outf)
      return false;

   for (size_t i=0; xcgi_cfg[i]; i++)
      if ((fprintf (outf, "%s\n", xcgi_cfg[i])) < 0)
         return false;

   return true;
}

bool xcgi_cfg_save (char **xcgi_cfg, const char *fpath, ...)
{
   bool ret = false;
   char *fname = NULL;
   FILE *outfile = NULL;
   va_list ap;

   va_start (ap, fpath);
   if (!(fname = ds_str_vcat (fpath, ap))) {
      fprintf (stderr, "%s: Failed to ds_str_vcat [%s]\n", __func__, fpath);
      goto errorexit;
   }

   if (!(outfile = fopen (fname, "w"))) {
      fprintf (stderr, "%s: Failed to open [%s]\n", __func__, fname);
      goto errorexit;
   }

   ret = xcgi_cfg_fwrite (xcgi_cfg, outfile);

errorexit:

   va_end (ap);

   if (outfile)
      fclose (outfile);

   free (fname);

   return ret;
}


char **xcgi_cfg_fread (FILE *inf)
{
   bool error = true;
   char **ret = NULL;
   static char line[4096];

   if (!inf)
      return NULL;

   while (!feof (inf) && !ferror (inf) && fgets (line, sizeof line -1, inf)) {

      char *tmp = strchr (line, '#');
      if (tmp)
         *tmp = 0;

      if ((tmp = strchr (line, '\n')))
        *tmp = 0;

      tmp = strchr (line, '=');
      if (!line[0] || !tmp)
         continue;

      *tmp++ = 0;

      char *name = &line[strlen (line)];
      char *value = &tmp[strlen (tmp)];

      if (!*name) name--;
      if (!*value) value--;

      while (name!=line && isspace (*name))
        *name-- = 0;

      while (value!=line && isspace (*value))
        *value-- = 0;

      name = line;
      value = tmp;

      while (*name && isspace (*name))
        name++;
      while (*value && isspace (*value))
        value++;

      if (!(xcgi_cfg_set (&ret, name, value)))
         goto errorexit;
   }

   error = false;

errorexit:
   if (error) {
      xcgi_cfg_del (ret);
      ret = NULL;
   }

   return ret;
}

char **xcgi_cfg_load (const char *fpath, ...)
{
   char **ret = NULL;
   char *fname = NULL;
   FILE *infile = NULL;
   va_list ap;

   va_start (ap, fpath);
   if (!(fname = ds_str_vcat (fpath, ap))) {
      fprintf (stderr, "%s: Failed to ds_str_vcat [%s]\n", __func__, fpath);
      goto errorexit;
   }

   if (!(infile = fopen (fname, "r"))) {
      fprintf (stderr, "%s: Failed to open [%s]\n", __func__, fname);
      goto errorexit;
   }

   ret = xcgi_cfg_fread (infile);

errorexit:

   va_end (ap);

   if (infile)
      fclose (infile);

   free (fname);

   return ret;
}

void xcgi_cfg_del (char **xcgi_cfg)
{
   if (!xcgi_cfg)
      return;

   for (size_t i=0; xcgi_cfg[i]; i++) {
      free (xcgi_cfg[i]);
   }
   free (xcgi_cfg);
}

static size_t xcgi_cfg_find (const char **xcgi_cfg, const char *name)
{
   if (!xcgi_cfg || !name)
      return (size_t)-1;

   size_t name_len = strlen (name);
   for (size_t i=0; xcgi_cfg[i]; i++) {
      if ((memcmp (xcgi_cfg[i], name, name_len))==0)
         return i;
   }

   return (size_t)-1;
}

bool xcgi_cfg_set (char ***dst, const char *name, const char *value)
{
   bool error = true;

   if (!name)
      return false;

   if (!value)
      value = "";

   size_t len = 0;
   for (size_t i=0; dst && (*dst) && (*dst)[i]; i++)
      len++;

   size_t element = xcgi_cfg_find ((const char **)(*dst), name);

   if (element == (size_t)-1) {
      char **array = NULL;
      if (!(array = realloc ((*dst), (sizeof ((*dst))) * (len + 2))))
         goto errorexit;
      array[len + 1] = NULL;
      array[len] = NULL;
      (*dst) = array;
      element = len;
   }

   free ((*dst)[element]);

   size_t element_len = strlen (name) + 1 + strlen (value) + 1;

   if (!((*dst)[element] = malloc (element_len)))
      goto errorexit;

   strcpy ((*dst)[element], name);
   strcat ((*dst)[element], "=");
   strcat ((*dst)[element], value);

   error = false;

errorexit:
   return !error;
}

const char *xcgi_cfg_get (char **xcgi_cfg, const char *name)
{
   size_t index = xcgi_cfg_find ((const char **)xcgi_cfg, name);
   if (index == (size_t)-1)
      return "";

   char *value = strchr (xcgi_cfg[index], '=');

   return value ? &value[1] : "";
}

#define TYPE_INT     (1)
#define TYPE_FLT     (2)
static bool xcgi_cfg_get_typed_value (uint8_t type, char **xcgi_cfg,
                                 const char *name, void *dst)
{
   const char *fmts = NULL;
   switch (type) {
      case TYPE_INT:    fmts = "%" PRIi64;   break;
      case TYPE_FLT:    fmts = "%lf";         break;
   }
   if (!fmts)
      return false;

   const char *svalue = xcgi_cfg_get (xcgi_cfg, name);
   return (sscanf (svalue, fmts, dst)) != 1 ? false : true;
}

bool xcgi_cfg_get_int (char **xcgi_cfg, const char *name, int64_t *dst)
{
   return xcgi_cfg_get_typed_value (TYPE_INT, xcgi_cfg, name, dst);
}

bool xcgi_cfg_get_flt (char **xcgi_cfg, const char *name, double *dst)
{
   return xcgi_cfg_get_typed_value (TYPE_FLT, xcgi_cfg, name, dst);
}


