#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

/*
“Disable tracing” (mask = 0) + run command
Twist tested: turning off tracing cleanly.
*/
int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: %s off|mask [command...]\n", argv[0]);
    exit(1);
  }

  int mask;
  if(strcmp(argv[1], "off") == 0) mask = 0;
  else mask = atoi(argv[1]);

  if(monitor(mask) < 0){
    fprintf(2, "monitor failed\n");
    exit(1);
  }

  // If no command provided, just exit after setting.
  if(argc == 2) exit(0);

  char *nargv[MAXARG];
  int i;
  for(i=2; i<argc && i<MAXARG; i++) nargv[i-2] = argv[i];
  nargv[i-2] = 0;
  exec(nargv[0], nargv);
  fprintf(2, "exec failed\n");
  exit(1);
}