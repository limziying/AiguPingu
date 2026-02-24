#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#define NBUCKET 5
#define NKEYS 100000

/*
 * VARIATION 2: SAFE CONCURRENT READS
 * * WHY THIS IS TESTED:
 * In the original lab, `get()` runs AFTER all `put()` operations finish. 
 * What if a server receives GET and PUT requests at the same time?
 * If `get()` traverses a linked list while `put()` is modifying pointers, 
 * `get()` will crash (segfault) or miss data. We must lock `get()` too!
 */
pthread_mutex_t locks[NBUCKET];

struct entry {
  int key;
  int value;
  struct entry *next;
};

struct entry *table[NBUCKET];
int keys[NKEYS];
int nthread = 1;

double now() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static void insert(int key, int value, struct entry **p, struct entry *n) {
  struct entry *e = malloc(sizeof(struct entry));
  e->key = key; e->value = value;
  e->next = n;
  *p = e;
}

static void put(int key, int value) {
  int i = key % NBUCKET;
  pthread_mutex_lock(&locks[i]);
  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) {
    if (e->key == key) break;
  }
  if(e) {
    e->value = value;
  } else {
    insert(key, value, &table[i], table[i]);
  }
  pthread_mutex_unlock(&locks[i]);
}

static struct entry* get(int key) {
  int i = key % NBUCKET;
  struct entry *e = 0;
  
  // SECURING THE READ: Lock the bucket before searching!
  // This prevents `put()` from modifying the list while we read it.
  pthread_mutex_lock(&locks[i]);
  
  for (e = table[i]; e != 0; e = e->next) {
    if (e->key == key) break;
  }
  
  // CRITICAL: We must unlock before returning. If we return `e` and 
  // forget to unlock, the bucket will be permanently locked!
  pthread_mutex_unlock(&locks[i]);
  return e;
}

static void *put_thread(void *xa) {
  int n = (int) (long) xa;
  int b = NKEYS/nthread;
  for (int i = 0; i < b; i++) {
    put(keys[b*n + i], n);
  }
  return NULL;
}

static void *get_thread(void *xa) {
  int n = (int) (long) xa;
  int missing = 0;
  for (int i = 0; i < NKEYS; i++) {
    struct entry *e = get(keys[i]);
    if (e == 0) missing++;
  }
  printf("%d: %d keys missing\n", n, missing);
  return NULL;
}

int main(int argc, char *argv[]) {
  pthread_t *tha;
  void *value;
  double t1, t0;

  for (int i = 0; i < NBUCKET; i++){
    pthread_mutex_init(&locks[i], NULL);
  }
  
  if (argc < 2) { exit(-1); }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread * 2); // Space for put and get threads

  srandom(0);
  for (int i = 0; i < NKEYS; i++) { keys[i] = random(); }

  t0 = now();
  // We can now run puts and gets SIMULTANEOUSLY safely!
  for(int i = 0; i < nthread; i++) {
    pthread_create(&tha[i], NULL, put_thread, (void *) (long) i);
    pthread_create(&tha[nthread + i], NULL, get_thread, (void *) (long) i);
  }
  
  for(int i = 0; i < nthread * 2; i++) {
    pthread_join(tha[i], &value);
  }
  t1 = now();
  
  printf("Concurrent execution finished in %.3f seconds\n", t1 - t0);

  for (int i = 0; i < NBUCKET; i++){
    pthread_mutex_destroy(&locks[i]);
  }
  return 0;       
}