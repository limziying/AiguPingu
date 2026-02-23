#include "kernel/types.h"
#include "user/user.h"

/*
The malloc() Wrapper
Why it's tested: To see if you understand that user-space malloc is just a wrapper 
around the system call sbrk. Note that malloc might add metadata headers, 
so the layout might be slightly shifted, but the byte-scan will still work.
*/
int main(int argc, char *argv[]) {
  int size = 10 * 4096;
  
  // Using malloc instead of sbrk
  char *mem = malloc(size);
  if(mem == 0) exit(1); // malloc returns 0 (NULL) on failure in xv6

  char *pattern = "This may help.";
  
  for(int i = 0; i < size - 64; i++) {
    if(strcmp(mem + i, pattern) == 0) {
      printf("%s\n", mem + i + 16);
      free(mem); // Good practice to free, though exit clears it anyway
      exit(0);
    }
  }
  
  free(mem);
  exit(0);
}