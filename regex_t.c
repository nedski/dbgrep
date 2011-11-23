/* test of using regex library */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

char test[100];
char regex[100];
int  res;

int main(void)

{

  /* test string */
  printf("Enter test string: ");
  fgets(test, sizeof(test), stdin);
  test[strlen(test) -1] = '\0';

  printf("Enter regex: ");
  fgets(regex, sizeof(regex), stdin);
  regex[strlen(regex) -1] = '\0';

  res = match(test, regex);
  if (1 == res)
    printf("Match for ");
  else
    printf("No match for ");

  printf("%s in: %s\n", regex, test);
  exit(EXIT_SUCCESS);
}

/* match subroutine taken from regcomp(3c) man page, Solaris */

match(const char *string, char *pattern)

{
  int status;
  regex_t re;
  if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
    perror("regcomp");
    exit(EXIT_FAILURE);      /* report error */
  }

  status = regexec(&re, string, (size_t) 0, NULL, 0);
  regfree(&re);
  if (status != 0) {
    return(0);
  }
  return(1);
}

