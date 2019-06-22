
#ifndef H_XCGI_AUTH
#define H_XCGI_AUTH

#include <stdbool.h>
#include <stdint.h>

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
 * Note that this module does not perform any authorisation of the caller
 * before executing any database action. The caller must use the
 * permissions functions in this module to ensure that the user is
 * properly authorised.
 */

#ifdef __cplusplus
extern "C" {
#endif

   // dbstring:   When dbtype==sqlite, this names a file for the sqlite
   //             database. When dbtype==postgres this must be a valid
   //             postgres connection string.
   // dbtype:     Either 'xcgi_db_SQLITE' or 'xcgi_db_POSTGRES'.
   bool xcgi_auth_createdb (const char *dbstring, xcgi_db_dbtype_t dbtype);

   ///////////////////////////////////////////////////////////////////////

   // Authenticates the session specified against the user in the
   // database, returns the user info in email, nick and id if the
   // specified session is in the database and valid.
   //
   // The caller must free the strings in email_dst and nick_dst
   // regardless of the return value. This function will first free the
   // strings stored at that location before populating them with new
   // values.
   bool xcgi_auth_user_session (xcgi_db_t *db,
                                const char   session_id[65],
                                char       **email_dst,
                                char       **nick_dst,
                                uint64_t    *id_dst);

   // Creates a new session, returns the session ID in the sess_id_dst
   // array. Returns true on success and false on error.
   bool xcgi_auth_user_login (xcgi_db_t *db,
                              const char *email, const char *password,
                              char sess_id_dst[65]);

   // Invalidates the session for user specified by email.
   void xcgi_auth_user_logout (xcgi_db_t *db,
                               const char *email);

   ///////////////////////////////////////////////////////////////////////

   // Create a new user, returns the user ID.
   uint64_t xcgi_auth_user_create (xcgi_db_t *db,
                                   const char *email,
                                   const char *nick,
                                   const char *password);

   // Removes a user from the database.
   bool xcgi_auth_user_rm (xcgi_db_t *db,
         const char *email);

   // Gets the user account information for the specified user. Returns
   // true on success and false on failure. The string destinations nick
   // and session must be freed by the caller regardless of the return
   // value. If they are not-NULL on entry to this function, this function
   // will free them before allocation storage for the output parameters.
   bool xcgi_auth_user_info (xcgi_db_t *db,
                             const char *email,
                             uint64_t   *id_dst,
                             char      **nick_dst,
                             char      session_dst[65]);

   // Updates the non-NULL parameters in the database. Returns true on
   // success and false on error. Uses the old_email parameter to find the
   // record to update.
   bool xcgi_auth_user_mod (xcgi_db_t *db,
                            const char *old_email,
                            const char *new_email,
                            const char *nick,
                            const char *password);


   ///////////////////////////////////////////////////////////////////////

   // Create a new group, returns the group ID.
   uint64_t xcgi_auth_group_create (xcgi_db_t *db,
                                    const char *name,
                                    const char *description);

   // Removes a group from the database.
   bool xcgi_auth_group_rm (xcgi_db_t *db,
                            const char *name);

   // Gets the group account information for the specified group. Returns
   // true on success and false on failure. The string destination
   // 'description' must be freed by the caller regardless of the return
   // value. If it is not NULL on entry to this function, this function will
   // free it before allocation storage for the description.
   bool xcgi_auth_group_info (xcgi_db_t *db,
                              const char *name,
                              uint64_t   *id_dst,
                              char      **description);

   // Updates the non-NULL parameters in the database. Returns true on
   // success and false on error. Uses the name parameter to find the
   // record to update.
   bool xcgi_auth_group_mod (xcgi_db_t *db,
                             const char *name,
                             const char *description);

   ///////////////////////////////////////////////////////////////////////

   // Add the specified user to the specified group. Returns true on
   // success and false on failure.
   bool xcgi_auth_group_adduser (xcgi_db_t *db,
                                 const char *name, const char *email);

   // Remove the specified user from the specified group. Returns true on
   // success and false on failure.
   bool xcgi_auth_group_rmuser (xcgi_db_t *db,
                                const char *name, const char *email);


   ///////////////////////////////////////////////////////////////////////

   // Generates a list of user records from the database that match the
   // email_pattern or the nick_pattern. If either pattern is NULL it is
   // ignored.
   //
   // The results are returned in three arrays. The caller must free all
   // three arrays.
   //
   // The number of results is stored in nitems.
   //
   // The emails and nicks string arrays are allocated to store the number
   // of strings in the results set for the respective columns (emails and
   // nicks) and each array is terminated with a NULL pointer. The caller
   // must free both the array as well as each element of the array, for
   // both the arrays.
   //
   // The ids array stores each ID and the entire array (but not each
   // element) must be freed by the caller.
   bool xcgi_auth_user_find (xcgi_db_t *db,
                             const char *email_pattern,
                             const char *nick_pattern,
                             uint64_t   *nitems,
                             char     ***emails,
                             char     ***nicks,
                             uint64_t  **ids);

   // Generates a list of group records from the database that match the
   // email_pattern or the nick_pattern. If either pattern is NULL it is
   // ignored.
   //
   // The results are returned in three arrays. The caller must free all
   // three arrays.
   //
   // The number of results is stored in nitems.
   //
   // The names and descriptions string arrays are allocated to store the
   // number of strings in the results set for the respective columns (names
   // and descriptions) and each array is terminated with a NULL pointer.
   // The caller must free both the array as well as each element of the
   // array, for both the arrays.
   //
   // The ids array stores each ID and the entire array (but not each
   // element) must be freed by the caller.
   bool xcgi_auth_group_find (xcgi_db_t *db,
                              const char *name_pattern,
                              uint64_t   *nitems,
                              char     ***names,
                              char     ***descriptions,
                              uint64_t  **ids);

   // Generates a list of records containing all users who are members of
   // the specified group.
   //
   // The results are returned in three arrays. The caller must free all
   // three arrays.
   //
   // The number of results is stored in nitems.
   //
   // The emails and nicks string arrays are allocated to store the number
   // of strings in the results set for the respective columns (emails and
   // nicks) and each array is terminated with a NULL pointer. The caller
   // must free both the array as well as each element of the array, for
   // both the arrays.
   //
   // The ids array stores each ID and the entire array (but not each
   // element) must be freed by the caller.
   bool xcgi_auth_group_members (xcgi_db_t *db,
                                 const char *name,
                                 uint64_t   *nitems,
                                 char     ***emails,
                                 char     ***nicks,
                                 uint64_t  **ids);

   ///////////////////////////////////////////////////////////////////////

   // Grant the specified permissions to the specified user for the
   // specified resource. Returns true on success and false on failure.
   bool xcgi_auth_perms_grant_user (xcgi_db_t *db,
                                    uint64_t perms, const char *resource,
                                    const char *email);

   // Revoke the specified permissions to the specified user for the
   // specified resource. Returns true on success and false on failure.
   bool xcgi_auth_perms_revoke_user (xcgi_db_t *db,
                                    uint64_t perms, const char *resource,
                                    const char *email);

   // Retrieve the group's permissions for the specified resource and
   // stores in in 'perms'. Returns true on success and false on failure.
   bool xcgi_auth_perms_get_group (xcgi_db_t *db,
                                   uint64_t *perms, const char *resource,
                                   const char *name);

   // Retrieve the user's permissions for the specified resource and
   // stores in in 'perms'. Returns true on success and false on failure.
   bool xcgi_auth_perms_get_user (xcgi_db_t *db,
                                  uint64_t *perms, const char *resource,
                                  const char *email);

   // Retrieve the effective permissions of the specified user for the
   // specified resource and stores it in 'perms'. Returns true on success
   // and false on failure.
   //
   // The effective permissions is the bitwise 'OR' of all the permission
   // bits of the user as well as all of the groups that the user belongs
   // to.
   bool xcgi_auth_perms_get_all (xcgi_db_t *db,
                                 uint64_t *perms, const char *resource,
                                 const char *email);
#ifdef __cplusplus
};
#endif


#endif
