
#ifndef H_XCGI_CFG
#define H_XCGI_CFG

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

   // Read the configuration from a file and return it. NULL is returned
   // on error.
   char **xcgi_cfg_fread (FILE *inf);
   char **xcgi_cfg_load (const char *fpath, ...);

   // Write the configuration in 'xcgi_cfg' to the specified file 'outf'. On
   // success true is returned and on any error the function aborts the
   // process and returns false.
   bool xcgi_cfg_fwrite (char **xcgi_cfg, FILE *outf);
   bool xcgi_cfg_save (char **xcgi_cfg, const char *fpath, ...);

   // Delete the configuration data stored in 'xcgi_cfg'
   void xcgi_cfg_del (char **xcgi_cfg);

   // Set the configuration 'name' to value 'value'. The argument 'dst'
   // must point to a realloc()able 'char **'. If the array being
   // pointed to by 'dst' is NULL then a new array is allocated and
   // returned and must be used in subsequent get/set operations. At some
   // point the caller must call xcgi_cfg_del() on the array pointed to by
   // 'dst'.
   //
   // On success true is returned, on failure false is returned and the
   // array pointed to by 'dst' remains valid.
   bool xcgi_cfg_set (char ***dst, const char *name, const char *value);

   // Returns the value for the configuration 'name'. If the 'name' does
   // not exist then an empty string is returned.
   const char *xcgi_cfg_get (char **xcgi_cfg, const char *name);

   // Convenience functions to read a configuration value as a particular
   // type. If the value is both found AND converted to the specified type
   // then true is returned and the 'dst' is populated with the value.
   //
   // If the value is not found, or if the value could not be converted to
   // the requested type then false is returned and the contents of the
   // 'dst' is indeterminate.
   bool xcgi_cfg_get_int (char **xcgi_cfg, const char *name, int64_t *dst);
   bool xcgi_cfg_get_flt (char **xcgi_cfg, const char *name, double *dst);


#ifdef __cplusplus
};
#endif

#endif

