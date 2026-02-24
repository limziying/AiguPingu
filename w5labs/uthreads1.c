#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 1: PASSING ARGUMENTS TO THREADS 
 * Demonstrates passing an integer argument into the a0 register.
 * Note: Requires corresponding assembly update in uthread_switch.S to load a0!
 */

struct thread_context {
  uint64 ra;
  uint64 sp;
  uint64 a0; // NEW: RISC-V passes first argument in a0
  uint64 s0; uint64 s1; uint64 s2; uint64 s3; uint64 s4; uint64 s5;
  uint64 s6; uint64 s7; uint64 s8; uint64 s9; uint64 s10; uint64 s11;
};

struct thread {
  char stack[STACK_SIZE];
  int state;
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

// NEW: Accepts an argument
void thread_create(void (*func)(int), int arg) {
  struct thread *t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  t->state = RUNNABLE;
  t->context.ra = (uint64) func;
  t->context.sp = (uint64) (t->stack + STACK_SIZE);
  t->context.a0 = (uint64) arg; // Store argument in context
}

void thread_yield(void) {
  current_thread->state = RUNNABLE;
  thread_schedule();
}

// We now only need ONE generic worker function!
void thread_worker(int my_id) {
  printf("Thread Worker started with ID: %d\n", my_id);
  for (int i = 0; i < 3; i++) {
    printf("Worker %d doing work phase %d\n", my_id, i);
    thread_yield();
  }
  printf("Worker %d exiting\n", my_id);
  current_thread->state = FREE;
  thread_schedule();
}

int main(int argc, char *argv[]) {
  thread_init();
  // Pass arguments 1, 2, and 3 to our threads
  thread_create(thread_worker, 1);
  thread_create(thread_worker, 2);
  thread_create(thread_worker, 3);
  
  current_thread->state = FREE;
  thread_schedule();
  exit(0);
}