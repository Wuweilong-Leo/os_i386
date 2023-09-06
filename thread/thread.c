#include "thread.h"
#include "debug.h"
#include "global.h"
#include "list.h"
#include "memory.h"
#include "print.h"
#include "stdint.h"
#include "string.h"
#include "sync.h"
/*
 * 调度时机:
 *        1. 时间片中断
 *        2. 获取不到锁
 *
 */

/* 优先级反馈调度 */
struct scheduler scheduler;

struct scheduler *cur_scheduler = &scheduler;

void scheduler_init() {
  cur_scheduler->running_thread = NULL;
  cur_scheduler->main_thread = NULL;
  memset(cur_scheduler->thread_table, 0, sizeof(tcb *) * MAX_THREAD_NUM);
  uint8_t *btmp_base = (uint8_t *)kernel_pages_malloc(1);
  bitmap_init(&cur_scheduler->rq_mask, btmp_base, PRIO_NUM);
  for (uint32_t i = 0; i < PRIO_NUM; i++) {
    list_init(&cur_scheduler->rq[i]);
  }
  list_init(&cur_scheduler->all_list);
  lock_init(&cur_scheduler->pid_lock);
}

/* 排队 */
void scheduler_rq_join(tcb *thread) {
  uint32_t prio = thread->priority;
  list_push_back(&cur_scheduler->rq[prio], &thread->general_tag);
  bitmap_set(&cur_scheduler->rq_mask, prio);
}

/* 插队 */
void scheduler_rq_jump(tcb *thread) {
  uint32_t prio = thread->priority;
  list_push_front(&cur_scheduler->rq[prio], &thread->general_tag);
  bitmap_set(&cur_scheduler->rq_mask, prio);
}

static void kernel_thread_entry(thread_func func, void *func_arg) {
  intr_enable(); // 每个线程都要保证初始状态中断开启

  (*func)(func_arg);
}

// 先不考虑pid不够的情况
static void pid_alloc(tcb *thread) {
  lock_acquire(&cur_scheduler->pid_lock);
  for (uint32_t i = 0; i < MAX_THREAD_NUM; i++) {
    if (cur_scheduler->thread_table[i] == NULL) {
      thread->pid = i;
      cur_scheduler->thread_table[i] = thread;
      break;
    }
  }
  lock_release(&cur_scheduler->pid_lock);
}

void thread_tcb_init(tcb *pthread, char *name, uint8_t prio) {
  memset(pthread, 0, sizeof(*pthread));
  strcpy(pthread->name, name);
  pthread->status =
      (pthread == cur_scheduler->main_thread) ? TASK_RUNNING : TASK_READY;
  pthread->priority = prio;
  pthread->self_kstack = (uint32_t *)((uint32_t)pthread + PG_SIZE);
  pthread->ticks = prio; // 优先级和可执行的ticks相关
  pthread->pg_dir = NULL;
  pid_alloc(pthread);
  pthread->stack_magic = 0x19980820;
}

void thread_stack_init(tcb *pthread, thread_func func, void *func_arg) {
  uint32_t kstack_tmp = (uint32_t)(pthread->self_kstack);
  /* 此处是预留出位置给用户进程 */
  kstack_tmp -= sizeof(struct intr_stack);
  kstack_tmp -= sizeof(struct thread_stack);
  pthread->self_kstack = (uint32_t *)kstack_tmp;
  struct thread_stack *kthread_stack =
      (struct thread_stack *)pthread->self_kstack;
  kthread_stack->eip = kernel_thread_entry;
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

void thread_init(tcb *pthread, char *name, uint8_t prio, thread_func func,
                 void *func_arg) {
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
  scheduler_rq_join(thread);
  //  加入全局队列
  list_push_back(&cur_scheduler->all_list, &thread->all_list_tag);
  return thread;
}

tcb *running_thread_get() {
  uint32_t esp;
  asm volatile("mov %%esp, %0" : "=g"(esp));
  return (tcb *)(esp & 0xfffff000);
}

static void main_thread_make() {
  cur_scheduler->running_thread = running_thread_get();
  cur_scheduler->main_thread = cur_scheduler->running_thread;
  thread_tcb_init(cur_scheduler->main_thread, "main", 31);
  list_push_back(&cur_scheduler->all_list,
                 &cur_scheduler->running_thread->all_list_tag);
}

void thread_all_init() {
  put_str("thread_all_init begin\n");
  scheduler_init();
  main_thread_make();
  put_str("thread_all_init done\n");
}

// 阻塞现在的线程。
void thread_block(enum task_status tsk_status) {
  ASSERT(tsk_status == TASK_BLOCKED || tsk_status == TASK_WAITING ||
         tsk_status == TASK_HANGING);
  enum intr_status intr_save = intr_disable();
  cur_scheduler->running_thread->status = tsk_status;
  // 执行完此函数之后，就切换到其他线程，只有把此线程重新加入就绪队列，才再有机会调度。
  /*
   *  schedule 是从ready 队列取线程运行。
   *  ready 队列中：
   *         1. 从未执行的线程        ------  kthread_func_run 开启中断
   *         2. 时间片结束等待调度的线程  ------ 中断返回时会自己恢复中断状态
   *         3. 刚获得资源的线程  ------
   * 因为资源而阻塞的线程，一定是卡在此函数中，此函数最后会恢复中断，
   *
   */
  schedule();
  intr_set_status(intr_save);
}

// 唤醒线程
void thread_unblock(tcb *tsk) {
  ASSERT(tsk->status == TASK_BLOCKED || tsk->status == TASK_WAITING ||
         tsk->status == TASK_HANGING);
  enum intr_status intr_save = intr_disable();
  tsk->status = TASK_READY;
  scheduler_rq_jump(tsk);
  intr_set_status(intr_save);
}

extern void task_trap(tcb *);
extern void context_load(tcb *);

void schedule() {
  task_trap(cur_scheduler->running_thread);
  return;
}

void task_prio_down(tcb *task) {
  task->priority = (task->priority + 1) % PRIO_NUM;
}

uint32_t highest_rq_get() {
  uint32_t prio = 0;
  struct bitmap *rq_mask = &cur_scheduler->rq_mask;
  while (bitmap_get(rq_mask, prio) == 0) {
    prio++;
  }
  return prio;
}

void main_schedule() {
  tcb *cur_task = cur_scheduler->running_thread;
  /* 保证切换时中断关闭 */
  ASSERT(intr_get_status() == INTR_OFF);
  // 只是因为时间片到了
  if (cur_task->status == TASK_RUNNING) {
    task_prio_down(cur_task);
    scheduler_rq_join(cur_task);
    cur_task->ticks = cur_task->priority;
    cur_task->status = TASK_READY;
  } else {
    /* 资源抢不到而阻塞不应该再加入ready队列 */
  }

  /* 必须保证ready队列有等待的线程 */
  uint32_t prio = highest_rq_get();
  struct list_elem *tsk_tag = list_pop_front(&cur_scheduler->rq[prio]);
  if (list_empty(&cur_scheduler->rq[prio])) {
    bitmap_clear(&cur_scheduler->rq_mask, prio);
  }
  tcb *next = ELEM2ENTRY(tcb, general_tag, tsk_tag);
  next->status = TASK_RUNNING;
  cur_scheduler->running_thread = next;
  /* 更新页目录表和更新tss0特权级的栈指针 */
  process_activate(next);
  context_load(next);
}