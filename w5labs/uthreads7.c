#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define SUSPENDED   0x5 // NEW
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 7: SUSPEND AND RESUME APIS */

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
struct thread* thread_create(void (*func)()) {
  struct thread *t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) { if (t->state == FREE) break; }
  t->state = RUNNABLE; t->context.ra = (uint64) func; t->context.sp = (uint64) (t->stack + STACK_SIZE);
  return t;
}
void thread_yield(void) { current_thread->state = RUNNABLE; thread_schedule(); }

// NEW APIS
void thread_suspend(struct thread *target) {
  if (target == current_thread) {
    target->state = SUSPENDED; thread_schedule();
  } else if (target->state == RUNNABLE) {
    target->state = SUSPENDED;
  }
}

void thread_resume(struct thread *target) {
  if (target->state == SUSPENDED) target->state = RUNNABLE;
}

void victim_thread(void) {
  printf("Victim: I'm running...\n");
  thread_yield();
  printf("Victim: I survived suspension!\n");
  current_thread->state = FREE; thread_schedule();
}

int main() {
  thread_init();
  struct thread* tv = thread_create(victim_thread);
  
  printf("Main: Suspending victim.\n");
  thread_suspend(tv);
  
  thread_yield(); // Give scheduler chance to run victim (it shouldn't run)
  
  printf("Main: Resuming victim.\n");
  thread_resume(tv);
  
  current_thread->state = FREE; thread_schedule();
  exit(0);
}