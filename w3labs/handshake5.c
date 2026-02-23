#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
Reverse handshake (“child ready” first)
Sometimes they swap roles: child must signal “READY” before parent sends.
*/

static int readn(int fd, void *buf, int n){
  int tot=0; char *p=buf;
  while(tot<n){
    int m=read(fd,p+tot,n-tot);
    if(m<=0) return m;
    tot+=m;
  }
  return tot;
}
static int writen(int fd, const void *buf, int n){
  int tot=0; const char *p=buf;
  while(tot<n){
    int m=write(fd,p+tot,n-tot);
    if(m<=0) return m;
    tot+=m;
  }
  return tot;
}

int main(int argc, char *argv[]){
  if(argc < 2){ fprintf(2,"usage: hs_ready_first <byte>\n"); exit(1); }

  unsigned char b = (unsigned char)atoi(argv[1]);
  char READY = 'R';

  int p2c[2], c2p[2];
  pipe(p2c); pipe(c2p);

  int pid=fork();
  if(pid==0){
    close(p2c[1]); close(c2p[0]);

    // Child announces readiness to parent
    writen(c2p[1], &READY, 1);

    // Then child receives real data
    unsigned char r;
    readn(p2c[0], &r, 1);
    printf("%d: child got %d after READY\n", getpid(), (int)r);

    close(p2c[0]); close(c2p[1]);
    exit(0);
  }

  close(p2c[0]); close(c2p[1]);

  // Parent waits for READY first
  char rdy;
  readn(c2p[0], &rdy, 1);
  printf("%d: parent got READY '%c'\n", getpid(), rdy);

  // Then parent sends the byte
  writen(p2c[1], &b, 1);

  close(p2c[1]); close(c2p[0]);
  wait(0);
  exit(0);
}