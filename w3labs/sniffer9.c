#include "kernel/types.h"
#include "user/user.h"

/*
Blind Heuristic Search (No Hint Pattern)
Why it's tested: What if secret.c is modified to remove the "This may help." 
hint? You now have to guess where the secret is by looking for contiguous 
blocks of readable text.
*/

int main(int argc, char *argv[]) {
  int size = 10 * 4096;
  char *mem = sbrk(size);
  if(mem == (char*)-1) exit(1);

  for(int i = 0; i < size - 20; i++) {
    // Check if we found a printable ASCII character
    if(mem[i] >= 'A' && mem[i] <= 'z') {
      
      // Let's assume the secret is at least 5 characters long
      int is_string = 1;
      for(int j = 0; j < 5; j++) {
        if(mem[i+j] < 32 || mem[i+j] > 126) {
          is_string = 0; // Hit a null or garbage byte too early
          break;
        }
      }
      
      if(is_string) {
        printf("Found potential secret: %s\n", mem + i);
        // We don't exit here, we print ALL potential secrets we find!
        i += strlen(mem + i); // Skip past this string
      }
    }
  }
  exit(0);
}