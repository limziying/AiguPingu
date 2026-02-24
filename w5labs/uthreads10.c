#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 10: MULTI-LEVEL FEEDBACK QUEUE (MLFQ)
 * Threads that yield/hog CPU drop to lower priority queues.
 */

struct thread_context {
  uint64 ra; uint64 sp;
  uint64 s0; uint64 s1; uint64 s2; uint64 s3; uint64 s4; uint64 s5;
  uint64 s6; uint64 s7; uint64 s8; uint64 s9; uint64 s10; uint64 s11;
};

struct thread {
  char stack[STACK_SIZE];
  int state;
  int queue_level; // NEW: 0 = High, 1 = Low
  struct thread_context context;
};

struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
extern void thread_switch(struct thread_context *old, struct thread_context *new);

void thread_init(void) { 
  current_thread = &all_thread[0]; 
  current_thread->state = RUNNING; 
}

void thread_schedule(void) {
  struct thread *t, *next_thread = 0;
  
  // 1. Try Level 0 (High Priority)
  for(int i = 0; i < MAX_THREAD; i++){
    if(all_thread[i].state == RUNNABLE && all_thread[i].queue_level == 0) {
      next_thread = &all_thread[i]; break;
    }
  }

  // 2. Fallback to Level 1 (Low Priority)
  if (next_thread == 0) {
    for(int i = 0; i < MAX_THREAD; i++){
      if(all_thread[i].state == RUNNABLE && all_thread[i].queue_level == 1) {
        next_thread = &all_thread[i]; break;
      }
    }
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
  t->state = RUNNABLE;
  t->queue_level = 0; // Everyone starts at High priority
  t->context.ra = (uint64) func; t->context.sp = (uint64) (t->stack + STACK_SIZE);
}

void thread_yield(void) { 
  // PENALTY: Dropping to level 1 for yielding
  if (current_thread->queue_level == 0) {
    printf("Thread dropping to priority Level 1!\n");
    current_thread->queue_level = 1; 
  }
  current_thread->state = RUNNABLE; thread_schedule(); 
}

void cpu_hog_thread(void) {
  printf("Hog: Start at Level %d\n", current_thread->queue_level);
  thread_yield(); // Forces drop to Level 1
  printf("Hog: Now running at Level %d\n", current_thread->queue_level);
  current_thread->state = FREE; thread_schedule();
}

int main() {
  thread_init();
  thread_create(cpu_hog_thread);
  current_thread->state = FREE; thread_schedule();
  exit(0);
}