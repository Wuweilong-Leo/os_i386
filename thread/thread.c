#include "thread.h"
#include "debug.h"
#include "global.h"
#include "list.h"
#include "memory.h"
#include "stdint.h"
#include "string.h"

#ifndef PG_SIEE
#define PG_SIZE 4096
#endif

tcb *main_thread;
tcb *running_thread;
struct list thread_ready_list;
struct list thread_all_list;

extern void switch_to(tcb *cur, tcb *next);

static void kthread_func_run(thread_func func, void *func_arg) {
  intr_enable(); // 每个线程都要保证初始状态中断开启

  (*func)(func_arg);
}

static void thread_tcb_init(tcb *pthread, char *name, uint8_t prio) {
  memset(pthread, 0, sizeof(*pthread));
  strcpy(pthread->name, name);
  pthread->status = (pthread == main_thread) ? TASK_RUNNING : TASK_READY;
  pthread->priority = prio;
  pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);
  pthread->ticks = prio; // 优先级和可执行的ticks相关
  pthread->pg_dir = NULL;
  pthread->stack_magic = 0x19980820;
}

static void thread_stack_init(tcb *pthread, thread_func func, void *func_arg) {
  uint32_t kstack_tmp = (uint32_t)(pthread->self_kstack);
  // kstack_tmp -= sizeof(struct intr_stack);
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
  /*
    创建完线程后，栈顶指针指向 栈底 - intr_stack - thread_stack
  */
}

static void thread_init(tcb *pthread, char *name, uint8_t prio,
                        thread_func func, void *func_arg) {
  thread_tcb_init(pthread, name, prio);
  thread_stack_init(pthread, func, func_arg);
}

static tcb *thread_create(char *thread_name, uint8_t thread_prio,
                          thread_func func, void *func_arg) {
  // 申请一页内存当做tcb空间和栈空间，tcb处于栈顶，即此内存空间最低处。
  tcb *thread = (tcb *)kernel_pages_malloc(1);
  // 初始化线程
  thread_init(thread, thread_name, thread_prio, func, func_arg);
  return thread;
}

tcb *thread_run(char *name, uint8_t prio, thread_func func, void *func_arg) {
  tcb *thread = thread_create(name, prio, func, func_arg);
  // 线程加入就绪队列
  ASSERT(!list_elem_find(&thread_ready_list, &thread->general_tag));
  list_push_back(&thread_ready_list, &thread->general_tag);
  ASSERT(list_elem_find(&thread_ready_list, &thread->general_tag));
  //  加入全局队列
  list_push_back(&thread_all_list, &thread->all_list_tag);
  return thread;
}

tcb *running_thread_get() {
  uint32_t esp;
  asm volatile("mov %%esp, %0" : "=g"(esp));
  return (tcb *)(esp & 0xfffff000);
}

static void main_thread_make() {
  running_thread = running_thread_get();
  main_thread = running_thread;
  thread_tcb_init(main_thread, "main", 31);
  ASSERT(!list_elem_find(&thread_all_list, &main_thread->all_list_tag));
  list_push_back(&thread_all_list, &main_thread->all_list_tag);
}

void schedule() {
  // 保证切换时中断关闭
  ASSERT(intr_get_status() == INTR_OFF);
  list_push_back(&thread_ready_list, &running_thread->general_tag);
  running_thread->ticks = running_thread->priority;
  running_thread->status = TASK_READY;

  // 必须保证ready队列有等待的线程
  ASSERT(!list_empty(&thread_ready_list));
  struct list_elem *thread_tag = list_pop_front(&thread_ready_list);
  tcb *next = ELEM2ENTRY(tcb, general_tag, thread_tag);
  next->status = TASK_RUNNING;
  tcb *cur = running_thread;
  running_thread = next;

  switch_to(cur, next);
}

void thread_all_init() {
  put_str("thread_all_init begin\n");
  list_init(&thread_ready_list);
  list_init(&thread_all_list);
  main_thread_make();
  put_str("thread_all_init done\n");
}