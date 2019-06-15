
#ifndef H_XCGI_DB
#define H_XCGI_DB

/* This is an agnostic interface to the sql backend. While it is possible
 * for the caller to use SQL statements that are specific to a single SQL
 * platform it is not advised as this means that different statements
 * would be needed for different backends.
 *
 * There are two datatypes: a connection object and a resultset object.
 * Both must be closed after use. Only parameterised statements should be
 * used, with the parameters defined as #1, #2, etc. within the statement.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

typedef struct xcgi_db_t xcgi_db_t;
typedef struct xcgi_db_res_t xcgi_db_res_t;

typedef enum {
   xcgi_db_UNKNOWN = 0,
   xcgi_db_SQLITE,
   xcgi_db_POSTGRES,
} xcgi_db_dbtype_t;

typedef enum {
   xcgi_db_col_UNKNOWN = 0,  // Mark end of list
   xcgi_db_col_INT32,        // int32_t
   xcgi_db_col_INT64,        // int64_t
   xcgi_db_col_UINT32,       // uint32_t
   xcgi_db_col_UINT64,       // uint64_t
   xcgi_db_col_DATETIME,     // int64_t
   xcgi_db_col_TEXT,         // char *
   xcgi_db_col_BLOB,
   xcgi_db_col_NULL
} xcgi_db_coltype_t;

#ifdef __cplusplus
extern "C" {
#endif

   // Does nothing when type!=sqlite, otherwise creates a new sqlite3 db
   bool xcgi_db_create (const char *dbname, xcgi_db_dbtype_t type);

   // Open a connection to the database, using the type specified. Returns
   // NULL on error.
   xcgi_db_t *xcgi_db_open (const char *dbname, xcgi_db_dbtype_t type);

   // Close the connection to the database. All resources will be freed
   // except existing xcgi_db_res_t objects.
   void xcgi_db_close (xcgi_db_t *db);

   // Return a description of the last error that occurred. The caller
   // must not free this string. NULL will be returned if no
   // error message is available.

   const char *xcgi_db_lasterr (xcgi_db_t *db);
   const char *xcgi_db_res_lasterr (xcgi_db_res_t *res);

   // Clear the last error messages stored.
   void xcgi_db_clearerr (xcgi_db_t *db);
   void xcgi_db_res_clearerr (xcgi_db_res_t *res);

   // Get the number of rows affected by the last operation executed on
   // this database. Only applies to insert/update/delete operations.
   uint64_t xcgi_db_count_changes (xcgi_db_t *db);

   // Get the last inserted ID for this result if query was an insert
   // operation. If query was NOT an insert operation then the return
   // value is not defined.
   uint64_t xcgi_db_res_last_id (xcgi_db_res_t *res);

   // Get the number of columns in this result-set.
   uint32_t xcgi_db_res_num_columns (xcgi_db_res_t *res);

   // Get the column name in this result-set. The return value is an array
   // of strings (char pointers) terminated with a NULL pointer, each of
   // which the caller must free. The caller must also free the array
   // itself.
   //
   // On error NULL is returned.
   char **xcgi_db_res_column_names (xcgi_db_res_t *res);

   // Uses the specified parameterised querystring and tuples in the
   // variadic arguments to construct a query that is executed on the
   // database. See explanation of tuple format in scan_columns below.
   //
   // Parameters in querystring are of the format "#n" where n is the
   // number of the parameter.
   //
   // Returns a result object (that may be empty if the query returned no
   // results) on success or NULL on error.
   xcgi_db_res_t *xcgi_db_exec (xcgi_db_t *db, const char *query, ...);
   xcgi_db_res_t *xcgi_db_execv (xcgi_db_t *db, const char *query, va_list *ap);

   // Same as the exec functions above, with the difference being that
   // results are ignored and only a success or failure is indicated with
   // true or false respectively.
   bool xcgi_db_exec_ignore (xcgi_db_t *db, const char *query, ...);
   bool xcgi_db_exec_ignorev (xcgi_db_t *db, const char *query, va_list *ap);

   // Executes a batch of statements and returns no results. Multiple
   // statements can be specified, ending with a NULL pointer. Each
   // statement may be composed of multiple statements itself, with each
   // statement separated with a semicolon.
   //
   // Statements must not have parameters. Execution stops on the first
   // error encountered.
   //
   // Return value is true if no errors were encountered and false if
   // any errors were encountered. The lasterr() function will return
   // the error message.
   //
   // This is a dangerous function to use on unknown input, so use with
   // care.
   bool xcgi_db_batch (xcgi_db_t *db, ...);

   // This is similar to the xcgi_db_batch() function, with the exception
   // being that the sql statements are all read from the FILE * passed in.
   //
   // NOTE: No parameterisation is possible, hence no parameters can be
   // passed in.
   //
   // Other than the exception listed above all the other behaviour,
   // including return value and error messages, remain the same as
   // xcgi_db_batch().
   //
   // This is a dangerous function to use on unknown input, so use with
   // care.
   bool xcgi_db_batchfile (xcgi_db_t *db, FILE *inf);

   // Returns 0 if no rows are available, 1 if a row is available and -1
   // if an error occurred.
   int xcgi_db_res_step (xcgi_db_res_t *res);

   // Scans in all the columns in the current row. Returns the number of
   // columns scanned in.
   //
   // Each variadic argument is a tuple consisting of the column type (see
   // enum above) and a pointer to a destination to store the data read.
   // In the case of TEXT columns the memory to store a string is malloced
   // by this function and must be freed by the caller.
   //
   // In the case of a BLOB field an extra variadic argument of type
   // (uint32_t *) is used to store the length of the blob.
   //
   // On success the number of fields scanned and stored is returned
   // (which may be zero if the query had no results). On error
   // (uint32_t)-1 is returned.
   uint32_t xcgi_db_scan_columns (xcgi_db_res_t *res, ...);
   uint32_t xcgi_db_scan_columnsv (xcgi_db_res_t *res, va_list *ap);

   // This is a bit of a tricky function. It executes the given query
   // using all the arguments in (...) as pairs of {type,value} until it
   // reaches type_UNKNOWN. It then fetches the results into tuples
   // *after* type_UNKNOWN using the remainder of the arguments on the
   // stack.
   //
   // In short, it first executes xcgi_db_exec(...), and then
   // xcgi_db_scan_columns(...) - the caller must take care to provide the
   // correct number of arguments or else the stack will be damaged.
   //
   // On any error prior to the fetching of the results (uint32_t)-1 is
   // returned. On any error during the fetching of the results processing
   // stops. The return value indicates how many columns were successfully
   // read in and stored, which may be less than expected if errors
   // occurred during processing.
   uint32_t xcgi_db_exec_and_fetch (xcgi_db_t *db, const char *query, ...);

   // Closes and deletes the resultset and all resources associated with
   // it.
   void xcgi_db_res_del (xcgi_db_res_t *res);

   // For diagnostics during development
   void xcgi_db_print (xcgi_db_t *db, FILE *outf);


#ifdef __cplusplus
};
#endif




#endif
