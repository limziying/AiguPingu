#include "kernel/types.h"
#include "user/user.h"

/*
Fast Page-Hop (The "Skip Nulls" Optimizer)
Why it's tested: Real-world exploits need 
to be fast. If a page is entirely zeroed out 
(e.g. from a different process that didn't 
trigger the bug), scanning it byte-by-byte wastes time. 
We can check the first byte; if the page is dead, we skip 
the whole 4096 bytes.
*/

int main(int argc, char *argv[]) {
  int size = 10 * 4096;
  char *mem = sbrk(size);
  char *pattern = "This may help.";
  
  for(int i = 0; i < size - 64; ) {
    
    // Quick heuristic: If this 64-byte chunk is entirely null, 
    // we jump forward significantly to save CPU cycles.
    if(mem[i] == 0 && mem[i+10] == 0 && mem[i+20] == 0) {
      i += 32; // Jump forward faster
      continue;
    }

    if(strcmp(mem + i, pattern) == 0) {
      printf("%s\n", mem + i + 16);
      exit(0);
    }
    i++;
  }
  exit(0);
}