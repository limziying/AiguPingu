#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2
#define STACK_SIZE  8192
#define MAX_THREAD  4

/* VARIATION 6: LOTTERY SCHEDULER
 * Threads get tickets; scheduler draws a random ticket.
 */

struct thread_context {
  uint64 ra; uint64 sp;
  uint64 s0; uint64 s1; uint64 s2; uint64 s3; uint64 s4; uint64 s5;
  uint64 s6; uint64 s7; uint64 s8; uint64 s9; uint64 s10; uint64 s11;
};

struct thread {
  char stack[STACK_SIZE];
  int state;
  int tickets; // NEW: Share of CPU time
  struct thread_context context;
};

struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
extern void thread_switch(struct thread_context *old, struct thread_context *new);

// Pseudo-random generator for xv6 user space
unsigned int seed = 1234;
int pseudo_rand() { seed = (1103515245 * seed + 12345); return (unsigned int)(seed / 65536) % 32768; }

void thread_init(void) { current_thread = &all_thread[0]; current_thread->state = RUNNING; }

void thread_schedule(void) {
  struct thread *t, *next_thread = 0;
  
  // 1. Count tickets
  int total_tickets = 0;
  for(int i = 0; i < MAX_THREAD; i++){
    if(all_thread[i].state == RUNNABLE) total_tickets += all_thread[i].tickets;
  }
  if (total_tickets == 0) exit(0);

  // 2. Draw ticket
  int winning_ticket = pseudo_rand() % total_tickets;
  
  // 3. Find winner
  int current_count = 0;
  for(int i = 0; i < MAX_THREAD; i++){
    t = &all_thread[i];
    if(t->state == RUNNABLE) {
      current_count += t->tickets;
      if (current_count > winning_ticket) { next_thread = t; break; }
    }
  }

  if (current_thread != next_thread) {
    next_thread->state = RUNNING; t = current_thread; current_thread = next_thread;
    thread_switch(&t->context, &next_thread->context);
  }
}

void thread_create(void (*func)(), int tickets) {
  struct thread *t;
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) { if (t->state == FREE) break; }
  t->state = RUNNABLE; t->tickets = tickets; 
  t->context.ra = (uint64) func; t->context.sp = (uint64) (t->stack + STACK_SIZE);
}

void thread_yield(void) { current_thread->state = RUNNABLE; thread_schedule(); }

void rich_thread(void) {
  printf("I have 100 tickets! I will run often.\n");
  current_thread->state = FREE; thread_schedule();
}

void poor_thread(void) {
  printf("I have 1 ticket. I rarely run.\n");
  current_thread->state = FREE; thread_schedule();
}

int main() {
  thread_init();
  thread_create(poor_thread, 1);
  thread_create(rich_thread, 100);
  current_thread->state = FREE; thread_schedule();
  exit(0);
}