
#ifndef H_XCGI_JSON
#define H_XCGI_JSON

#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

   /* ********************************************************************
    * Searches the specified JSON string and returns the location of the
    * value named by the fields {field1, ... fieldN}, where the fields are
    * specified as variadic arguments to this function.
    *
    * Note that the returned value is a pointer into the original JSON
    * source string, and thus is not NULL-terminated. Use the
    * xcgi_json_length() function to determine the length of the value
    * being pointed to.
    *
    * EXAMPLE:
    * Given the following JSON input:
    * {  "some_fields": 3.14598,
    *    "one" :
    *       { "two" : [ "ONE": 1, "TWO": 2, "THREE": 3, "FOUR": 4 ] },
    *    "some_other_fields": "More data"
    * }
    *
    * We find the value of element 'one.two.THREE' in JSON tree 'src':
    *       const char *val = xcgi_json (src, "one", "two", "three", NULL);
    *
    * The value 'val' will now contain a pointer to the substring:
    *    '3,  "FOUR": 4 ] },\n      "some_other_fields": "More data"\n}'
    *
    * The exact substring consisting of only the value and not the rest of
    * the JSON source ('3', instead of '3, "FOUR"...') can be copied using
    * the xcgi_json_length() function, which returns '1' for this
    * example.
    * }
    */
   const char *xcgi_jsonv (const char *json_src, const char *field, va_list ap);
   const char *xcgi_json (const char *json_src, const char *field, ...);

   /* ********************************************************************
    * Returns the length of the string as interpreted as a JSON value. See
    * above description of xcgi_json() for an example.
    */
   size_t xcgi_json_length (const char *json_element);

   // TODO: Implement this when json[index] functionality is needed
   const char *json_index (const char *json_src, size_t index);


#ifdef __cplusplus
};
#endif


#endif

