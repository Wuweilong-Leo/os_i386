#include "thread.h"
#include "debug.h"
#include "global.h"
#include "memory.h"
#include "stdint.h"
#include "string.h"

#define PG_SIZE 4096

static void kthread_func_run(thread_func func, void *func_arg) {
  (*func)(func_arg);
}

static void thread_init(tcb *pthread, char *name, uint8_t prio,
                        thread_func func, void *func_arg) {
  memset(pthread, 0, sizeof(*pthread));
  strcpy(pthread->name, name);
  pthread->status = TASK_RUNNING;
  pthread->priority = prio;
  pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);
  pthread->stack_magic = 0x19980820;
  // 把栈指针指向thread_stack
  uint32_t kstack_tmp = (uint32_t)(pthread->self_kstack);
  kstack_tmp -= sizeof(struct intr_stack);
  kstack_tmp -= sizeof(struct thread_stack);
  pthread->self_kstack = (uint32_t *)kstack_tmp;
  struct thread_stack *kthread_stack =
      (struct thread_stack *)pthread->self_kstack;
  kthread_stack->eip = kthread_func_run;
  kthread_stack->func = func;
  kthread_stack->func_arg = func_arg;
  kthread_stack->ebp = 0;
  kthread_stack->ebx = 0;
  kthread_stack->esi = 0;
  kthread_stack->edi = 0;
}

static tcb *thread_create(char *thread_name, uint8_t thread_prio,
                          thread_func func, void *func_arg) {
  // 申请一页内存当做tcb空间和栈空间，tcb处于栈顶，即此内存空间最低处。
  tcb *thread = kernel_malloc(1);
  ASSERT(thread != NULL);
  // 初始化线程
  thread_init(thread, thread_name, thread_prio, func, func_arg);
  return thread;
}

tcb *thread_run(char *name, uint8_t prio, thread_func func, void *func_arg) {
  tcb *thread = thread_create(name, prio, func, func_arg);
  // ret 会把栈顶作为eip, 即执行kthread_func_run
  asm volatile(
      "movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret" ::"g"(
          thread->self_kstack)
      : "memory");
  return thread;
}