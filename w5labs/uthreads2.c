#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 2: PRIORITY-BASED SCHEDULING 
 * Lower priority numbers execute first.
 */

struct thread_context {
  uint64 ra; uint64 sp;
  uint64 s0; uint64 s1; uint64 s2; uint64 s3; uint64 s4; uint64 s5;
  uint64 s6; uint64 s7; uint64 s8; uint64 s9; uint64 s10; uint64 s11;
};

struct thread {
  char stack[STACK_SIZE];
  int state;
  int priority; // NEW: Priority level
  struct thread_context context;
};

struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
extern void thread_switch(struct thread_context *old, struct thread_context *new);

void thread_init(void) {
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
}

// NEW: Priority Scheduler Logic
void thread_schedule(void) {
  struct thread *t, *next_thread = 0;
  int best_priority = 9999; 

  // Look for the absolute lowest priority number
  for(int i = 0; i < MAX_THREAD; i++){
    t = &all_thread[i];
    if(t->state == RUNNABLE && t->priority < best_priority) {
      best_priority = t->priority;
      next_thread = t;
    }
  }

  if (next_thread == 0) {
    printf("thread_schedule: no runnable threads\n");
    exit(0);
  }

  if (current_thread != next_thread) {
    next_thread->state = RUNNING;
    t = current_thread;
    current_thread = next_thread;
    thread_switch(&t->context, &next_thread->context);
  }
}

// NEW: Pass priority when creating
void thread_create(void (*func)(), int priority) {
  struct thread *t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  t->state = RUNNABLE;
  t->priority = priority;
  t->context.ra = (uint64) func;
  t->context.sp = (uint64) (t->stack + STACK_SIZE);
}

void thread_yield(void) {
  current_thread->state = RUNNABLE;
  thread_schedule();
}

void high_priority_thread(void) {
  printf("High Priority (Prio 1) thread running!\n");
  current_thread->state = FREE; thread_schedule();
}

void low_priority_thread(void) {
  printf("Low Priority (Prio 10) thread running!\n");
  current_thread->state = FREE; thread_schedule();
}

int main() {
  thread_init();
  
  // Create low priority first, but high priority will be scheduled first!
  thread_create(low_priority_thread, 10);
  thread_create(high_priority_thread, 1);
  
  current_thread->state = FREE;
  thread_schedule();
  exit(0);
}