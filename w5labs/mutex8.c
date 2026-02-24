#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#define NBUCKET 5
#define NKEYS 100

/*
 * VARIATION 8: CONDITION VARIABLES (Waiting for Data)
 * * WHY THIS IS TESTED:
 * Mutexes protect data, but Condition Variables coordinate TIMING.
 * If Thread A tries to `get(key)` but Thread B hasn't `put(key)` yet, 
 * Thread A can use a Condition Variable to go to sleep. Thread B will 
 * broadcast a "Wake Up" signal when it inserts the key!
 */
pthread_mutex_t locks[NBUCKET];
pthread_cond_t cvs[NBUCKET]; // Array of condition variables

struct entry { int key; int value; struct entry *next; };
struct entry *table[NBUCKET];
int keys[NKEYS];

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
  
  // WAKE UP SIGNAL: Tell any waiting threads "Hey, I just updated bucket 'i'!"
  pthread_cond_broadcast(&cvs[i]);
  
  pthread_mutex_unlock(&locks[i]);
}

static struct entry* get_wait(int key) {
  int i = key % NBUCKET;
  pthread_mutex_lock(&locks[i]);
  
  struct entry *e = 0;
  while (1) { // Loop is necessary to handle spurious wakeups
    for (e = table[i]; e != 0; e = e->next) {
      if (e->key == key) break;
    }
    if (e != NULL) break; // Found it!
    
    // NOT FOUND YET. Go to sleep!
    // pthread_cond_wait automatically unlocks `locks[i]`, puts thread to sleep,
    // and automatically RE-LOCKS it when awoken by a broadcast!
    pthread_cond_wait(&cvs[i], &locks[i]);
  }
  
  pthread_mutex_unlock(&locks[i]);
  return e;
}

static void *get_thread(void *xa) {
  // This thread will sleep until the put_thread does its job!
  get_wait(keys[0]); 
  printf("Get thread successfully awoke and found the key!\n");
  return NULL;
}

static void *put_thread(void *xa) {
  sleep(1); // Artificially delay to force get_thread to wait
  put(keys[0], 99);
  return NULL;
}

int main() {
  pthread_t t1, t2;
  for (int i = 0; i < NBUCKET; i++){ 
    pthread_mutex_init(&locks[i], NULL); 
    pthread_cond_init(&cvs[i], NULL);
  }
  keys[0] = 42;

  pthread_create(&t1, NULL, get_thread, NULL);
  pthread_create(&t2, NULL, put_thread, NULL);
  
  pthread_join(t1, NULL); pthread_join(t2, NULL);
  return 0;       
}