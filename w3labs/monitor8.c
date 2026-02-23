/*
Kernel tweak: print only on SUCCESS (return >= 0)
Twist tested: conditional logging.
*/

// kernel/syscall.c inside syscall()
// After executing syscalls[num], you have return value in "ret".
if(p->monitor_mask & (1 << num)){
  if(ret >= 0){
    printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], ret);
  }
}