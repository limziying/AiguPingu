#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define ZOMBIE      0x3 // NEW: Thread finished, waiting for join
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 3: EXIT AND JOIN
 * Threads become ZOMBIES upon exit and must be harvested.
 */

struct thread_context {
  uint64 ra; uint64 sp;
  uint64 s0; uint64 s1; uint64 s2; uint64 s3; uint64 s4; uint64 s5;
  uint64 s6; uint64 s7; uint64 s8; uint64 s9; uint64 s10; uint64 s11;
};

struct thread {
  char stack[STACK_SIZE];
  int state;
  int exit_status; // NEW: Store exit code
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
  t = current_thread + 1;
  for(int i = 0; i < MAX_THREAD; i++){
    if(t >= all_thread + MAX_THREAD) t = all_thread;
    if(t->state == RUNNABLE) { next_thread = t; break; }
    t = t + 1;
  }
  if (next_thread == 0) { exit(0); }
  
  if (current_thread != next_thread) {
    next_thread->state = RUNNING;
    t = current_thread;
    current_thread = next_thread;
    thread_switch(&t->context, &next_thread->context);
  }
}

struct thread* thread_create(void (*func)()) {
  struct thread *t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  t->state = RUNNABLE;
  t->context.ra = (uint64) func;
  t->context.sp = (uint64) (t->stack + STACK_SIZE);
  return t; // Return pointer for joining
}

void thread_yield(void) {
  current_thread->state = RUNNABLE; thread_schedule();
}

// NEW: Thread Exit
void thread_exit(int status) {
  current_thread->exit_status = status;
  current_thread->state = ZOMBIE;
  thread_schedule();
}

// NEW: Thread Join
void thread_join(struct thread *target) {
  while(target->state != ZOMBIE && target->state != FREE) {
    thread_yield(); // Wait for it to finish
  }
  if (target->state == ZOMBIE) {
    printf("Harvested thread with status %d\n", target->exit_status);
    target->state = FREE;
  }
}

void thread_a(void) {
  printf("Thread A doing work...\n");
  thread_yield();
  printf("Thread A exiting with status 42.\n");
  thread_exit(42);
}

int main() {
  thread_init();
  struct thread* ta = thread_create(thread_a);
  
  printf("Main thread waiting for Thread A to finish...\n");
  thread_join(ta);
  printf("Main thread finished.\n");
  
  exit(0);
}