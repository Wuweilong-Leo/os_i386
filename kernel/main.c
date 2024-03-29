#include "console.h"
#include "debug.h"
#include "init.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "process.h"
#include "syscall.h"
#include "thread.h"

void thread1(void *);
void thread2(void *);
void process1(void);
void process2(void);

int main() {
  put_str("I AM KERNEL\n");
  init_all();
  thread_run("kthread1", 8, thread1, "arg1_");
  thread_run("kthread2", 8, thread2, "arg2_");
  process_run(process1, "process1");
  process_run(process2, "process2");
  intr_enable();
  uint32_t pid = RUNNING_THREAD->pid;
  intr_disable();
  console_put_int(pid);
  intr_enable();
  while (1) {
    console_put_int(pid);
  }
}

void thread1(void *arg) {
  uint32_t pid = RUNNING_THREAD->pid;
  intr_disable();
  console_put_int(pid);
  intr_enable();
  while (1) {
    console_put_int(pid);
  }
}

void thread2(void *arg) {
  uint32_t pid = RUNNING_THREAD->pid;
  intr_disable();
  console_put_int(pid);
  intr_enable();
  while (1) {
    console_put_int(pid);
  }
}

void process1() {
  put_int(pid_get());
  while (1) {
  }
}

void process2() {
  put_int(pid_get());
  while (1) {
  }
}