#include <stdlib.h>
#include <stdio.h>

#include "pubsub_error.h"

const char *pubsub_error_msg (int errorcode)
{
   static char ret[25];

   static const struct {
      int code;
      const char *msg;
   } errs[] = {

   };

   for (size_t i=0; i<sizeof errs/sizeof errs[0]; i++) {
      if (errs[i].code==errorcode)
         return errs[i].msg;
   }

   snprintf (ret, "Unknown Error [%i]", errorcode);

   return ret;
}
