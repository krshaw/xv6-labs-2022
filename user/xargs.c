#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

void
append(char *res[], char *A[], int a_len, char *B[], int b_len) {
    for (int i = 0; i < a_len; i++) res[i] = A[i];
    for (int i = 0; i < b_len; i++) res[i+a_len - 1] = B[i];
}

void
split(char *strs[], char *str, char delim, int* c) {
    int i = 0;
    int j = 0;
    strs[i] = malloc(512 * sizeof(char));
    //char single_str[512];
    char *p = str;
    while (*p) {
        if (*p == delim) {
            strs[i][j++] = 0;
            j = 0;
            strs[++i] = malloc(512 * sizeof(char));
            p++;
            continue;
        }
        strs[i][j++] = *p++;
    }
    strs[i][j++] = 0;
    *c = i + 1;
}

void apply(char *command[], char* additional_args, int argc) {
    char *args[MAXARG];
    int more_argc;
    char *more_args[MAXARG];
    split(more_args, additional_args, ' ', &more_argc);

    //for (int i = 0; i < more_argc; i++) printf("%s\n", more_args[i]);
    if (argc - 1 + more_argc > MAXARG) {
        fprintf(2, "too many args!\n");
    }
    //for (int i = 0; i < argc; i++) printf("%s ", command[i]);
    //for (int i = 0; i < more_argc; i++) printf("%s ", more_args[i]);
    //printf("\n");
    append(args, command, argc, more_args, more_argc);
    args[argc-1+more_argc] = 0;
    //printf("running command:\n");
    //for (int i = 0; i < argc + more_argc - 1; i++) printf("%s ", args[i]);
    //printf("\n");
    if (fork() == 0) {
        // child
        //for (int i = 0; i < argc - 1 + more_argc; i++) printf("%s\n", args[i]);
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
  int i = 0;
  while(read(0, &buf[i++], 1) > 0) {}
  char additional_args[512];
  int j = 0;
  char **command = &argv[1];
  char *p = buf;

  while (*p) {
    if (*p == '\n') {
        additional_args[j] = 0;
        j = 0;
        apply(command, additional_args, argc);
        p++;
        continue;
    }
    //printf("%s\n", additional_args);
    additional_args[j++] = *p++;
  }
  exit(0);
}
