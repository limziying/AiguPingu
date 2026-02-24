#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#define NBUCKET 5
#define NKEYS 100000

/*
 * VARIATION 3: READ-WRITE LOCKS (pthread_rwlock_t)
 * * WHY THIS IS TESTED:
 * Standard mutexes block EVERYONE. If 5 threads want to safely read bucket 0, 
 * they have to wait for each other. 
 * RW-Locks allow MULTIPLE readers at the same time, but Writers get EXCLUSIVE access.
 * This is the ultimate optimization for read-heavy programs.
 */
pthread_rwlock_t rwlocks[NBUCKET]; // Array of Read-Write locks

struct entry { int key; int value; struct entry *next; };
struct entry *table[NBUCKET];
int keys[NKEYS];
int nthread = 1;

double now() {
  struct timeval tv; gettimeofday(&tv, 0);
  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static void insert(int key, int value, struct entry **p, struct entry *n) {
  struct entry *e = malloc(sizeof(struct entry));
  e->key = key; e->value = value; e->next = n; *p = e;
}

static void put(int key, int value) {
  int i = key % NBUCKET;
  
  // WRITER LOCK: Blocks other writers AND readers. 
  // Modifying the list must be done alone.
  pthread_rwlock_wrlock(&rwlocks[i]);

  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) {
    if (e->key == key) break;
  }
  if(e) e->value = value;
  else insert(key, value, &table[i], table[i]);
  
  pthread_rwlock_unlock(&rwlocks[i]);
}

static struct entry* get(int key) {
  int i = key % NBUCKET;
  struct entry *e = 0;
  
  // READER LOCK: Blocks writers, but allows OTHER readers to lock simultaneously!
  pthread_rwlock_rdlock(&rwlocks[i]);
  
  for (e = table[i]; e != 0; e = e->next) {
    if (e->key == key) break;
  }
  
  pthread_rwlock_unlock(&rwlocks[i]);
  return e;
}

static void *put_thread(void *xa) {
  int n = (int) (long) xa; int b = NKEYS/nthread;
  for (int i = 0; i < b; i++) put(keys[b*n + i], n);
  return NULL;
}

static void *get_thread(void *xa) {
  int n = (int) (long) xa; int missing = 0;
  for (int i = 0; i < NKEYS; i++) {
    struct entry *e = get(keys[i]);
    if (e == 0) missing++;
  }
  printf("%d: %d keys missing\n", n, missing);
  return NULL;
}

int main(int argc, char *argv[]) {
  pthread_t *tha; void *value; double t1, t0;

  // Initialize Read-Write Locks
  for (int i = 0; i < NBUCKET; i++){
    pthread_rwlock_init(&rwlocks[i], NULL);
  }
  
  if (argc < 2) exit(-1);
  nthread = atoi(argv[1]); tha = malloc(sizeof(pthread_t) * nthread);

  srandom(0);
  for (int i = 0; i < NKEYS; i++) keys[i] = random();

  t0 = now();
  for(int i = 0; i < nthread; i++) pthread_create(&tha[i], NULL, put_thread, (void *) (long) i);
  for(int i = 0; i < nthread; i++) pthread_join(tha[i], &value);
  t1 = now();
  printf("Puts done: %.3f seconds\n", t1 - t0);

  t0 = now();
  for(int i = 0; i < nthread; i++) pthread_create(&tha[i], NULL, get_thread, (void *) (long) i);
  for(int i = 0; i < nthread; i++) pthread_join(tha[i], &value);
  t1 = now();
  printf("Gets done: %.3f seconds\n", t1 - t0);

  // Clean up
  for (int i = 0; i < NBUCKET; i++){
    pthread_rwlock_destroy(&rwlocks[i]);
  }
  return 0;       
}