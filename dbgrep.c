/* Find regex in value of a Berkeley DB */
/* $Id: dbgrep.c,v 1.5 2012/09/14 20:54:18 neilk Exp $ */

/*
 * $Log: dbgrep.c,v $
 * Revision 1.5  2012/09/14 20:54:18  neilk
 * Updated for Berkeley version 5.1.29
 *
 * Revision 1.4  2000/03/21 15:43:20  neilk
 * Added -v switch.
 *
 * Revision 1.3  2000/03/21 15:35:38  neilk
 * Accepts db and regexp from command line. Default db is now production.
 *
 * Revision 1.2  2000/03/21 14:29:56  neilk
 * Version that prompts user for regex.
 *
 * Revision 1.1  2000/03/21 13:38:49  neilk
 * Initial revision
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <db.h>
#include <sys/types.h>
#include <regex.h>


char *my_name;
char *database;
char *rcs_id;

/* substr returns len chars from src starting at offset */

void substr(char dest[], char src[], int offset, int len)
{
  int i;
  for(i = 0; i < len && src[offset + i] != '\0'; i++)
    dest[i] = src[i + offset];
  dest[i] = '\0';
} /* substr */



void fail(char msg[])
{
  perror(msg);
  exit(EXIT_FAILURE);
} /* fail */


void usage(void)
{
  fprintf(stderr, "usage: %s -d<database> -r<regex> [-v]\n", my_name);
  fprintf(stderr, "Look for <regex> in values of Berkeley DB <database>.\n\n");
  fprintf(stderr, "  -d: database to search\n");
  fprintf(stderr, "      Default is %s\n", database);
  fprintf(stderr, "  -r: regular expression; see regex(5)\n");
  fprintf(stderr, "  -v: verbose; print summary\n\n");
  fprintf(stderr, "%s\n\n", rcs_id);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{

  /* database */
  DB *dbp;
  DBC *dbc;
  DBT key, value;
  int ret;

  /* regex */
  regex_t re;
  int status;

  /* misc */
  int total = 0;
  int matches = 0;
  int verbose = 0;
  char *val_str, *regex;
  database = "/files/web/production/db/user-db"; /* default to prod db */
  rcs_id = "$Id: dbgrep.c,v 1.5 2012/09/14 20:54:18 neilk Exp $";

  /* process command line */
  my_name = argv[0];

  while ((argc > 1 ) && (argv[1][0] == '-')) {
    /*
     * argv[1][1] is the option
     */
    switch (argv[1][1]) {
      /*
       * -d database
       */
    case 'd':
      database = &argv[1][2];
      break;
      
      /*
       * -r regex
       */
    case 'r':
      regex = &argv[1][2];
      break;

      /*
       * -v verbose (summary info after search)
       */
    case 'v':
      verbose = 1;
      break;

      /* 
       * else print usage
       */
    default:
      usage();

    } /* switch */

    /* 
     * incr argument list
     * decr argument count
     */
    argv++;
    argc--;

  } /* while */


  if (regex == NULL)
    usage();

  /* compile regexp */
  if (regcomp(&re, regex, REG_EXTENDED|REG_NOSUB) != 0)
    fail("regcomp");

  /* create db structure */
  db_create(&dbp, NULL, 0);

  /* open db file */
  ret = dbp->open(dbp, NULL, database, NULL, DB_HASH, DB_RDONLY, 0666);

  if(ret)
    fail("db_open");



  /* Create a cursor */
  ret = dbp->cursor(dbp, NULL, &dbc, 0);

  if(ret)
    fail("cursor");



  /* Init key/value pair */
  memset(&key, 0, sizeof(key));
  memset(&value, 0, sizeof(value));



  /* Cycle through the database */
  while((ret = dbc->c_get(dbc, &key, &value, DB_NEXT)) 
	!= DB_NOTFOUND) {
    
    /* c_get returns a block of data in value.data of which only
       value.size is the data record. Must take first (value.size - 1)
       chars of value.data */
    val_str = (char *)malloc(value.size + 1);

    if (val_str == NULL)
      fail("malloc");

    substr(val_str, value.data, (size_t) 0, value.size);

    /* Compare regexp against data */
    status = regexec(&re, (char *)val_str, (size_t) 0, NULL, 0);
   
    if (status == 0) {
      /* print record if there's a match */
      printf("%.*s\t%s\n", (int)key.size, (char *)key.data, val_str);
      matches++;
    }

    free(val_str);

    total++;

  } /* while */


  regfree(&re);

  if (verbose) {
    printf("# %s: regex: %s\n", my_name, regex);
    printf("# %s: database: %s\n", my_name, database);
    printf("# %s: records: %d; matches: %d\n", my_name, total, matches);
  }

  /* Clean up & exit */
  dbp->sync(dbp, 0);
  dbc->c_close(dbc); /* close the cursor */
  dbp->close(dbp, 0);

  exit(EXIT_SUCCESS);

} /* main */


