#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
Parent sends to TWO children and waits for BOTH acks
Common lab twist: multiple children, collect acknowledgements.
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

// Create one child that does: read 1 byte -> print -> write ack byte back
static int spawn_child(int p2c[2], int c2p[2]) {
  int pid = fork();
  if(pid == 0){
    close(p2c[1]); close(c2p[0]);

    unsigned char r;
    if(readn(p2c[0], &r, 1) != 1) exit(1);
    printf("%d: child got %d\n", getpid(), (int)r);

    // ack = r (echo)
    if(writen(c2p[1], &r, 1) != 1) exit(1);

    close(p2c[0]); close(c2p[1]);
    exit(0);
  }
  return pid;
}

int main(int argc, char *argv[]){
  if(argc < 2){ fprintf(2,"usage: hs_two_children <byte>\n"); exit(1); }
  unsigned char b = (unsigned char)atoi(argv[1]);

  // one pipe pair per child
  int p2c1[2], c2p1[2], p2c2[2], c2p2[2];
  pipe(p2c1); pipe(c2p1);
  pipe(p2c2); pipe(c2p2);

  int pid1 = spawn_child(p2c1, c2p1);
  int pid2 = spawn_child(p2c2, c2p2);

  // Parent closes unused ends (important!)
  close(p2c1[0]); close(c2p1[1]);
  close(p2c2[0]); close(c2p2[1]);

  // Send to both children
  writen(p2c1[1], &b, 1);
  writen(p2c2[1], &b, 1);

  // Read both acks
  unsigned char a1, a2;
  readn(c2p1[0], &a1, 1);
  readn(c2p2[0], &a2, 1);

  printf("%d: parent got ack1=%d ack2=%d\n", getpid(), (int)a1, (int)a2);

  close(p2c1[1]); close(c2p1[0]);
  close(p2c2[1]); close(c2p2[0]);

  wait(0);
  wait(0);
  exit(0);
}