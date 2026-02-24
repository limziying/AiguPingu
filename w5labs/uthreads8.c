#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 8: THREAD IDs (TID)
 * Assigning and retrieving unique handles.
 */

struct thread_context {
  uint64 ra; uint64 sp;
  uint64 s0; uint64 s1; uint64 s2; uint64 s3; uint64 s4; uint64 s5;
  uint64 s6; uint64 s7; uint64 s8; uint64 s9; uint64 s10; uint64 s11;
};

struct thread {
  char stack[STACK_SIZE];
  int state;
  int tid; // NEW: Unique ID
  struct thread_context context;
};

struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
int next_tid = 1;

extern void thread_switch(struct thread_context *old, struct thread_context *new);

void thread_init(void) { 
  current_thread = &all_thread[0]; 
  current_thread->state = RUNNING; 
  current_thread->tid = 0; // Main is 0
}

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

// NEW: Returns TID
int thread_create(void (*func)()) {
  struct thread *t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) { if (t->state == FREE) break; }
  t->state = RUNNABLE;
  t->tid = next_tid++; // Assign ID
  t->context.ra = (uint64) func; t->context.sp = (uint64) (t->stack + STACK_SIZE);
  return t->tid;
}

int get_tid() { return current_thread->tid; }

void thread_yield(void) { current_thread->state = RUNNABLE; thread_schedule(); }

void thread_worker(void) {
  printf("Hello from thread with TID: %d\n", get_tid());
  current_thread->state = FREE; thread_schedule();
}

int main() {
  thread_init();
  int tid1 = thread_create(thread_worker);
  int tid2 = thread_create(thread_worker);
  printf("Main thread created TIDs: %d and %d\n", tid1, tid2);
  
  current_thread->state = FREE; thread_schedule();
  exit(0);
}