#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
“Handshake barrier” with 2-step confirmation (send → ack → confirm)
Common lab idea: require two acknowledgements to prove both sides reached a point.
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
  if(argc < 2){ fprintf(2,"usage: hs_two_phase <byte>\n"); exit(1); }
  unsigned char b = (unsigned char)atoi(argv[1]);

  int p2c[2], c2p[2];
  pipe(p2c); pipe(c2p);

  unsigned char ACK1 = 'A';
  unsigned char ACK2 = 'B';

  int pid=fork();
  if(pid==0){
    close(p2c[1]); close(c2p[0]);

    // Phase 1: receive data
    unsigned char r;
    readn(p2c[0], &r, 1);
    printf("%d: child got %d\n", getpid(), (int)r);

    // Reply with ACK1
    writen(c2p[1], &ACK1, 1);

    // Phase 2: wait for parent's confirmation ACK2
    unsigned char conf;
    readn(p2c[0], &conf, 1);
    if(conf == ACK2) printf("%d: child got confirm ACK2\n", getpid());
    else printf("%d: child got wrong confirm %d\n", getpid(), (int)conf);

    close(p2c[0]); close(c2p[1]);
    exit(0);
  }

  close(p2c[0]); close(c2p[1]);

  // Phase 1: send data
  writen(p2c[1], &b, 1);

  // Wait for ACK1
  unsigned char a;
  readn(c2p[0], &a, 1);
  printf("%d: parent got ACK1 '%c'\n", getpid(), a);

  // Phase 2: send confirmation ACK2
  writen(p2c[1], &ACK2, 1);

  close(p2c[1]); close(c2p[0]);
  wait(0);
  exit(0);
}