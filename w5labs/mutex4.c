#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#define NBUCKET 5
#define NKEYS 100000

/*
 * VARIATION 4: DEADLOCK PREVENTION VIA LOCK ORDERING
 * * WHY THIS IS TESTED:
 * Multi-key operations (like transferring money between two bank accounts, 
 * or swapping two hash values) require grabbing TWO locks at once.
 * If Thread A grabs Lock 1 then Lock 2, while Thread B grabs Lock 2 then Lock 1,
 * they will DEADLOCK and freeze forever.
 * SOLUTION: Always acquire locks in a consistent global order (e.g., lower index first).
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
  pthread_mutex_lock(&locks[i]);
  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) { if (e->key == key) break; }
  if(e) e->value = value;
  else insert(key, value, &table[i], table[i]);
  pthread_mutex_unlock(&locks[i]);
}

// THE DEADLOCK-FREE SWAP FUNCTION
static void swap_values(int key1, int key2) {
  int b1 = key1 % NBUCKET;
  int b2 = key2 % NBUCKET;

  if (b1 == b2) {
    // Both keys are in the same bucket. Just lock it once!
    pthread_mutex_lock(&locks[b1]);
  } else if (b1 < b2) {
    // ALWAYS lock the lower index first
    pthread_mutex_lock(&locks[b1]);
    pthread_mutex_lock(&locks[b2]);
  } else { // b2 < b1
    // Even though b2 was passed second, we lock it FIRST because its ID is lower
    pthread_mutex_lock(&locks[b2]);
    pthread_mutex_lock(&locks[b1]);
  }

  // CRITICAL SECTION: Safe to manipulate both buckets
  // (In a real scenario, we'd search for the keys and swap their values here)
  
  // UNLOCK: Order of unlocking doesn't cause deadlocks, but reverse-order is standard
  if (b1 == b2) {
    pthread_mutex_unlock(&locks[b1]);
  } else if (b1 < b2) {
    pthread_mutex_unlock(&locks[b2]);
    pthread_mutex_unlock(&locks[b1]);
  } else {
    pthread_mutex_unlock(&locks[b1]);
    pthread_mutex_unlock(&locks[b2]);
  }
}

static void *swap_thread(void *xa) {
  // Randomly swap values to stress-test our locking logic
  for (int i = 0; i < NKEYS - 1; i++) {
    swap_values(keys[i], keys[i+1]);
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  pthread_t tha[2]; 
  
  for (int i = 0; i < NBUCKET; i++){ pthread_mutex_init(&locks[i], NULL); }
  srandom(0);
  for (int i = 0; i < NKEYS; i++) keys[i] = random();

  // Run two swap threads concurrently. If lock ordering is wrong, this will freeze!
  pthread_create(&tha[0], NULL, swap_thread, NULL);
  pthread_create(&tha[1], NULL, swap_thread, NULL);
  
  pthread_join(tha[0], NULL);
  pthread_join(tha[1], NULL);

  printf("Swap threads completed without deadlocking!\n");

  for (int i = 0; i < NBUCKET; i++){ pthread_mutex_destroy(&locks[i]); }
  return 0;       
}