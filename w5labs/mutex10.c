#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h> // Required for C11 Atomics

#define NBUCKET 5
#define NKEYS 100000

/*
 * VARIATION 10: LOCK-FREE COUNTERS VIA ATOMICS
 * * WHY THIS IS TESTED:
 * If the professor asks you to count the TOTAL number of inserts across all threads,
 * you might be tempted to use a global variable `int count` and a global mutex.
 * But we established in Variation 1 that global locks destroy parallel speedup!
 * SOLUTION: Use hardware-level atomic operations. The CPU guarantees these 
 * operations happen safely in one clock cycle without needing OS-level locks.
 */

pthread_mutex_t locks[NBUCKET];

// ATOMIC COUNTER: thread-safe without mutexes!
_Atomic int total_inserts = 0; 

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
  pthread_mutex_lock(&locks[i]);

  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) { if (e->key == key) break; }
  
  if(e) {
    e->value = value;
  } else {
    insert(key, value, &table[i], table[i]);
    
    // ATOMIC INCREMENT: 
    // Safely adds 1 to the counter without needing to lock a global mutex!
    atomic_fetch_add(&total_inserts, 1);
  }
  
  pthread_mutex_unlock(&locks[i]);
}

static void *put_thread(void *xa) {
  int n = (int) (long) xa; int b = NKEYS/nthread;
  for (int i = 0; i < b; i++) put(keys[b*n + i], n);
  return NULL;
}

int main(int argc, char *argv[]) {
  pthread_t *tha; 
  for (int i = 0; i < NBUCKET; i++){ pthread_mutex_init(&locks[i], NULL); }
  
  if (argc < 2) exit(-1);
  nthread = atoi(argv[1]); tha = malloc(sizeof(pthread_t) * nthread);

  srandom(0);
  for (int i = 0; i < NKEYS; i++) keys[i] = random();

  for(int i = 0; i < nthread; i++) pthread_create(&tha[i], NULL, put_thread, (void *) (long) i);
  for(int i = 0; i < nthread; i++) pthread_join(tha[i], NULL);
  
  // Print the atomic counter at the end
  printf("Total successful new inserts (tracked atomically): %d\n", total_inserts);

  for (int