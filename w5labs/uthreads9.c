#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 9: DIRECTED YIELDING (`yield_to`)
 * Bypassing the scheduler for fast cooperations.
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
struct thread *ta, *tb; // Pointers for ping pong

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

struct thread* thread_create(void (*func)()) {
  struct thread *t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) { if (t->state == FREE) break; }
  t->state = RUNNABLE; t->context.ra = (uint64) func; t->context.sp = (uint64) (t->stack + STACK_SIZE);
  return t;
}

// NEW: Yield specifically to another thread
void thread_yield_to(struct thread *target) {
  if (target->state != RUNNABLE) return;
  
  current_thread->state = RUNNABLE;
  target->state = RUNNING;
  
  struct thread *t = current_thread;
  current_thread = target;
  
  // BYPASS SCHEDULER COMPLETELY!
  thread_switch(&t->context, &target->context);
}

void ping(void) {
  for(int i=0; i<3; i++) {
    printf("PING\n");
    thread_yield_to(tb); // Give control directly to tb
  }
  current_thread->state = FREE; thread_schedule();
}

void pong(void) {
  for(int i=0; i<3; i++) {
    printf("PONG\n");
    thread_yield_to(ta); // Give control directly to ta
  }
  current_thread->state = FREE; thread_schedule();
}

int main() {
  thread_init();
  ta = thread_create(ping);
  tb = thread_create(pong);
  
  current_thread->state = FREE; thread_schedule();
  exit(0);
}