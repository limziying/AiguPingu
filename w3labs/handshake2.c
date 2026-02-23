#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
Handshake with a DIFFERENT ack byte (“OK”)
Common tweak: child returns an acknowledgement token (not the same byte).
*/

static int readn(int fd, void *buf, int n){
  int tot = 0; char *p = buf;
  while(tot < n){
    int m = read(fd, p+tot, n-tot);
    if(m <= 0) return m; // 0 EOF, -1 error
    tot += m;
  }
  return tot;
}
static int writen(int fd, const void *buf, int n){
  int tot = 0; const char *p = buf;
  while(tot < n){
    int m = write(fd, p+tot, n-tot);
    if(m <= 0) return m;
    tot += m;
  }
  return tot;
}

int main(int argc, char *argv[]){
  if(argc < 2){ fprintf(2,"usage: hs_ack <byte>\n"); exit(1); }

  int p2c[2], c2p[2];
  pipe(p2c); pipe(c2p);

  char b = (char)atoi(argv[1]);
  char ack = 'K'; // choose a fixed ack token

  int pid = fork();
  if(pid == 0){
    close(p2c[1]); close(c2p[0]);

    char r;
    if(readn(p2c[0], &r, 1) != 1){ fprintf(2,"child read fail\n"); exit(1); }
    printf("%d: child got %d\n", getpid(), (int)(unsigned char)r);

    // child replies with ACK, not the original byte
    if(writen(c2p[1], &ack, 1) != 1){ fprintf(2,"child write fail\n"); exit(1); }

    close(p2c[0]); close(c2p[1]);
    exit(0);
  }

  close(p2c[0]); close(c2p[1]);

  writen(p2c[1], &b, 1);

  char r;
  if(readn(c2p[0], &r, 1) != 1){ fprintf(2,"parent read fail\n"); exit(1); }

  // parent verifies ACK
  if(r != ack){
    printf("%d: parent expected ACK '%c' but got '%c'\n", getpid(), ack, r);
  } else {
    printf("%d: parent got ACK '%c'\n", getpid(), r);
  }

  close(p2c[1]); close(c2p[0]);
  wait(0);
  exit(0);
}