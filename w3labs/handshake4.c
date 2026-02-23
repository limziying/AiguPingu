#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
Ping-pong handshake N times
Common tweak: repeat the handshake loop and keep order correct.
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
  if(argc < 3){
    fprintf(2,"usage: hs_pingpong <byte> <count>\n");
    exit(1);
  }

  unsigned char b = (unsigned char)atoi(argv[1]);
  int count = atoi(argv[2]);

  int p2c[2], c2p[2];
  pipe(p2c); pipe(c2p);

  int pid = fork();
  if(pid == 0){
    close(p2c[1]); close(c2p[0]);

    for(int i=0;i<count;i++){
      unsigned char r;
      if(readn(p2c[0], &r, 1) != 1) exit(1);
      // child increments the byte each round (visible change)
      r = (unsigned char)(r + 1);
      if(writen(c2p[1], &r, 1) != 1) exit(1);
    }

    close(p2c[0]); close(c2p[1]);
    exit(0);
  }

  close(p2c[0]); close(c2p[1]);

  unsigned char cur = b;
  for(int i=0;i<count;i++){
    writen(p2c[1], &cur, 1);
    readn(c2p[0], &cur, 1);
    printf("%d: round %d got %d\n", getpid(), i, (int)cur);
  }

  close(p2c[1]); close(c2p[0]);
  wait(0);
  exit(0);
}