
#include "kernel/types.h"
#include "user/user.h"

void
start_sieve(int p[]) {
    if (p == 0) return;
    close(p[1]);
    int num, prime, n;
    n = read(p[0], &num, 4);
    if (n == 0) exit(0);
    prime = num;
    printf("prime %d\n", prime);
    // try creating multiple pipes, if run into resource issues, try using same pipe
    int next_pipe[2];
    pipe(next_pipe);
    if (fork() == 0) {
        close(p[0]);
        start_sieve(next_pipe);
        exit(0);
    } else {
        close(next_pipe[0]);
        while (read(p[0], &num, 4) > 0) {
            //printf("got %d\n", num);
            // "if p does not divide n, send n to right neighbor"
            if (num % prime != 0) {
                write(next_pipe[1], &num, 4);
            }
        }
        close(p[0]);
        close(next_pipe[1]);
        wait(0);
        exit(0);
    }
}

int
main(int argc, char *argv[])
{
  // use pipe and fork to setup the pipeline
  // first process feeds numbers 2 through 35 into the pipeline
  // for each prime, create one process that reads from its left neighbor over a pipe and writes to its right neighbor over another pipe
  // once first process reaches 35, it should wait until the entire pipeline terminates, all children, grandchildren, etc.
  // main primes process should only exit after all output has been printed, and after all primes processes have exited
  // each sub prime process should only exit once its child has exited
  // thus just waiting on the direct child is sufficient for our needs
  int p[2];
  pipe(p);
  if (fork() == 0) {
      // child stuff
      start_sieve(p);
  } else {
      close(p[0]);
      //int num;
      for (int i = 2; i < 36; i++) {
        write(p[1], &i, 4);
      }
      close(p[1]);
      wait(0);
      exit(0);
      // parent stuff (feeding nums to the pipe)
  }
  exit(0);
}
