/*
Kernel tweak: include the syscall number too
Twist tested: formatting & indexing correctness.
*/

if(p->monitor_mask & (1 << num)){
  printf("%d: syscall #%d %s -> %d\n",
         p->pid, num, syscall_names[num], ret);
}