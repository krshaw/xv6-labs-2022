
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // create a pair of pipes, one for each direction
  // use pipe to create a pipe
  // use fork to create a child
  // remember: [0] is read, [1] is write
  int p[2];
  pipe(p);
  if (fork() == 0) {
      // in child process
      char byte;
      read(p[0], &byte, 1);
      printf("%d: received ping\n", getpid());
      write(p[1], &byte, 1);
      close(p[1]);
      exit(0);
  } else { // in parent process
      char byte = 1;
      write(p[1], &byte, 1);
      char read_byte;
      read(p[0], &read_byte, 1);
      printf("%d: received pong\n", getpid());
      close(p[0]);
      exit(0);
  }
  exit(0);
}
