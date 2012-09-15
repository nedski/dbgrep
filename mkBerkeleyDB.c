/**
   @file      mkBerkeleyDB.c
   @author    Mitch Richling <http://www.mitchr.me/>
   @Copyright Copyright 1998 by Mitch Richling.  All rights reserved.
   @brief     How to create and populate a Berkeley DB@EOL
   @Keywords  dbm ndbm berkeley db gdbm
   @Std       C99

   @Tested    
              - Solaris 2.8
              - MacOS X.2
              - Linux (RHEL 3.0)
*/

#include <sys/types.h>          /* UNIX types      POSIX */
#include <stdio.h>              /* I/O lib         ISOC  */
#include <stdlib.h>             /* Standard Lib    ISOC  */
#include <string.h>             /* Strings         ISOC  */
#include <db.h>                 /* dbm header      ????  */

#define DATABASE "access.db"

void zeroDBT(DBT *dbt);
void loadDBT(DBT *dbt, void *data, int length);
void closeDB(DB *dbp);

#define NUMELE 4

int main() { 
  DB *myDB; 
  int dbRet, i;
  DBT key, value;
  char *keys[NUMELE] = {"foo", "bar", "foobar", "foo"};
  char *vals[NUMELE] = {"FOO", "BAR", "FOOBAR", "FOOZAM"};

  /* Create the DB */
  if ((dbRet = db_create(&myDB, NULL, 0)) != 0) { // Open failure
    fprintf(stderr, "db_create: %s\n", db_strerror(dbRet)); 
    exit (1); 
  } else {
    printf("DB created.\n");
  } /* end if */

  /* Associate DB with a file (create a btree)*/
  if ((dbRet = myDB->open(myDB, NULL, "berkeleydbTest.db", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) { 
    myDB->err(myDB, dbRet, "berkeleydbTest.db"); 
    closeDB(myDB);
    exit(1);
  } else {
    printf("DB opened.\n");
  } /* end if */

  /* Note we store null terminated strings.  They don't have to be null
     terminated -- or even arrays of chars.  I just do this to make this
     example simple -- this is also a VERY common use pattern in C. */
  for(i=0; i<NUMELE; i++) {
    /* Zero out and load up our key/value DBTs */
    loadDBT(&key,   keys[i], strlen(keys[i])+1);
    loadDBT(&value, vals[i], strlen(vals[i])+1);
    /* Store our key value pair now... */
    dbRet = myDB->put(myDB, NULL, &key, &value, DB_NOOVERWRITE); // 0, not DB_NOOVERWRITE
    switch(dbRet) {
    case 0: 
      printf("Store: '%s' ==> '%s'\n", (char *)key.data, (char *)value.data); 
      break;
    case DB_KEYEXIST: 
      printf("Could not store '%s' because it was already in the DB.\n", (char *)key.data); 
      break;
    default: 
      myDB->err(myDB, dbRet, "DB->put"); 
      closeDB(myDB);
      exit(1);
    } /* end switch */
  } /* end for */

  /* Lookup the second one.. */
  loadDBT(&key, keys[1], strlen(keys[1])+1);
  dbRet = myDB->get(myDB, NULL, &key, &value, 0);
  switch(dbRet) {
  case 0:
    printf("Got record: '%s' ==> '%s'\n", (char *)key.data, (char *)value.data);
    break;
  case DB_NOTFOUND:
    printf("Fail: Could not find key '%s' in DB.\n", (char *)key.data);
    break;
  default:
    myDB->err(myDB, dbRet, "DB->get");
    closeDB(myDB);
    exit(1);
  } /* end switch */

  /* Now delete the second record.. */
  loadDBT(&key, keys[1], strlen(keys[1])+1);
  dbRet = myDB->del(myDB, NULL, &key, 0);
  switch(dbRet) {
  case 0: 
    printf("Deleted: '%s'\n", (char *)key.data);
    break;
  case DB_NOTFOUND:
    printf("Fail: Could not find key '%s' in DB.\n", (char *)key.data);
    break;
  default:
    myDB->err(myDB, dbRet, "DB->del");
    closeDB(myDB);
    exit(1);
  } /* end switch */

  /* We should always close our DB -- even before we exit.. */
  closeDB(myDB);
  printf("DB closed... Bye!\n");

  exit(1);
} /* func main */

/* The sleepycat people ask that things be zeroed so that they can add
   stuff to the DBT struct someday.  This function is just a little
   helper to do just that. */
void zeroDBT(DBT *dbt) {
  memset(dbt, 0, sizeof(DBT));  
} /* func zeroDBT */

/* This is a simple data loader for DBTs to save a couple of lines of code. */
void loadDBT(DBT *dbt, void *data, int length) {
  zeroDBT(dbt);
  dbt->data = data;
  dbt->size = length;
} /* func loadDBT */

/* I put this in a function so it can be called in error
   conditions. Normally people would put this kind of stuff in an exit
   handler, try construct in C++, or a destructor. */
void closeDB(DB *dbp) {
  int dbRet;
  dbRet = dbp->close(dbp, 0);
  switch(dbRet) {
  case 0:
    break;
  default:
    printf("Fail: Could not close the db...\n");
  }
} /* func closeDB */
