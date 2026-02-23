#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
“Ring” handshake: parent → child1 → child2 → parent
Classic: make a pipeline.
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
  if(argc < 2){ fprintf(2,"usage: hs_ring3 <byte>\n"); exit(1); }
  unsigned char b = (unsigned char)atoi(argv[1]);

  // pipes: P->C1, C1->C2, C2->P
  int p_c1[2], c1_c2[2], c2_p[2];
  pipe(p_c1); pipe(c1_c2); pipe(c2_p);

  int pid1 = fork();
  if(pid1 == 0){
    // child1: read from p_c1[0], write to c1_c2[1]
    close(p_c1[1]);  // don't write P->C1
    close(c1_c2[0]); // don't read C1->C2
    // close unrelated pipe completely
    close(c2_p[0]); close(c2_p[1]);

    unsigned char r;
    readn(p_c1[0], &r, 1);
    printf("%d: child1 got %d\n", getpid(), (int)r);

    r = (unsigned char)(r + 10); // mutate to show it passed through
    writen(c1_c2[1], &r, 1);

    close(p_c1[0]); close(c1_c2[1]);
    exit(0);
  }

  int pid2 = fork();
  if(pid2 == 0){
    // child2: read from c1_c2[0], write to c2_p[1]
    close(c1_c2[1]);
    close(c2_p[0]);
    // close unrelated pipe completely
    close(p_c1[0]); close(p_c1[1]);

    unsigned char r;
    readn(c1_c2[0], &r, 1);
    printf("%d: child2 got %d\n", getpid(), (int)r);

    r = (unsigned char)(r + 20);
    writen(c2_p[1], &r, 1);

    close(c1_c2[0]); close(c2_p[1]);
    exit(0);
  }

  // parent: write to p_c1[1], read from c2_p[0]
  close(p_c1[0]);
  close(c1_c2[0]); close(c1_c2[1]); // parent doesn't use middle pipe
  close(c2_p[1]);

  writen(p_c1[1], &b, 1);

  unsigned char back;
  readn(c2_p[0], &back, 1);
  printf("%d: parent got final %d\n", getpid(), (int)back);

  close(p_c1[1]); close(c2_p[0]);

  wait(0); wait(0);
  exit(0);
}