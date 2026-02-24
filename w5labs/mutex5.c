#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#define NBUCKET 5
#define NKEYS 100000

/*
 * VARIATION 5: SPINLOCKS
 * * WHY THIS IS TESTED:
 * Mutexes put a waiting thread to sleep (context switch). Context switches take CPU cycles.
 * Hash table insertions are FAST. By the time a thread goes to sleep, the lock 
 * might already be free! 
 * Spinlocks keep the thread awake in a "while loop" continuously checking the lock. 
 * On multi-core systems, this can be much faster for short critical sections.
 * Note: Spinlocks are a standard part of pthreads, but might not be available on some MacOS versions.
 */
pthread_spinlock_t spinlocks[NBUCKET];

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
  
  // SPIN LOCK: Burns CPU cycles "spinning" until the lock is acquired.
  pthread_spin_lock(&spinlocks[i]);

  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) { if (e->key == key) break; }
  if(e) e->value = value;
  else insert(key, value, &table[i], table[i]);
  
  // SPIN UNLOCK
  pthread_spin_unlock(&spinlocks[i]);
}

static struct entry* get(int key) {
  int i = key % NBUCKET;
  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) { if (e->key == key) break; }
  return e;
}

static void *put_thread(void *xa) {
  int n = (int) (long) xa; int b = NKEYS/nthread;
  for (int i = 0; i < b; i++) put(keys[b*n + i], n);
  return NULL;
}

int main(int argc, char *argv[]) {
  pthread_t *tha; void *value; double t1, t0;

  for (int i = 0; i < NBUCKET; i++){
    // Initialize spinlock. PTHREAD_PROCESS_PRIVATE restricts it to this program.
    pthread_spin_init(&spinlocks[i], PTHREAD_PROCESS_PRIVATE);
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

  for (int i = 0; i < NBUCKET; i++){
    pthread_spin_destroy(&spinlocks[i]);
  }
  return 0;       
}