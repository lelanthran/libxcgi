
#include <stdio.h>
#include <stdlib.h>

#include "xcgi_json.h"

#define JSOURCE \
  "{\n"\
  "   \"field1\":   \"value1\",\n"\
  "   \"field2\":   \"value2\",\n"\
  "   \"field3\":   {\n"\
  "            \"f3.1\":   123,\n"\
  "            \"f3.2\":   3.14159,\n"\
  "            \"f3.3\":   {\n"\
  "                  \"f3.3.1\":   \"v3.3.1\",\n"\
  "            }\n"\
  "            \"f3.4\":   \"s 3.4\",\n"\
  "            \"f3.5\":   null,\n"\
  "   }\n"\
  "   \"field4\":   [ \"a0\", \"a1\", \"a2\", \"a3\" ],\n"\
  "   \"field5\":   \"value5\"\n"\
  "};\n"

#define FIELD  "field3", "f3.3", "f3.3.1"
int main (void)
{
   int ret = EXIT_FAILURE;
   printf ("Testing xcgi_json_find\n%s\n", JSOURCE);

   const char *needle1 = xcgi_json_find (JSOURCE, FIELD, NULL);
   if (!needle1) {
      fprintf (stderr, "Failed to find the field\n");
      goto errorexit;
   }
   size_t needle1_len = xcgi_json_length (needle1);

   printf ("Element : [%s]\n", needle1);
   printf ("Element length: %zu\n", needle1_len);
   printf ("======================================\n\n");

   needle1 = xcgi_json_find (JSOURCE, "field3", NULL);
   needle1_len = xcgi_json_length (needle1);
   printf ("Element : [%s]\n", needle1);
   printf ("Element length: %zu\n", needle1_len);
   printf ("======================================\n\n");

   needle1 = xcgi_json_find (JSOURCE, "field3", "f3.1", NULL);
   needle1_len = xcgi_json_length (needle1);
   printf ("Element : [%s]\n", needle1);
   printf ("Element length: %zu\n", needle1_len);
   printf ("======================================\n\n");

   needle1 = xcgi_json_find (JSOURCE, "field3", "f3.2", NULL);
   needle1_len = xcgi_json_length (needle1);
   printf ("Element : [%s]\n", needle1);
   printf ("Element length: %zu\n", needle1_len);
   printf ("======================================\n\n");

   needle1 = xcgi_json_find (JSOURCE, "field3", "f3.4", NULL);
   needle1_len = xcgi_json_length (needle1);
   printf ("Element : [%s]\n", needle1);
   printf ("Element length: %zu\n", needle1_len);
   printf ("======================================\n\n");

   needle1 = xcgi_json_find (JSOURCE, "field3", "f3.5", NULL);
   needle1_len = xcgi_json_length (needle1);
   printf ("Element : [%s]\n", needle1);
   printf ("Element length: %zu\n", needle1_len);
   printf ("======================================\n\n");

   // TODO: This is an array; test that accessing via index is possible.
   needle1 = xcgi_json_find (JSOURCE, "field4", NULL);
   needle1_len = xcgi_json_length (needle1);
   printf ("Element : [%s]\n", needle1);
   printf ("Element length: %zu\n", needle1_len);
   printf ("======================================\n\n");

   ret = EXIT_SUCCESS;

errorexit:

   return ret;
}

