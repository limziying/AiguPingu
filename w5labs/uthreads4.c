#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 4: USER-LEVEL MUTEX VIA YIELD
 * Implementing a lock entirely in user space.
 */

struct thread_context {
  uint64 ra; uint64 sp;
  uint64 s0; uint64 s1; uint64 s2; uint64 s3; uint64 s4; uint64 s5;
  uint64 s6; uint64 s7; uint64 s8; uint64 s9; uint64 s10; uint64 s11;
};

struct thread {
  char stack[STACK_SIZE]; int state;
  struct thread_context context;
};

struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
extern void thread_switch(struct thread_context *old, struct thread_context *new);

void thread_init(void) { current_thread = &all_thread[0]; current_thread->state = RUNNING; }
void thread_schedule(void) {
  struct thread *t, *next_thread = 0; t = current_thread + 1;
  for(int i = 0; i < MAX_THREAD; i++){
    if(t >= all_thread + MAX_THREAD) t = all_thread;
    if(t->state == RUNNABLE) { next_thread = t; break; }
    t = t + 1;
  }
  if (next_thread == 0) { exit(0); }
  if (current_thread != next_thread) {
    next_thread->state = RUNNING; t = current_thread; current_thread = next_thread;
    thread_switch(&t->context, &next_thread->context);
  }
}
void thread_create(void (*func)()) {
  struct thread *t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) { if (t->state == FREE) break; }
  t->state = RUNNABLE; t->context.ra = (uint64) func; t->context.sp = (uint64) (t->stack + STACK_SIZE);
}
void thread_yield(void) { current_thread->state = RUNNABLE; thread_schedule(); }

// --- CUSTOM MUTEX IMPLEMENTATION ---
struct uthread_mutex { int locked; };
void mutex_init(struct uthread_mutex *m) { m->locked = 0; }
void mutex_lock(struct uthread_mutex *m) {
  while(__sync_lock_test_and_set(&m->locked, 1) != 0) {
    thread_yield(); // Yield if we can't get the lock
  }
}
void mutex_unlock(struct uthread_mutex *m) { __sync_lock_release(&m->locked); }

// --- TEST WORKLOAD ---
struct uthread_mutex lock;
int shared_counter = 0;

void thread_worker(void) {
  for (int i = 0; i < 1000; i++) {
    mutex_lock(&lock);
    int temp = shared_counter;
    thread_yield(); // Force race conditions if lock doesn't work!
    shared_counter = temp + 1;
    mutex_unlock(&lock);
  }
  printf("Worker finished.\n");
  current_thread->state = FREE; thread_schedule();
}

int main() {
  thread_init();
  mutex_init(&lock);
  thread_create(thread_worker);
  thread_create(thread_worker);
  
  // Yield enough times for workers to finish
  for(int i=0; i<3000; i++) thread_yield();
  printf("Final counter (should be 2000): %d\n", shared_counter);
  
  exit(0);
}