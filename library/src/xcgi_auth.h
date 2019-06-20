
#ifndef H_XCGI_AUTH
#define H_XCGI_AUTH

#include <stdbool.h>

#include "xcgi_db.h"

/* ********************************************************************
 * This is the authentication and authorization module. This module provides
 * a minimal session management and access control system using the
 * specified database (see xcgi.h).
 *
 * This module allows the caller to:
 *    1. Create new users and groups,
 *    2. Modify users (their password, email, etc),
 *    3. Add and remove users to and from groups,
 *    4. Authenticate users and create a timed session for future
 *       requests,
 *    5. Add and remove user/group access to named resources,
 *    6. Specify different levels of access for both resources and
 *       operations.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

   // dbstring:   When dbtype==sqlite, this names a file for the sqlite
   //             database. When dbtype==postgres this must be a valid
   //             postgres connection string.
   // dbtype:     Either 'xcgi_db_SQLITE' or 'xcgi_db_POSTGRES'.
   bool xcgi_auth_createdb (const char *dbstring, xcgi_db_dbtype_t dbtype);


#ifdef __cplusplus
};
#endif


#endif
