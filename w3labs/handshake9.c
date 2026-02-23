#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
Handshake with “status code” (OK / ERROR)
Common tweak: child validates and returns success/fail.
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
  if(argc < 2){ fprintf(2,"usage: hs_status <byte>\n"); exit(1); }

  int p2c[2], c2p[2];
  pipe(p2c); pipe(c2p);

  unsigned char b = (unsigned char)atoi(argv[1]);

  int pid=fork();
  if(pid==0){
    close(p2c[1]); close(c2p[0]);

    unsigned char r;
    readn(p2c[0], &r, 1);

    // Example rule: only accept even bytes
    unsigned char status = (r % 2 == 0) ? 0 : 1; // 0=OK, 1=ERR

    printf("%d: child got %d -> status %d\n", getpid(), (int)r, (int)status);
    writen(c2p[1], &status, 1);

    close(p2c[0]); close(c2p[1]);
    exit(0);
  }

  close(p2c[0]); close(c2p[1]);

  writen(p2c[1], &b, 1);

  unsigned char st;
  readn(c2p[0], &st, 1);

  if(st == 0) printf("%d: parent OK\n", getpid());
  else        printf("%d: parent ERROR (child rejected byte)\n", getpid());

  close(p2c[1]); close(c2p[0]);
  wait(0);
  exit(0);
}