/*
Kernel tweak: stop tracing after N syscalls (per-process counter)

Twist tested: process state + persistence across exec/fork.
*/

/*Add to struct proc (kernel/proc.h):*/

uint32 monitor_mask;
int monitor_left;   // how many syscall prints remain (-1 for infinite)

/*In sys_monitor():*/

// monitor(mask, limit) variant could be used; or hardcode for lab test.
p->monitor_mask = mask;
p->monitor_left = 100; // e.g., print next 100 monitored syscalls then stop

/*In syscall():*/

if(p->monitor_mask & (1 << num)){
  if(p->monitor_left != 0){
    printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], ret);
    if(p->monitor_left > 0) p->monitor_left--;
  }
}

/*And in fork() copy it:*/

np->monitor_mask = p->monitor_mask;
np->monitor_left = p->monitor_left;