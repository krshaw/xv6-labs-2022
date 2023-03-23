#include "kernel/types.h"
#include "user/user.h"

void
apply(char *command[], char* additional_args, int argc) {
    printf("applying command with %s\n", additional_args);
}

int
main(int argc, char *argv[])
{
  // find all files in a directory with a given name
  char buf[512];
  int n = read(0, buf, sizeof buf);
  buf[n] = 0;
  char additional_args[512];
  int j = 0;
  char **command = argv + 1;
  char *p = buf;
  while (*p) {
    if (*p == '\n') {
        additional_args[j] = 0;
        j = 0;
        apply(command, additional_args, argc);
        p++;
        continue;
    }
    additional_args[j++] = *p++;
  }
  exit(0);
}
