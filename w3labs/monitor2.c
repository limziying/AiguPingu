#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

/*
“monitor should accept hex masks” (e.g., 0x7fffffff)
Twist tested: parsing (not just atoi).
*/

// Parse decimal or hex (0x...)
static int
parse_mask(const char *s)
{
  int mask = 0;
  if(s[0]=='0' && (s[1]=='x' || s[1]=='X')){
    // hex parse manually (xv6 doesn't always have strtol)
    for(int i=2; s[i]; i++){
      char c = s[i];
      int v;
      if(c>='0' && c<='9') v = c - '0';
      else if(c>='a' && c<='f') v = 10 + (c - 'a');
      else if(c>='A' && c<='F') v = 10 + (c - 'A');
      else break;
      mask = (mask << 4) | v;
    }
    return mask;
  }
  return atoi(s);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "Usage: %s mask command [args...]\n", argv[0]);
    exit(1);
  }

  int mask = parse_mask(argv[1]);

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