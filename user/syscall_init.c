#include "print.h"
#include "syscall.h"
#include "thread.h"

#define SYSCALL_NUM 32
typedef void *syscall;
syscall syscall_table[SYSCALL_NUM];

uint32_t sys_pid_get() { return running_thread->pid; }

void syscall_init() {
  put_str("syscall_init start\n");
  syscall_table[PID_GET] = sys_pid_get;
  put_str("syscall_init done\n");
}