#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
Basic 1 byte handshake
*/
// Write exactly n bytes (robust helper)
static int writen(int fd, const void *buf, int n) {
  int tot = 0;
  const char *p = (const char*)buf;
  while (tot < n) {
    int m = write(fd, p + tot, n - tot);
    if (m < 0) return -1;
    if (m == 0) break; // unusual for write, but keep safe
    tot += m;
  }
  return tot;
}

// Read exactly n bytes (robust helper)
static int readn(int fd, void *buf, int n) {
  int tot = 0;
  char *p = (char*)buf;
  while (tot < n) {
    int m = read(fd, p + tot, n - tot);
    if (m < 0) return -1;
    if (m == 0) break; // EOF
    tot += m;
  }
  return tot;
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: hs_basic <byte>\n");
    exit(1);
  }

  int p2c[2], c2p[2];
  if (pipe(p2c) < 0 || pipe(c2p) < 0) {
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  int x = atoi(argv[1]);
  char b = (char)x;

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // CHILD: read from p2c[0], write to c2p[1]
    close(p2c[1]);
    close(c2p[0]);

    char r;
    if (readn(p2c[0], &r, 1) != 1) {
      fprintf(2, "child read failed\n");
      exit(1);
    }

    printf("%d: child got %d\n", getpid(), (int)(unsigned char)r);

    if (writen(c2p[1], &r, 1) != 1) {
      fprintf(2, "child write failed\n");
      exit(1);
    }

    close(p2c[0]);
    close(c2p[1]);
    exit(0);
  }

  // PARENT: write to p2c[1], read from c2p[0]
  close(p2c[0]);
  close(c2p[1]);

  if (writen(p2c[1], &b, 1) != 1) {
    fprintf(2, "parent write failed\n");
    exit(1);
  }

  char r;
  if (readn(c2p[0], &r, 1) != 1) {
    fprintf(2, "parent read failed\n");
    exit(1);
  }

  printf("%d: parent got %d\n", getpid(), (int)(unsigned char)r);

  close(p2c[1]);
  close(c2p[0]);

  wait(0);
  exit(0);
}