#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
Use EOF as the “done” signal (no ack byte)
Common trick: closing the write end causes read() to return 0 (EOF).
*/

int main(int argc, char *argv[]){
  int p2c[2];
  if(pipe(p2c) < 0){ fprintf(2,"pipe fail\n"); exit(1); }

  int pid = fork();
  if(pid == 0){
    // child only reads until EOF
    close(p2c[1]);

    int count = 0;
    unsigned char r;
    while(1){
      int n = read(p2c[0], &r, 1);
      if(n < 0){ fprintf(2,"child read err\n"); exit(1); }
      if(n == 0) break; // EOF means parent closed write end
      count++;
      printf("%d: child got %d (msg #%d)\n", getpid(), (int)r, count);
    }

    printf("%d: child saw EOF, total=%d\n", getpid(), count);
    close(p2c[0]);
    exit(0);
  }

  // parent writes multiple bytes then closes to signal done
  close(p2c[0]);

  // send 3 bytes as an example
  unsigned char a = 1, b = 2, c = 3;
  write(p2c[1], &a, 1);
  write(p2c[1], &b, 1);
  write(p2c[1], &c, 1);

  // This close is the "handshake done" signal
  close(p2c[1]);

  wait(0);
  exit(0);
}