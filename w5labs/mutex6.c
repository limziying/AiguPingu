#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sched.h> // Needed for sched_yield()
#include <sys/time.h>

#define NBUCKET 5
#define NKEYS 100000

/*
 * VARIATION 6: NON-BLOCKING LOCKS (trylock) WITH BACK-OFF
 * * WHY THIS IS TESTED:
 * `pthread_mutex_lock` is blocking. If you want to do OTHER work while waiting 
 * for a lock, you use `trylock`. If `trylock` fails (returns EBUSY), we manually 
 * yield the CPU back to the OS scheduler using `sched_yield()`. This is a classic 
 * OS synchronization pattern.
 */
pthread_mutex_t locks[NBUCKET];

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
  
  // NON-BLOCKING ATTEMPT
  // trylock returns 0 if successful, or an error code (like EBUSY) if it's already locked
  while (pthread_mutex_trylock(&locks[i]) != 0) {
    // The lock is busy! 
    // Instead of waiting, we politely tell the OS "I'm giving up my CPU timeslice, 
    // let someone else run for a bit."
    sched_yield(); 
  }

  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) { if (e->key == key) break; }
  if(e) e->value = value;
  else insert(key, value, &table[i], table[i]);
  
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
  
  printf("Trylock + yield completed successfully!\n");

  for (int i = 0; i < NBUCKET; i++){ pthread_mutex_destroy(&locks[i]); }
  return 0;       
}