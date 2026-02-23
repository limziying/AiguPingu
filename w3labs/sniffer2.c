#include "kernel/types.h"
#include "user/user.h"

/*
Incremental Page-by-Page Allocation
Why it's tested: sbrk(10 * 4096) allocates a massive chunk at once. 
A professor might restrict how much you can sbrk at a single time to 
test your understanding of pagination.
*/
int main(int argc, char *argv[]) {
  char *pattern = "This may help.";
  
  // Allocate and check one page (4096 bytes) at a time
  for(int p = 0; p < 10; p++) {
    char *page = sbrk(4096);
    if(page == (char*)-1) exit(1);

    // Search only within this newly allocated page
    for(int i = 0; i < 4096 - 64; i++) {
      if(strcmp(page + i, pattern) == 0) {
        printf("%s\n", page + i + 16);
        exit(0);
      }
    }
  }
  
  printf("sniffer: secret not found\n");
  exit(0);
}
