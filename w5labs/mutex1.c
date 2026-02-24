#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#define NBUCKET 5
#define NKEYS 100000

/* * VARIATION 1: COARSE-GRAINED LOCKING (GLOBAL LOCK)
 * Instead of an array of locks, we use a single lock for the entire hash table.
 * * WHY THIS IS TESTED:
 * Professors use this to demonstrate that adding locks doesn't automatically mean 
 * "parallel speedup". Because every thread must acquire this single global lock 
 * for EVERY insert, the program becomes fully sequential. 2 threads will run 
 * slower than 1 thread due to lock contention overhead.
 */
pthread_mutex_t global_lock;

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
  
  // LOCK: One lock to rule them all. If Thread A accesses bucket 0, 
  // Thread B CANNOT access bucket 1 because the global lock is taken.
  pthread_mutex_lock(&global_lock);

  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) {
    if (e->key == key) break;
  }
  if(e) {
    e->value = value;
  } else {
    insert(key, value, &table[i], table[i]);
  }
  
  // UNLOCK: Let the next thread in.
  pthread_mutex_unlock(&global_lock);
}

static struct entry* get(int key) {
  int i = key % NBUCKET;
  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) {
    if (e->key == key) break;
  }
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

  // Initialize the single global lock
  pthread_mutex_init(&global_lock, NULL);
  
  if (argc < 2) {
    fprintf(stderr, "Usage: %s nthreads\n", argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);

  srandom(0);
  assert(NKEYS % nthread == 0);
  for (int i = 0; i < NKEYS; i++) {
    keys[i] = random();
  }

  t0 = now();
  for(int i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, put_thread, (void *) (long) i) == 0);
  }
  for(int i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  t1 = now();
  printf("%d puts, %.3f seconds, %.0f puts/second\n", NKEYS, t1 - t0, NKEYS / (t1 - t0));

  t0 = now();
  for(int i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, get_thread, (void *) (long) i) == 0);
  }
  for(int i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  t1 = now();
  printf("%d gets, %.3f seconds, %.0f gets/second\n", NKEYS*nthread, t1 - t0, (NKEYS*nthread) / (t1 - t0));

  // Destroy the global lock
  pthread_mutex_destroy(&global_lock);
  return 0;       
}