#include "kernel/types.h"
#include "user/user.h"

/*
“Support -p (monitor current shell without exec)”
Twist tested: enabling tracing for current process only (no command run).
*/
int
main(int argc, char *argv[])
{
  // Example: monitor_setonly 32
  if(argc != 2){
    fprintf(2, "Usage: %s mask\n", argv[0]);
    exit(1);
  }

  int mask = atoi(argv[1]);
  if(monitor(mask) < 0){
    fprintf(2, "monitor failed\n");
    exit(1);
  }

  // No exec: just exits after setting mask.
  // If you run this as a child of sh, it won't affect sh unless sh itself calls it.
  exit(0);
}