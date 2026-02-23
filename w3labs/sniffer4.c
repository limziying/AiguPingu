#include "kernel/types.h"
#include "user/user.h"

/*
The Fuzzy Offset Scanner (Unknown Offset)
Why it's tested: What if the professor changes
strcpy(data + 16, argv[1]) to strcpy(data + 32, argv[1]) 
during the exam? This variation finds the hint, then scans forward
to find the first printable character of the secret, making it offset-proof.
*/

int main(int argc, char *argv[]) {
  int size = 10 * 4096;
  char *mem = sbrk(size);
  if(mem == (char*)-1) exit(1);

  char *pattern = "This may help.";
  
  for(int i = 0; i < size - 100; i++) {
    if(strcmp(mem + i, pattern) == 0) {
      
      // We found the hint. Now, let's scan forward past the hint.
      int j = i + strlen(pattern) + 1; 
      
      // Skip all null bytes or unprintable garbage until we hit the secret
      while((mem[j] < 32 || mem[j] > 126) && j < i + 64) {
        j++;
      }
      
      // j is now pointing at the start of the secret
      printf("%s\n", mem + j);
      exit(0);
    }
  }
  exit(0);
}