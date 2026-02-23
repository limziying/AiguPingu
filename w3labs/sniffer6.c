#include "kernel/types.h"
#include "user/user.h"

/*
No-Library Manual String Matcher
Why it's tested: Sometimes professors forbid the use of standard
library functions like strcmp to test your ability to manipulate 
pointers and arrays manually.
*/
int main(int argc, char *argv[]) {
  int size = 10 * 4096;
  char *mem = sbrk(size);
  if(mem == (char*)-1) exit(1);

  char *pattern = "This may help.";
  int pat_len = 14; // Length of the pattern
  
  for(int i = 0; i < size - 64; i++) {
    int match = 1;
    
    // Manual character-by-character comparison
    for(int j = 0; j < pat_len; j++) {
      if(mem[i + j] != pattern[j]) {
        match = 0;
        break; // Mismatch, break early
      }
    }
    
    if(match) {
      printf("%s\n", mem + i + 16);
      exit(0);
    }
  }
  exit(0);
}
