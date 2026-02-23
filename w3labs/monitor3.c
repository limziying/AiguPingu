#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/syscall.h"   // for SYS_read, SYS_open, ...
#include "user/user.h"

/*
“monitor takes syscall names instead of mask”
Twist tested: mapping names → bit positions.
This is a common test: monitor read,open,close grep hello README
*/

// Very small name->syscall-number map (extend as needed)
static int
sysno_from_name(const char *s)
{
  if(strcmp(s,"read")==0) return SYS_read;
  if(strcmp(s,"write")==0) return SYS_write;
  if(strcmp(s,"open")==0) return SYS_open;
  if(strcmp(s,"close")==0) return SYS_close;
  if(strcmp(s,"fork")==0) return SYS_fork;
  if(strcmp(s,"exec")==0) return SYS_exec;
  if(strcmp(s,"exit")==0) return SYS_exit;
  if(strcmp(s,"wait")==0) return SYS_wait;
  return -1;
}

// Parse a comma-separated list: "read,open,close"
static int
parse_list_to_mask(char *list)
{
  int mask = 0;

  // strtok not always available in xv6; do manual split
  char *p = list;
  while(*p){
    char *start = p;
    while(*p && *p != ',') p++;
    if(*p == ','){ *p = 0; p++; } // terminate token, advance

    int no = sysno_from_name(start);
    if(no >= 0) mask |= (1 << no);
  }

  return mask;
}

int
main(int argc, char *argv[])
{
  // Usage: monitor_names read,open,close command ...
  if(argc < 3){
    fprintf(2, "Usage: %s syscalls_list command [args...]\n", argv[0]);
    exit(1);
  }

  int mask = parse_list_to_mask(argv[1]);

  if(monitor(mask) < 0){
    fprintf(2, "monitor failed\n");
    exit(1);
  }

  char *nargv[MAXARG];
  int i;
  for(i=2; i<argc && i<MAXARG; i++) nargv[i-2] = argv[i];
  nargv[i-2] = 0;

  exec(nargv[0], nargv);
  fprintf(2, "exec failed\n");
  exit(1);
}