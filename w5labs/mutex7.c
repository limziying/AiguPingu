#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define NBUCKET 5
#define NKEYS 100000

/*
 * VARIATION 7: HAND-OVER-HAND LOCKING (Crab Walking)
 * * WHY THIS IS TESTED:
 * This is an advanced data-structures concept. Instead of locking the whole bucket, 
 * we place a lock inside EVERY NODE. We traverse the list by holding the current node, 
 * grabbing the next node, and THEN releasing the current node.
 * This allows multiple threads to traverse the EXACT SAME bucket simultaneously 
 * as long as they are on different nodes!
 */

struct entry {
  int key;
  int value;
  struct entry *next;
  pthread_mutex_t node_lock; // NEW: Lock per node
};

// We still need a lock just to access the head pointers of the buckets safely
pthread_mutex_t bucket_locks[NBUCKET]; 
struct entry *table[NBUCKET];

static void put(int key, int value) {
  int i = key % NBUCKET;
  
  pthread_mutex_lock(&bucket_locks[i]);
  struct entry *curr = table[i];
  
  // Case 1: Bucket is completely empty
  if (curr == NULL) {
    struct entry *e = malloc(sizeof(struct entry));
    e->key = key; e->value = value; e->next = NULL;
    pthread_mutex_init(&e->node_lock, NULL);
    table[i] = e;
    pthread_mutex_unlock(&bucket_locks[i]);
    return;
  }

  // Case 2: Bucket has nodes. Lock the first node, then release the bucket lock.
  pthread_mutex_lock(&curr->node_lock);
  pthread_mutex_unlock(&bucket_locks[i]);

  // Hand-over-hand traversal
  while (1) {
    if (curr->key == key) {
      curr->value = value; // Update existing
      pthread_mutex_unlock(&curr->node_lock);
      return;
    }
    
    if (curr->next == NULL) {
      // Reached the end. Append new node to the tail.
      struct entry *e = malloc(sizeof(struct entry));
      e->key = key; e->value = value; e->next = NULL;
      pthread_mutex_init(&e->node_lock, NULL);
      
      curr->next = e;
      pthread_mutex_unlock(&curr->node_lock);
      return;
    }
    
    // CRAB WALKING: 
    // 1. Get pointer to next node
    struct entry *next_node = curr->next;
    // 2. Lock the next node
    pthread_mutex_lock(&next_node->node_lock);
    // 3. Unlock the current node
    pthread_mutex_unlock(&curr->node_lock);
    // 4. Move forward
    curr = next_node;
  }
}

// Dummy main to compile
int main(int argc, char *argv[]) {
  for (int i = 0; i < NBUCKET; i++){ pthread_mutex_init(&bucket_locks[i], NULL); }
  put(10, 1); put(15, 2); put(10, 3);
  printf("Hand-over-hand locking ran successfully.\n");
  return 0;       
}