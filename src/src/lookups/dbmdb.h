/* $Cambridge: exim/src/src/lookups/dbmdb.h,v 1.4 2007/01/08 10:50:19 ph10 Exp $ */

/*************************************************
*     Exim - an Internet mail transport agent    *
*************************************************/

/* Copyright (c) University of Cambridge 1995 - 2007 */
/* See the file NOTICE for conditions of use and distribution. */

/* Header for the dbm lookup. Use dbmdb in the code to avoid name
clashes with external library names. */

extern void *dbmdb_open(uschar *, uschar **);
extern BOOL  dbmdb_check(void *, uschar *, int, uid_t *, gid_t *, uschar **);
extern int   dbmdb_find(void *, uschar *, uschar *, int, uschar **, uschar **,
               BOOL *);
extern int   dbmnz_find(void *, uschar *, uschar *, int, uschar **, uschar **,
               BOOL *);
extern void  dbmdb_close(void *);

/* End of lookups/dbmdb.h */