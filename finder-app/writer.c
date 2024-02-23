#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char **argv) {
  if (argc != 3) {
    syslog(LOG_USER|LOG_ERR, "Wrong argument input!");
    return 1; 
  }
  // read the argument
  char *write_file = argv[1];
  char *write_str = argv[2];

  // open file
  FILE *file = fopen(write_file, "w");
  fprintf(file, "%s", write_str);
  syslog(LOG_USER|LOG_DEBUG, "Writing %s to %s", write_str, write_file);
  return 0;
}
