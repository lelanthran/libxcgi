#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <time.h>

// TODO: The blob type was not tested!

#include "sqlite3.h"
#include <postgresql/libpq-fe.h>

#include "xcgi_db.h"

#define SQLDB_OOM(s)          fprintf (stderr, "OOM [%s]\n", s)

static char *lstr_dup (const char *src)
{
   if (!src)
      return NULL;

   size_t len = strlen (src) + 1;

   char *ret = malloc (len);
   if (!ret)
      return NULL;

   return strcpy (ret, src);
}

bool xcgi_db_create (const char *dbname, xcgi_db_dbtype_t type)
{
   sqlite3 *newdb = NULL;
   bool ret = false;

   if (type!=xcgi_db_SQLITE)
      return true;

   if (!dbname)
      return false;

   int mode = (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
   int rc = sqlite3_open_v2 (dbname, &newdb, mode, NULL);
   if (rc!=SQLITE_OK) {
      fprintf (stderr, "(%s) Unable to create sqlite file - %s [%m]\n",
               dbname, sqlite3_errstr (rc));
      goto errorexit;
   }

   ret = true;
errorexit:
   if (newdb)
      sqlite3_close (newdb);

   return ret;
}

// TODO: Do we need a lockfile for SQLITE?
struct xcgi_db_t {
   xcgi_db_dbtype_t type;
   char *lasterr;

   // Used for sqlite only
   sqlite3 *sqlite_db;

   // Used for postgres only
   PGconn *pg_db;
   uint64_t nchanges;
};

struct xcgi_db_res_t {
   xcgi_db_dbtype_t type;
   xcgi_db_t       *dbcon;
   char          *lasterr;

   // For sqlite
   sqlite3_stmt *sqlite_stmt;

   // For postgres
   PGresult *pgr;
   int current_row;
   int nrows;
   uint64_t last_id;
};


static void err_printf (char **dst, const char *fmts, va_list ap)
{
   va_list apc;
   char *tmp = NULL;
   size_t len = 0;

   if (!dst || !fmts)
      return;

   va_copy (apc, ap);

   if (!(len = vsnprintf (tmp, 0, fmts, apc)))
      return;

   if (!(tmp = malloc (len + 1))) {
      fprintf (stderr, "FATAL ERROR: Out of memory in err_printf()\n");
      return;
   }

   memset (tmp, 0, len + 1);

   vsnprintf (tmp, len, fmts, ap);
   free (*dst);
   (*dst) = tmp;

   va_end (apc);
}

static void db_err_printf (xcgi_db_t *db, const char *fmts, ...)
{
   va_list ap;
   if (!db)
      return;

   va_start (ap, fmts);
   err_printf (&db->lasterr, fmts, ap);
   va_end (ap);
}

static void res_err_printf (xcgi_db_res_t *res, const char *fmts, ...)
{
   va_list ap;
   if (!res)
      return;

   va_start (ap, fmts);
   err_printf (&res->lasterr, fmts, ap);
   va_end (ap);
}

// A lot of the following functions will be refactored only when working
// on the postgresql integration
static xcgi_db_t *sqlitedb_open (xcgi_db_t *ret, const char *dbname)
{
   bool error = true;
   int mode = SQLITE_OPEN_READWRITE;
   int rc = sqlite3_open_v2 (dbname, &ret->sqlite_db, mode, NULL);
   if (rc!=SQLITE_OK) {
      const char *tmp =  sqlite3_errstr (rc);
      fprintf (stderr, "(%s) Unable to open database: %s\n", dbname, tmp);
      goto errorexit;
   }

   error = false;

errorexit:
   if (error) {
      xcgi_db_close (ret);
      ret = NULL;
   }
   return ret;
}

static xcgi_db_t *pgdb_open (xcgi_db_t *ret, const char *dbname)
{
   bool error = false;

   if (!(ret->pg_db = PQconnectdb (dbname))) {
      SQLDB_OOM (dbname);
      goto errorexit;
   }

   if ((PQstatus (ret->pg_db))==CONNECTION_BAD) {
      fprintf (stderr, "[%s] Connection failure: [%s]\n",
                       dbname,
                       PQerrorMessage (ret->pg_db));
      goto errorexit;
   }

   error = false;

errorexit:
   if (error) {
      xcgi_db_close (ret);
      ret = NULL;
   }

   return ret;
}

xcgi_db_t *xcgi_db_open (const char *dbname, xcgi_db_dbtype_t type)
{
   xcgi_db_t *ret = malloc (sizeof *ret);
   if (!ret) {
      SQLDB_OOM (dbname);
      goto errorexit;
   }

   memset (ret, 0, sizeof *ret);

   ret->type = type;

   if (!dbname)
      goto errorexit;

   switch (type) {
      case xcgi_db_SQLITE:   return sqlitedb_open (ret, dbname);
      case xcgi_db_POSTGRES: return pgdb_open (ret, dbname);
      default:             fprintf (stderr, "Error: dbtype [%u] is unknown\n",
                                            type);
                           goto errorexit;
   }

errorexit:
   free (ret);
   return NULL;
}

void xcgi_db_close (xcgi_db_t *db)
{
   if (!db)
      return;

   switch (db->type) {
      case xcgi_db_SQLITE:   sqlite3_close (db->sqlite_db);               break;
      case xcgi_db_POSTGRES: PQfinish (db->pg_db);                        break;
      default:             db_err_printf (db, "Unknown type %i\n", db->type);
                           break;
   }

   free (db->lasterr);
   memset (db, 0, sizeof *db);
   free (db);
}

const char *xcgi_db_lasterr (xcgi_db_t *db)
{
   return db ? db->lasterr : "(NULL SQLDB OBJECT)";
}

void xcgi_db_clearerr (xcgi_db_t *db)
{
   if (!db)
      return;

   free (db->lasterr);
   db->lasterr = NULL;
}

static char *fix_string (xcgi_db_dbtype_t type, const char *string)
{
   char *ret = NULL;
   char r = 0;

   switch (type) {
      case xcgi_db_SQLITE:      r = '?'; break;
      case xcgi_db_POSTGRES:    r = '$'; break;
      default:                r = 0;   break;
   }

   if (!r) {
      fprintf (stderr, "(%i) Unknown type\n", type);
      return NULL;
   }

   ret = lstr_dup (string);
   if (!ret) {
      SQLDB_OOM (string);
      return NULL;
   }

   char *tmp = ret;
   while (*tmp) {
      if (*tmp=='#')
         *tmp = r;
      tmp++;
   }

   return ret;
}

static void res_seterr (xcgi_db_res_t *res, const char *msg)
{
   if (!res || !msg)
      return;

   free (res->lasterr);
   res->lasterr = lstr_dup (msg);
}

const char *xcgi_db_res_lasterr (xcgi_db_res_t *res)
{
   if (!res)
      return NULL;

   return res->lasterr;
}

void xcgi_db_res_clearerr (xcgi_db_res_t *res)
{
   if (!res)
      return;

   free (res->lasterr);
   res->lasterr = NULL;
}

uint64_t xcgi_db_count_changes (xcgi_db_t *db)
{
   uint64_t ret = 0;

   if (!db)
      return 0;

   switch (db->type) {
      case xcgi_db_SQLITE:   ret = sqlite3_changes (db->sqlite_db);
                           break;

      case xcgi_db_POSTGRES: ret = db->nchanges;
                           break;

      default:             ret = 0;
                           break;
   }

   return ret;
}

uint64_t xcgi_db_res_last_id (xcgi_db_res_t *res)
{
   uint64_t ret = 0;

   if (!res)
      return 0;

   switch (res->type) {
      case xcgi_db_SQLITE:   ret = sqlite3_last_insert_rowid (res->dbcon->sqlite_db);
                           break;

      case xcgi_db_POSTGRES: ret = res->last_id;
                           break;

      default:             ret = 0;
                           break;
   }

   return ret;
}

uint32_t xcgi_db_res_num_columns (xcgi_db_res_t *res)
{
   if (!res)
      return 0;

   uint32_t ret = 0;

   switch (res->type) {
      case  xcgi_db_SQLITE:  ret = sqlite3_column_count (res->sqlite_stmt);
                           break;

      case xcgi_db_POSTGRES: ret = PQnfields (res->pgr);
                           break;

      default:             ret = 0;
                           break;
   }

   return ret;
}

char **xcgi_db_res_column_names (xcgi_db_res_t *res)
{
   bool error = true;
   char **ret = NULL;
   uint32_t ncols = 0;

   if (!res)
      goto errorexit;

   if (!(ncols = xcgi_db_res_num_columns (res)))
      goto errorexit;

   if (!(ret = malloc ((sizeof *ret) * (ncols + 1))))
      goto errorexit;

   memset (ret, 0, (sizeof *ret) * (ncols + 1));

   for (uint32_t i=0; i<ncols; i++) {
      const char *tmp = NULL;
      switch (res->type) {
         case  xcgi_db_SQLITE:  tmp = sqlite3_column_name (res->sqlite_stmt, i);
                              break;

         case xcgi_db_POSTGRES: tmp = PQfname (res->pgr, i);
                              break;

         default:             tmp = NULL;
                              break;
      }
      if (!(ret[i] = lstr_dup (tmp)))
         goto errorexit;
   }

   error = false;

errorexit:
   if (error) {
      for (size_t i=0; ret && ret[i]; i++)
         free (ret[i]);

      free (ret);
      ret = NULL;
   }

   return ret;
}

static xcgi_db_res_t *sqlitedb_exec (xcgi_db_t *db, char *qstring, va_list *ap)
{
   int counter = 0;
   bool error = true;
   xcgi_db_res_t *ret = malloc (sizeof *ret);
   if (!ret)
      return NULL;
   memset (ret, 0, sizeof *ret);

   ret->type = xcgi_db_SQLITE;

   int rc = sqlite3_prepare_v2 (db->sqlite_db, qstring, -1,
                                &ret->sqlite_stmt, NULL);
   if (rc!=SQLITE_OK) {
      const char *tmp = sqlite3_errstr (rc);
      db_err_printf (db, "Fatal error: %s/%i\n[%s]\n", tmp, rc, qstring);
      goto errorexit;
   }

   sqlite3_stmt *stmt = ret->sqlite_stmt;
   xcgi_db_coltype_t coltype = va_arg (*ap, xcgi_db_coltype_t);
   while (coltype!=xcgi_db_col_UNKNOWN) {
      int32_t *v_int;
      int64_t *v_int64;
      char **v_text;
      void *v_blob;
      void *v_ptr;
      uint32_t *v_bloblen;
      int err = 0;
      int index = counter + 1;

      switch (coltype) {
         case xcgi_db_col_UNKNOWN:
            db_err_printf (db, "Impossible error\n");
            goto errorexit;

         case xcgi_db_col_UINT32:
         case xcgi_db_col_INT32:
            v_int = va_arg (*ap, int32_t *);
            err = sqlite3_bind_int (stmt, index, *v_int);
            break;

         case xcgi_db_col_DATETIME:
         case xcgi_db_col_UINT64:
         case xcgi_db_col_INT64:
            v_int64 = va_arg (*ap, int64_t *);
            err = sqlite3_bind_int64 (stmt, index, *v_int64);
            break;

         case xcgi_db_col_TEXT:
            v_text = va_arg (*ap, char **);
            err = sqlite3_bind_text (stmt, index, (*v_text),
                                     -1, SQLITE_TRANSIENT);
            break;

         case xcgi_db_col_BLOB:
            v_blob = va_arg (*ap, void *);
            v_bloblen = va_arg (*ap, uint32_t *);
            err = sqlite3_bind_blob (stmt, index, *(uint8_t **)v_blob,
                                     *v_bloblen, SQLITE_TRANSIENT);
            break;

         case xcgi_db_col_NULL:
            v_ptr = va_arg (*ap, void *); // Discard the next argument
            v_ptr = v_ptr;
            err = sqlite3_bind_null (stmt, index);
            break;

         default:
            db_err_printf (db, "Unknown column type: %u\n", coltype);
            break;
      }

      if (err!=SQLITE_OK) {
         db_err_printf (db, "Unable to bind %i\n", index);
         goto errorexit;
      }
      counter++;
      coltype = va_arg (*ap, xcgi_db_coltype_t);
   }

   error = false;

errorexit:
   if (error) {
      xcgi_db_res_del (ret);
      ret = NULL;
   }
   return ret;
}

static xcgi_db_res_t *pgdb_exec (xcgi_db_t *db, char *qstring, va_list *ap)
{
   bool error = true;
   int nParams = 0;
   xcgi_db_res_t *ret = NULL;

   char **paramValues = NULL;

   static const Oid *paramTypes = NULL;
   static const int *paramLengths = NULL;
   static const int *paramFormats = NULL;

   va_list ac;
   va_copy (ac, (*ap));
   xcgi_db_coltype_t coltype = va_arg (ac, xcgi_db_coltype_t);
   while (coltype!=xcgi_db_col_UNKNOWN) {
      nParams++;
      const void *ignore = va_arg (ac, const void *);
      ignore = ignore;
      coltype = va_arg (ac, xcgi_db_coltype_t);
   }
   va_end (ac);


   ret = malloc (sizeof *ret);
   if (!ret)
      goto errorexit;
   memset (ret, 0, sizeof *ret);

   paramValues = malloc ((sizeof *paramValues) * (nParams + 1));
   if (!paramValues)
      goto errorexit;
   memset (paramValues, 0, (sizeof *paramValues) * (nParams + 1));

   ret->type = xcgi_db_POSTGRES;

   size_t index = 0;
   coltype = va_arg (*ap, xcgi_db_coltype_t);
   while (coltype!=xcgi_db_col_UNKNOWN) {
      uint32_t *v_uint;
      uint64_t *v_uint64;
      int32_t *v_int;
      int64_t *v_int64;
      char **v_text;
      void *v_blob;
      void *v_ptr;
      uint32_t *v_bloblen;

      char intstr[42];
      char *value = intstr;

      switch (coltype) {
         case xcgi_db_col_UNKNOWN:
            db_err_printf (db, "Impossible error\n");
            goto errorexit;

         case xcgi_db_col_UINT32:
            v_uint = va_arg (*ap, uint32_t *);
            sprintf (intstr, "%i", *v_uint);
            break;

         case xcgi_db_col_INT32:
            v_int = va_arg (*ap, int32_t *);
            sprintf (intstr, "%i", *v_int);
            break;

         case xcgi_db_col_UINT64:
            v_uint64 = va_arg (*ap, uint64_t *);
            sprintf (intstr, "%" PRIu64, *v_uint64);
            break;

         case xcgi_db_col_INT64:
            v_int64 = va_arg (*ap, int64_t *);
            sprintf (intstr, "%" PRIi64, *v_int64);
            break;

         case xcgi_db_col_DATETIME:
            // TODO: ???

         case xcgi_db_col_TEXT:
            v_text = va_arg (*ap, char **);
            value = *v_text;
            break;

         case xcgi_db_col_BLOB:
            v_blob = va_arg (*ap, void *);
            v_bloblen = va_arg (*ap, uint32_t *);
            // TODO: ???
            v_bloblen = v_bloblen;
            v_blob = v_blob;
            value = NULL;
            break;

         case xcgi_db_col_NULL:
            v_ptr = va_arg (*ap, void *); // Discard the next argument
            v_ptr = v_ptr;
            value = NULL;
            break;

         default:
            db_err_printf (db, "Unknown column type: %u\n", coltype);
            break;
      }

      paramValues[index++] = lstr_dup (value);
      coltype = va_arg (*ap, xcgi_db_coltype_t);
   }

   ret->pgr = PQexecParams (db->pg_db, qstring, nParams,
                                                paramTypes,
                           (const char *const *)paramValues,
                                                paramLengths,
                                                paramFormats,
                                                0);
   if (!ret->pgr) {
      db_err_printf (db, "Possible OOM error (pg)\n[%s]\n", qstring);
      goto errorexit;
   }

   ExecStatusType rs = PQresultStatus (ret->pgr);
   if (rs != PGRES_COMMAND_OK && rs != PGRES_TUPLES_OK) {
      db_err_printf (db, "Bad postgres return status [%s]\n[%s]\n",
                          PQresultErrorMessage (ret->pgr),
                          qstring);
      goto errorexit;
   }

   ret->current_row = 0;
   ret->nrows = PQntuples (ret->pgr);
   const char *tmp = PQcmdTuples (ret->pgr);
   if (tmp) {
      sscanf (tmp, "%" PRIu64, &db->nchanges);
   } else {
      db->nchanges = 0;
   }

   Oid last_id = PQoidValue (ret->pgr);
   ret->last_id = last_id == InvalidOid ? (uint64_t)-1 : last_id;

   error = false;

errorexit:

   for (int i=0; paramValues && i<nParams; i++) {
      free (paramValues[i]);
   }
   free (paramValues);

   if (error) {
      xcgi_db_res_del (ret);
      ret = NULL;
   }
   return ret;
}

xcgi_db_res_t *xcgi_db_exec (xcgi_db_t *db, const char *query, ...)
{
   xcgi_db_res_t *ret = NULL;
   va_list ap;

   va_start (ap, query);
   ret = xcgi_db_execv (db, query, &ap);
   va_end (ap);

   return ret;
}

xcgi_db_res_t *xcgi_db_execv (xcgi_db_t *db, const char *query, va_list *ap)
{
   bool error = true;
   xcgi_db_res_t *ret = NULL;
   char *qstring = NULL;

   if (!db || !query)
      return NULL;

   qstring = fix_string (db->type, query);
   if (!qstring) {
      SQLDB_OOM (query);
      goto errorexit;
   }

   xcgi_db_clearerr (db);

   switch (db->type) {
      case xcgi_db_SQLITE:   ret = sqlitedb_exec (db, qstring, ap);       break;
      case xcgi_db_POSTGRES: ret = pgdb_exec (db, qstring, ap);           break;
      default:             db_err_printf (db, "(%i) Unknown type\n", db->type);
                           goto errorexit;
   }
   if (!ret) {
      db_err_printf (db, "Failed to execute stmt [%s]\n", qstring);
      goto errorexit;
   }

   ret->dbcon = db;
   error = false;

errorexit:
   free (qstring);
   if (error) {
      xcgi_db_res_del (ret);
      ret = NULL;
   }
   return ret;
}

bool xcgi_db_exec_ignore (xcgi_db_t *db, const char *query, ...)
{
   va_list ap;

   va_start (ap, query);
   bool ret = xcgi_db_exec_ignorev (db, query, &ap);
   va_end (ap);

   return ret;
}

bool xcgi_db_exec_ignorev (xcgi_db_t *db, const char *query, va_list *ap)
{
   bool error = true;

   xcgi_db_res_t *res = NULL;

   if (!(res = xcgi_db_execv (db, query, ap)))
      goto errorexit;

   if ((xcgi_db_res_step (res))==-1)
      goto errorexit;

   error = false;

errorexit:
   xcgi_db_res_del (res);

   return !error;

}


static bool sqlitedb_batch (xcgi_db_t *db, va_list ap)
{
   bool ret = true;
   if (!db->sqlite_db)
      return false;

   char *qstring = va_arg (ap, char *);

   while (ret && qstring) {
      char *errmsg = NULL;
      int rc = sqlite3_exec (db->sqlite_db, qstring, NULL, NULL, &errmsg);

      if (rc!=SQLITE_OK) {
         ret = false;
         db_err_printf (db, "DB exec failure [%s]\n[%s]\n", qstring, errmsg);
      }

      sqlite3_free (errmsg);
      qstring = va_arg (ap, char *);
   }

   return ret;
}

static bool pgdb_batch (xcgi_db_t *db, va_list ap)
{
   bool ret = true;

   if (!db->pg_db)
      return false;

   char *qstring = va_arg (ap, char *);

   while (ret && qstring) {
      PGresult *result = PQexec (db->pg_db, qstring);
      if (!result) {
         db_err_printf (db, "PGSQL - OOM error\n[%s]\n", qstring);
         ret = false;
         break;
      }
      ExecStatusType rc = PQresultStatus (result);
      if (rc == PGRES_BAD_RESPONSE || rc == PGRES_FATAL_ERROR) {
         const char *rc_msg = PQresStatus (rc),
                    *res_msg = PQresultErrorMessage (result);

         db_err_printf (db, "pgdb_batch: Bad postgres result[%s]\n[%s]\n[%s]\n",
                            rc_msg,
                            res_msg,
                            qstring);
         ret = false;
         PQclear (result);
         break;
      }

      PQclear (result);
      qstring =  va_arg (ap, char *);
   }

   return ret;
}


bool xcgi_db_batch (xcgi_db_t *db, ...)
{
   bool ret;
   va_list ap;

   if (!db)
      return false;

   va_start (ap, db);

   switch (db->type) {

      case xcgi_db_SQLITE:   ret = sqlitedb_batch (db, ap);   break;

      case xcgi_db_POSTGRES: ret = pgdb_batch (db, ap);       break;

      default:             db_err_printf (db, "(%i) Format unsupported\n",
                                              db->type);
                           ret = false;
                           break;
   }
   va_end (ap);
   return ret;
}

static char *push_char (char **dst, size_t *len, char c)
{
   char *tmp = realloc ((*dst), (*len)+2);
   if (!tmp) {
      return NULL;
   }
   (*dst) = tmp;
   (*dst)[(*len)] = c & 0xff;
   (*dst)[(*len)+1] = 0;
   (*len)++;

   return (*dst);
}

static char *read_stmt (FILE *inf)
{
   char *ret = NULL;
   size_t len = 0;
   int c = 0;
   int pc = 0;
   char cquote = 0;
   bool comment = false;

   while (!feof (inf) && !ferror (inf) && ((c = fgetc (inf))!=EOF)) {

      if (c=='\n')
         comment = false;

      if (c=='-' && pc=='-')
         comment = true;

      if (!cquote) {
         if (c == '\'') cquote = '\'';
         if (c == '"')  cquote = '"';
         if (c == ';' && !comment) {
            // We can probably do without the terminating ';' here, we are
            // returning a full statement anyway and the caller will
            // execute if immediately, hence there is no reason to free
            // ret and return NULL.
            ret = push_char (&ret, &len, c);
            break;
         }
      } else {
         if (c == '\'' || c == '"')
            cquote = 0;
      }

      if (!(ret = push_char (&ret, &len, c))) {
         free (ret);
         return NULL;
      }

      pc = c;
   }

   return ret;
}

bool xcgi_db_batchfile (xcgi_db_t *db, FILE *inf)
{
   char *stmt = NULL;

   while ((stmt = read_stmt (inf))) {
      printf ("FOUND STATEMENT: [%s]\n", stmt);
      if (!(xcgi_db_batch (db, stmt, NULL))) {
         db_err_printf (db, "Batchfile statement failed: \n%s\n", stmt);
         free (stmt);
         return false;
      }

      free (stmt);
   }
   return true;
}

static int sqlite_res_step (xcgi_db_res_t *res)
{
   int rc = sqlite3_step (res->sqlite_stmt);

   if (rc==SQLITE_DONE)
      return 0;

   if (rc!=SQLITE_ROW) {
      res_seterr (res, sqlite3_errstr (rc));
      return -1;
   }

   return 1;
}

static int pgdb_res_step (xcgi_db_res_t *res)
{
   return res->nrows - res->current_row ? 1 : 0;
}

int xcgi_db_res_step (xcgi_db_res_t *res)
{
   int ret = -1;
   if (!res)
      return ret;

   switch (res->type) {
      case xcgi_db_SQLITE:   ret = sqlite_res_step (res);  break;
      case xcgi_db_POSTGRES: ret = pgdb_res_step (res);    break;
      default:             ret = -1;                     break;
   }
   return ret;
}

static uint64_t convert_ISO8601_to_uint64 (const char *src)
{
   struct tm tm;

   memset (&tm, 0, sizeof tm);

   if ((sscanf (src, "%4d-%2d-%2d %2d:%2d:%2d", &tm.tm_year,
                                                &tm.tm_mon,
                                                &tm.tm_mday,
                                                &tm.tm_hour,
                                                &tm.tm_min,
                                                &tm.tm_sec)) != 6)
      return (uint64_t)-1;

   tm.tm_year -= 1900;
   tm.tm_mon -= 1;

   return mktime (&tm);
}

uint32_t sqlite_scan (xcgi_db_res_t *res, va_list *ap)
{
   uint32_t ret = 0;

   if (!res)
      return (uint32_t)-1;

   sqlite3_stmt *stmt = res->sqlite_stmt;

   xcgi_db_coltype_t coltype = va_arg (*ap, xcgi_db_coltype_t);

   int numcols = sqlite3_column_count (stmt);

   while (coltype!=xcgi_db_col_UNKNOWN && numcols--) {

      void *dst = va_arg (*ap, void *);
      uint32_t *blen;
      const void *tmp;

      switch (coltype) {

         case xcgi_db_col_UNKNOWN:
            res_err_printf (res, "Possibly corrupt stack");
            // TODO: Store value in result
            return (uint32_t)-1;

         case xcgi_db_col_UINT32:
         case xcgi_db_col_INT32:
            *(uint32_t *)dst = sqlite3_column_int (stmt, ret);
            break;

         case xcgi_db_col_DATETIME:
            tmp = sqlite3_column_text (stmt, ret);
            *(uint64_t *)dst = convert_ISO8601_to_uint64 (tmp);
            if ((*(uint64_t *)dst) == (uint64_t)-1)
               return (uint32_t)-1;
            break;

         case xcgi_db_col_UINT64:
         case xcgi_db_col_INT64:
            *(uint64_t *)dst = sqlite3_column_int (stmt, ret);
            break;

         case xcgi_db_col_TEXT:
            *(char **)dst =
               lstr_dup ((char *)sqlite3_column_text (stmt, ret));
            if (!(*(char **)dst)) {
               SQLDB_OOM (sqlite3_column_text (stmt, ret));
               return (uint32_t)-1;
            }
            break;

         case xcgi_db_col_BLOB:
            blen = va_arg (*ap, uint32_t *);
            tmp = sqlite3_column_blob (stmt, ret);
            *blen = sqlite3_column_bytes (stmt, ret);
            *(void **)dst = malloc ((*blen));
            if (!dst) {
               SQLDB_OOM ("Blob type");
               return (uint32_t)-1;
            }
            memcpy (*(void **)dst, tmp, *blen);
            break;

         case xcgi_db_col_NULL:
            res_err_printf (res, "Error: NULL type not supported\n");
            // TODO: Store value in result
            return (uint32_t)-1;

      }
      coltype = va_arg (*ap, xcgi_db_coltype_t);
      ret++;
   }

   return ret;
}

uint32_t pgdb_scan (xcgi_db_res_t *res, va_list *ap)
{
   uint32_t ret = 0;

   if (!res)
      return (uint32_t)-1;

   xcgi_db_coltype_t coltype = va_arg (*ap, xcgi_db_coltype_t);

   int numcols = PQnfields (res->pgr);
   int index = 0;

   while (coltype!=xcgi_db_col_UNKNOWN && numcols--) {

      void *dst = va_arg (*ap, void *);

      int32_t  i32;
      uint32_t u32;
      int64_t  i64;
      uint64_t u64;

      const char *value = PQgetvalue (res->pgr, res->current_row, index++);
      if (!value)
         return (uint32_t)-1;

      switch (coltype) {

         case xcgi_db_col_UNKNOWN:
            res_err_printf (res, "Possibly corrupt stack");
            // TODO: Store value in result
            return (uint32_t)-1;

         case xcgi_db_col_UINT32:
            if ((sscanf (value, "%u", &u32))!=1)
               return (uint32_t)-1;
            *(uint32_t *)dst = u32;
            break;

         case xcgi_db_col_INT32:
            if ((sscanf (value, "%i", &i32))!=1)
               return (uint32_t)-1;
            *(uint32_t *)dst = i32;
            break;

         case xcgi_db_col_UINT64:
            if ((sscanf (value, "%" PRIu64, &u64))!=1)
               return (uint32_t)-1;
            *(uint32_t *)dst = u64;
            break;

         case xcgi_db_col_INT64:
            if ((sscanf (value, "%" PRIu64, &i64))!=1)
               return (uint32_t)-1;
            *(uint32_t *)dst = i64;
            break;

         case xcgi_db_col_TEXT:
            if ((*(char **)dst = lstr_dup (value))==NULL)
               return (uint32_t)-1;
            break;

         case xcgi_db_col_DATETIME:
            *(uint64_t *)dst = convert_ISO8601_to_uint64 (value);
            if ((*(uint64_t *)dst) == (uint64_t)-1)
               return (uint32_t)-1;
            break;

         case xcgi_db_col_BLOB:
            res_err_printf (res, "Error: BLOB type not supported\n");
            // TODO: Store value in result
            return (uint32_t)-1;

         case xcgi_db_col_NULL:
            res_err_printf (res, "Error: NULL type not supported\n");
            // TODO: Store value in result
            return (uint32_t)-1;

      }
      coltype = va_arg (*ap, xcgi_db_coltype_t);
      ret++;
   }

   res->current_row++;
   return ret;
}

uint32_t xcgi_db_scan_columns (xcgi_db_res_t *res, ...)
{
   uint32_t ret = 0;
   va_list ap;

   va_start (ap, res);
   ret = xcgi_db_scan_columnsv (res, &ap);
   va_end (ap);

   return ret;
}

uint32_t xcgi_db_scan_columnsv (xcgi_db_res_t *res, va_list *ap)
{
   uint32_t ret = (uint32_t)-1;

   switch (res->type) {
      case xcgi_db_SQLITE:   ret = sqlite_scan (res, ap);  break;
      case xcgi_db_POSTGRES: ret = pgdb_scan (res, ap);    break;
      default:             ret = (uint32_t)-1;           break;
   }

   return ret;
}

uint32_t xcgi_db_exec_and_fetch (xcgi_db_t *db, const char *query, ...)
{
   va_list ap;
   uint32_t ret = (uint32_t)-1;
   xcgi_db_res_t *res = NULL;

   va_start (ap, query);

   xcgi_db_clearerr (db);

   res = xcgi_db_execv (db, query, &ap);
   if (!res) {
      db_err_printf (db, "[%s] - sql exection failed: %s\n",
                         query, xcgi_db_lasterr (db));
      goto errorexit;
   }

   int step = xcgi_db_res_step (res);
   if (step==0) {
      db_err_printf (db, "[%s] - No results (%s)!\n",
                         query, xcgi_db_lasterr (db));
      goto errorexit;
   }
   if (step==-1) {
      db_err_printf (db, "[%s] - Error during fetch (%s)\n",
                         query, xcgi_db_lasterr (db));
      goto errorexit;
   }
   if (step!=1) {
      db_err_printf (db, "[%s] - Unknown error (%s)\n",
                         query, xcgi_db_lasterr (db));
      goto errorexit;
   }

   ret = xcgi_db_scan_columnsv (res, &ap);

errorexit:
   xcgi_db_res_del (res);
   va_end (ap);

   return ret;
}

void xcgi_db_res_del (xcgi_db_res_t *res)
{
   if (!res)
      return;

   xcgi_db_res_clearerr (res);

   switch (res->type) {
      case xcgi_db_SQLITE:   sqlite3_finalize (res->sqlite_stmt);         break;
      case xcgi_db_POSTGRES: PQclear (res->pgr);                          break;
      default:             res_err_printf (res, "(%i) Unknown type\n",
                                                res->type);
                           // TODO: Store value in result
                           break;
   }
   free (res);
}

void xcgi_db_print (xcgi_db_t *db, FILE *outf)
{
   if (!outf) {
      outf = stdout;
   }

   if (!db) {
      fprintf (outf, "xcgi_db_t obvject is NULL\n");
      return;
   }

   fprintf (outf, "%30s: %s\n", "type", db->type==xcgi_db_SQLITE ?
                                        "SQLITE" : "POSTGRES");
   fprintf (outf, "%30s: %s\n", "lasterr", db->lasterr);
}

