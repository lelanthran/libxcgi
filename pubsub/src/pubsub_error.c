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
      { EPUBSUB_SUCCESS,         "Success"                              },
      { EPUBSUB_UNIMPLEMENTED,   "Function not implemented"             },
      { EPUBSUB_INTERNAL_ERROR,  "Internal program error"               },
      { EPUBSUB_ENDPOINT,        "Unknown endpoint"                     },
      { EPUBSUB_MISSING_PARAMS,  "Missing parameters"                   },
      { EPUBSUB_BAD_PARAMS,      "Bad parameters"                       },
      { EPUBSUB_NOT_AUTH,        "Not authenticated"                    },
      { EPUBSUB_AUTH_FAILURE,    "Authentication failure"               },
      { EPUBSUB_PERM_DENIED,     "Permission denied"                    },
      { EPUBSUB_RESOURCE_EXISTS, "Specific resource already exists"     },
   };

   for (size_t i=0; i<sizeof errs/sizeof errs[0]; i++) {
      if (errs[i].code==errorcode)
         return errs[i].msg;
   }

   snprintf (ret, sizeof ret, "Unknown Error [%i]", errorcode);

   return ret;
}
