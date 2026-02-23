#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

/*
“Multiple commands” (monitor then run two commands sequentially)
Twist tested: exec replaces process, so you must fork for each command.
*/
// Usage example:
// monitor_two 32 "grep hello README" "grep world README"
//
// xv6 doesn't have a full shell parser here; simplest is to hardcode
// or pass command+args separately in real use. This is just study structure.

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "Usage: %s mask cmd1 [cmd1args...] -- cmd2 [cmd2args...]\n", argv[0]);
    exit(1);
  }

  int mask = atoi(argv[1]);
  if(monitor(mask) < 0){
    fprintf(2, "monitor failed\n");
    exit(1);
  }

  // Split argv by "--"
  int sep = -1;
  for(int i=2;i<argc;i++){
    if(strcmp(argv[i],"--")==0){ sep=i; break; }
  }
  if(sep == -1){
    fprintf(2, "need -- separator\n");
    exit(1);
  }

  char *cmd1[MAXARG], *cmd2[MAXARG];
  int c1=0, c2=0;

  for(int i=2; i<sep && c1<MAXARG-1; i++) cmd1[c1++] = argv[i];
  cmd1[c1] = 0;

  for(int i=sep+1; i<argc && c2<MAXARG-1; i++) cmd2[c2++] = argv[i];
  cmd2[c2] = 0;

  // Run cmd1 in a child
  if(fork()==0){
    exec(cmd1[0], cmd1);
    exit(1);
  }
  wait(0);

  // Run cmd2 in a child (still inherits mask from parent)
  if(fork()==0){
    exec(cmd2[0], cmd2);
    exit(1);
  }
  wait(0);

  exit(0);
}