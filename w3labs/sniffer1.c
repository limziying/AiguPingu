#include "kernel/types.h"
#include "user/user.h"

/*
The "Struct Overlay" Method (Cleanest C Approach)
Why it's tested: Professors love seeing idiomatic C code. 
Instead of using "magic numbers" like mem + i + 16, we can define a struct 
that maps exactly to the expected memory layout
*/

// Define the exact layout we expect to find in memory
struct SecretLayout {
  char hint[16];   // "This may help." + null terminators take up 16 bytes
  char secret[64]; // The secret follows immediately
};

int main(int argc, char *argv[]) {
  int size = 10 * 4096;
  char *mem = sbrk(size);
  
  if(mem == (char*)-1) exit(1);

  // We stop early enough so the struct doesn't read out of bounds
  for(int i = 0; i < size - sizeof(struct SecretLayout); i++) {
    // Cast the raw memory pointer to our struct type
    struct SecretLayout *layout = (struct SecretLayout *)(mem + i);
    
    // Check if the hint matches
    if(strcmp(layout->hint, "This may help.") == 0) {
      // If it does, the secret is naturally accessible via the struct!
      printf("%s\n", layout->secret);
      exit(0);
    }
  }
  printf("sniffer: secret not found\n");
  exit(0);
}
