#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

/*
Monitor + run command (baseline, your current one)
Twist tested: correct argv shifting + exec.
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
    fprintf(2, "%s: monitor failed\n", argv[0]);
    exit(1);
  }

  // Build argv for the command: argv[2..] becomes nargv[0..]
  char *nargv[MAXARG];
  int i;
  for(i = 2; i < argc && i < MAXARG; i++){
    nargv[i-2] = argv[i];
  }
  nargv[i-2] = 0; // exec requires null-terminated argv array

  exec(nargv[0], nargv);

  // Only runs if exec fails
  fprintf(2, "exec %s failed\n", nargv[0]);
  exit(1);
}