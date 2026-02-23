#include "kernel/types.h"
#include "user/user.h"

/*
Targeted Secret Verification Mode
Why it's tested: The assignment might ask you to build a tool that verifies
if a specific secret exists in memory, rather than just printing whatever it finds.
*/
int main(int argc, char *argv[]) {
  if(argc != 2) {
    printf("Usage: sniffer <target-secret>\n");
    exit(1);
  }

  int size = 10 * 4096;
  char *mem = sbrk(size);
  char *target = argv[1];
  
  for(int i = 0; i < size - strlen(target); i++) {
    // We just search straight for the target secret!
    if(strcmp(mem + i, target) == 0) {
      printf("SUCCESS: Found target '%s' in memory at offset %d\n", target, i);
      exit(0);
    }
  }
  
  printf("FAILURE: Target not found.\n");
  exit(0);
}