/* Find regex in value of a Berkeley DB */
/* $Id: dbgrep.c,v 1.4 2000/03/21 15:43:20 neilk Exp neilk $ */

/* 
 $Log: dbgrep.c,v $
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
#include <errno.h>
#include <db.h>
#include <sys/types.h>
#include <regex.h>


char *my_name;
char *database;
char *rcs_id;

/* initial size of data for value of data */
int  val_alloc = (1024 * 4); 

/* sub declarations */
void substr(char dest[], char src[], int offset, int len);
void fail(char msg[]);
void usage(void);


int main(int argc, char *argv[])
{

  /* database */
  DB *db;
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
  char *val_str;
  char *regex;

  database = "/files/web/dev/db/user-db"; /* default to prod db */
  rcs_id = "$Id: dbgrep.c,v 1.4 2000/03/21 15:43:20 neilk Exp neilk $";

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



  /* open db file */
  /* ret = db_open(database, DB_HASH, DB_RDONLY, 0666, NULL, NULL, &db); */
  ret = DB->open(database, DB_HASH, DB_RDONLY, 0666, NULL, NULL, &db);

  if(ret)
    fail("db_open");



  /* Create a cursor */
  ret = db->cursor(db, NULL, &dbc, 0);

  if(ret)
    fail("cursor");



  /* Init key/value pair */
  memset(&key, 0, sizeof(key));
  memset(&value, 0, sizeof(value));

  /* Allocate memory for val_str */
  val_str = (char *)malloc(val_alloc);

  if (val_str == NULL)
    fail("malloc"); 
  
  /* Cycle through the database */
  while((ret = dbc->c_get(dbc, &key, &value, DB_NEXT)) 
	!= DB_NOTFOUND) {
    
    /* check size; if necessary, reallocate memory for val_str */
    if ( val_alloc < value.size ) {
      while (val_alloc < value.size) {
	val_alloc *= 2; 
      }
      val_str = (char*)realloc(val_str, val_alloc);
      if (val_str == NULL)
	fail("realloc");
    }
    
    /* c_get returns a block of data in value.data of which only
       value.size is the data record. Must take first (value.size - 1)
       chars of value.data */
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
    /* printf("%d\n", total); */

  } /* while */


  regfree(&re);

  if (verbose) {
    printf("# %s: regex: %s\n", my_name, regex);
    printf("# %s: database: %s\n", my_name, database);
    printf("# %s: records: %d; matches: %d\n", my_name, total, matches);
  }

  /* Clean up & exit */
  db->sync(db, 0);
  dbc->c_close(dbc); /* close the cursor */
  db->close(db, 0);

  exit(EXIT_SUCCESS);

} /* main */


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
