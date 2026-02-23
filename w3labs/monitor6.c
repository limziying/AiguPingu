#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

/*
“Run command in a child; parent stays as monitor wrapper”
Twist tested: inheritance via fork + parent/child roles.
*/
int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "Usage: %s mask command [args...]\n", argv[0]);
    exit(1);
  }

  int mask = atoi(argv[1]);
  if(monitor(mask) < 0){
    fprintf(2, "monitor failed\n");
    exit(1);
  }

  char *nargv[MAXARG];
  int i;
  for(i=2; i<argc && i<MAXARG; i++) nargv[i-2] = argv[i];
  nargv[i-2] = 0;

  int pid = fork();
  if(pid < 0){
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if(pid == 0){
    // Child inherits monitor_mask (because kernel copies in fork).
    exec(nargv[0], nargv);
    fprintf(2, "exec failed\n");
    exit(1);
  }

  // Parent can wait & exit cleanly
  wait(0);
  exit(0);
}