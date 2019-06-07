
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "xcgi_json.h"

// A strchr() implementation that respects escaped characters.
static const char *lstrchr (const char *haystack, char needle)
{
   const char *tmp = haystack;
   while ((tmp = strchr (tmp, needle))) {
      if (tmp == haystack || tmp[-1] != '\\')
         return tmp;
   }
   return NULL;
}

// Find the position of a json field named 'field' (no quotes) within the
// string 'start'. Returns a pointer to the start of the field if the
// field exists, or NULL if the field was not found in the string.
static const char *find_field (const char *start, const char *field)
{
   const char *tmp = start;
   size_t flen = strlen (field);

   while ((tmp = lstrchr (tmp, '"'))) {
      if ((memcmp (++tmp, field, flen))==0) {
         const char *end = lstrchr (tmp, '"');
         if (!end)
            continue;

         size_t tmplen = end - tmp;

         if (tmplen < (flen + 2)) {
            if (tmp[flen] == '"') {
               const char *end = &tmp[flen + 1];
               while (*end && isspace (*end))
                  end++;
               return *end == ':' ? &tmp[0] : NULL;
            }
         }
      }
   }
   return NULL;
}

// Returns a pointer to the value part of a json field. Note that field
// values which are strings in json (they have quotes) are returned as is,
// and no validation on the field syntax is performed.
static const char *find_value (const char *field)
{
   const char *ret = lstrchr (field, '"');
   if (!ret)
      return NULL;

   if (!(ret = lstrchr (ret, ':')))
      return NULL;

   ret++;

   while (*ret && isspace (*ret))
      ret++;

   return ret;
}

const char *xcgi_jsonv (const char *json_src, const char *field, va_list ap)
{
   const char *begin = json_src;

   while (begin && field) {
      if (!(begin = find_field (begin, field)))
         break;

      if (!(field = va_arg (ap, char *)))
         return find_value (begin);
   }

   return NULL;
}

const char *xcgi_json (const char *json_src, const char *field, ...)
{
   va_list ap;
   va_start (ap, field);

   const char *ret = xcgi_jsonv (json_src, field, ap);

   va_end (ap);
   return ret;
}

static const char *find_closing (const char *s, char close_char)
{
   size_t nlevels = 1;
   const char *ret = s;
   if (!ret || *ret=='\\')
      return NULL;

   char quote_char = 0;
   while (*ret++ && nlevels) {
      if (quote_char) {
         if (*ret == quote_char)
            quote_char = 0;
         continue;
      }

      if (*ret == '"') {
         quote_char = *ret;
         continue;
      }

      if (*ret==s[0]) {
         nlevels++;
         continue;
      }

      if ((ret[-1] != '\\') && (*ret == close_char))
         nlevels--;
   }

   return *ret ? ret : NULL;
}

size_t xcgi_json_length (const char *json_element)
{
   const char *tmp = NULL;

   if (!json_element || !json_element[0])
      return 0;

   switch (json_element[0]) {
      case '"':   tmp = lstrchr (&json_element[1], '"');
                  break;

      case '{':   tmp = find_closing (json_element, '}');
                  break;

      case '[':   tmp = find_closing (json_element, ']');
                  break;

      default:    tmp = &json_element[1];
                  while (*tmp && *tmp != ','
                              && *tmp != ']'
                              && *tmp != '}'
                              && !isspace (*tmp))
                     tmp++;
                  if (*tmp)
                     tmp--;
                  break;
   }

   tmp++;
   return tmp ? tmp - json_element : 0;
}
