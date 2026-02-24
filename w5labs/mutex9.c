#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define NBUCKET 5
#define NKEYS 100000

/*
 * VARIATION 9: PREVENTING "FALSE SHARING" (CACHE PADDING)
 * * WHY THIS IS TESTED:
 * A standard array of mutexes sits consecutively in RAM. A CPU cache line is 
 * typically 64 bytes. This means `locks[0]` and `locks[1]` are in the SAME cache line.
 * If Core 1 locks bucket 0, and Core 2 locks bucket 1, the CPU hardware will 
 * constantly invalidate and bounce the cache line between cores, destroying performance.
 * SOLUTION: Pad each lock with empty bytes so they sit on separate cache lines.
 */

struct padded_lock {
  pthread_mutex_t lock;
  // Pad the struct to be exactly 64 bytes (the size of a standard CPU cache line)
  char padding[64 - sizeof(pthread_mutex_t)]; 
};

// Use the padded lock array!
struct padded_lock padded_locks[NBUCKET];

struct entry { int key; int value; struct entry *next; };
struct entry *table[NBUCKET];
int keys[NKEYS];
int nthread = 1;

static void insert(int key, int value, struct entry **p, struct entry *n) {
  struct entry *e = malloc(sizeof(struct entry));
  e->key = key; e->value = value; e->next = n; *p = e;
}

static void put(int key, int value) {
  int i = key % NBUCKET;
  
  // Access the actual lock inside the padded structure
  pthread_mutex_lock(&padded_locks[i].lock);

  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) { if (e->key == key) break; }
  if(e) e->value = value;
  else insert(key, value, &table[i], table[i]);
  
  pthread_mutex_unlock(&padded_locks[i].lock);
}

static void *put_thread(void *xa) {
  int n = (int) (long) xa; int b = NKEYS/nthread;
  for (int i = 0; i < b; i++) put(keys[b*n + i], n);
  return NULL;
}

int main(int argc, char *argv[]) {
  pthread_t *tha; 
  // Verify that our struct is 64 bytes big
  printf("Size of padded lock struct: %lu bytes\n", sizeof(struct padded_lock));

  for (int i = 0; i < NBUCKET; i++){ 
    pthread_mutex_init(&padded_locks[i].lock, NULL); 
  }
  
  if (argc < 2) exit(-1);
  nthread = atoi(argv[1]); tha = malloc(sizeof(pthread_t) * nthread);

  srandom(0);
  for (int i = 0; i < NKEYS; i++) keys[i] = random();

  for(int i = 0; i < nthread; i++) pthread_create(&tha[i], NULL, put_thread, (void *) (long) i);
  for(int i = 0; i < nthread; i++) pthread_join(tha[i], NULL);
  
  for (int i = 0; i < NBUCKET; i++){ pthread_mutex_destroy(&padded_locks[i].lock); }
  return 0;       
}