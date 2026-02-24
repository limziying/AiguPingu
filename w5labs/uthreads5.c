#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"AA

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define SLEEPING    0x4 // NEW: Sleeping state
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 5: THREAD SLEEP
 * Pausing threads for a specified number of "ticks"
 */

struct thread_context {
  uint64 ra; uint64 sp;
  uint64 s0; uint64 s1; uint64 s2; uint64 s3; uint64 s4; uint64 s5;
  uint64 s6; uint64 s7; uint64 s8; uint64 s9; uint64 s10; uint64 s11;
};

struct thread {
  char stack[STACK_SIZE];
  int state;
  int wakeup_tick; // NEW: Tick to wake up at
  struct thread_context context;
};

struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
int current_tick = 0; // NEW: Global clock

extern void thread_switch(struct thread_context *old, struct thread_context *new);

void thread_init(void) { current_thread = &all_thread[0]; current_thread->state = RUNNING; }

void thread_schedule(void) {
  current_tick++; // Advance clock every schedule call

  // WAKE UP PASS
  for(int i = 0; i < MAX_THREAD; i++){
    if(all_thread[i].state == SLEEPING && all_thread[i].wakeup_tick <= current_tick) {
      all_thread[i].state = RUNNABLE;
    }
  }

  struct thread *t, *next_thread = 0;
  t = current_thread + 1;
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

// NEW: Thread Sleep
void thread_sleep(int ticks) {
  current_thread->wakeup_tick = current_tick + ticks;
  current_thread->state = SLEEPING;
  thread_schedule();
}

void thread_a(void) {
  printf("A: Sleeping for 5 ticks.\n");
  thread_sleep(5);
  printf("A: Woke up!\n");
  current_thread->state = FREE; thread_schedule();
}

void thread_b(void) {
  for(int i=0; i<10; i++) {
    printf("B: Tick %d\n", current_tick);
    thread_yield();
  }
  current_thread->state = FREE; thread_schedule();
}

int main() {
  thread_init();
  thread_create(thread_a);
  thread_create(thread_b);
  current_thread->state = FREE; thread_schedule();
  exit(0);
}