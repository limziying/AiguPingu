#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
Handshake with a small “packet” (2 bytes: seq + payload)
Common tweak: send more than 1 byte and require exact structure.
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

struct Packet2 {
  unsigned char seq;     // sequence number
  unsigned char payload; // data
};

int main(int argc, char *argv[]){
  if(argc < 3){
    fprintf(2,"usage: hs_packet2 <seq 0-255> <payload 0-255>\n");
    exit(1);
  }

  int p2c[2], c2p[2];
  pipe(p2c); pipe(c2p);

  struct Packet2 pkt;
  pkt.seq = (unsigned char)atoi(argv[1]);
  pkt.payload = (unsigned char)atoi(argv[2]);

  int pid = fork();
  if(pid == 0){
    close(p2c[1]); close(c2p[0]);

    struct Packet2 r;
    if(readn(p2c[0], &r, sizeof(r)) != sizeof(r)){
      fprintf(2,"child read packet failed\n");
      exit(1);
    }

    printf("%d: child got seq=%d payload=%d\n",
      getpid(), (int)r.seq, (int)r.payload);

    // ack: send back same seq only (1 byte) to confirm correct packet
    if(writen(c2p[1], &r.seq, 1) != 1){
      fprintf(2,"child ack failed\n");
      exit(1);
    }

    close(p2c[0]); close(c2p[1]);
    exit(0);
  }

  close(p2c[0]); close(c2p[1]);

  writen(p2c[1], &pkt, sizeof(pkt));

  unsigned char ack;
  if(readn(c2p[0], &ack, 1) != 1){
    fprintf(2,"parent ack read failed\n");
    exit(1);
  }

  printf("%d: parent got ack seq=%d\n", getpid(), (int)ack);

  close(p2c[1]); close(c2p[0]);
  wait(0);
  exit(0);
}