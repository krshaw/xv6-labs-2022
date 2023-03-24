#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

void
append(char *res[], char *A[], int a_len, char B[MAXARG][512], int b_len) {
    for (int i = 0; i < a_len; i++) res[i] = A[i];
    for (int i = 0; i < b_len; i++) res[i+a_len - 1] = B[i];
}

void
split(char strs[MAXARG][512], char *str, char delim, int* c) {
    int i = 0;
    int j = 0;
    //char single_str[512];
    char *p = str;
    while (*p) {
        if (*p == delim) {
            strs[i][j++] = 0;
            j = 0;
            i++;
            p++;
            continue;
        }
        //single_str[j++] = *p++;
        strs[i][j++] = *p++;
    }
    *c = i;
}

void
apply(char *command[], char* additional_args, int argc) {

    //printf("applying command with %s\n", additional_args);
    // split additional_args by space
    char *args[MAXARG];
    int more_argc;
    char more_args[MAXARG][512];
    split(more_args, additional_args, ' ', &more_argc);
    for (int i = 0; i < more_argc; i++) printf("%s\n", more_args[i]);
    //printf("MORE_ARGC: %d\n", more_argc);
    if (argc - 1 + more_argc > MAXARG) {
        fprintf(2, "too many args!\n");
    }
    append(args, command + 1, argc - 1, more_args, more_argc);
 
    if (fork() == 0) {
        // child
        printf("%s\n", command[0]);
        exec(command[0], args);
        printf("exec error\n");
    } else {
        wait(0);
        exit(0);
    }
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
      fprintf(2, "usage: xargs [command]\n");
      exit(0);
  }
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
        //printf("HERE\n");
        apply(command, additional_args, argc);
        p++;
        continue;
    }
    additional_args[j++] = *p++;
  }
  exit(0);
}
