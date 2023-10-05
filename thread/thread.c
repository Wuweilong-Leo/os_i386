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
 * schedule chance:
 *        1. time slice
 *        2. can not acquire lock
 */

/* 优先级反馈调度器 */
struct scheduler scheduler;

struct scheduler *cur_scheduler = &scheduler;

void scheduler_init() {
  RUNNING_THREAD = NULL;
  cur_scheduler->need_schedule = false;
  cur_scheduler->idle_thread = NULL;
  memset(cur_scheduler->thread_table, 0, sizeof(tcb *) * MAX_THREAD_NUM);
  uint8_t *btmp_base = (uint8_t *)kernel_pages_malloc(1);
  bitmap_init(RQ_MASK_BITMAP, btmp_base, PRIO_NUM);
  for (uint32_t i = 0; i < PRIO_NUM; i++) {
    list_init(THIS_RQ(i));
  }
  list_init(&cur_scheduler->all_list);
  mutex_init(&cur_scheduler->pid_mtx);
}

/* 排队 */
void scheduler_rq_join(tcb *thread) {
  uint32_t prio = thread->priority;
  list_push_back(THIS_RQ(prio), &thread->ready_tag);
  bitmap_set(RQ_MASK_BITMAP, prio);
  if (RUNNING_THREAD->priority > prio) {
    cur_scheduler->need_schedule = true;
  }
}

/* 插队 */
void scheduler_rq_jump(tcb *thread) {
  uint32_t prio = thread->priority;
  list_push_front(THIS_RQ(prio), &thread->ready_tag);
  bitmap_set(RQ_MASK_BITMAP, prio);
  if (RUNNING_THREAD->priority > prio) {
    cur_scheduler->need_schedule = true;
  }
}

static void kernel_thread_entry(thread_func func, void *func_arg) {
  intr_enable(); // 每个线程都要保证初始状态中断开启
  (*func)(func_arg);
}

// 先不考虑pid不够的情况
static void pid_alloc(tcb *thread) {
  mutex_lock(&cur_scheduler->pid_mtx);
  for (uint32_t i = 0; i < MAX_THREAD_NUM; i++) {
    if (cur_scheduler->thread_table[i] == NULL) {
      thread->pid = i;
      cur_scheduler->thread_table[i] = thread;
      break;
    }
  }
  mutex_unlock(&cur_scheduler->pid_mtx);
}

void thread_tcb_init(tcb *pthread, char *name, uint8_t prio) {
  memset(pthread, 0, sizeof(*pthread));
  strcpy(pthread->name, name);
  pthread->status =
      (pthread == cur_scheduler->idle_thread) ? TASK_RUNNING : TASK_READY;
  pthread->priority = prio;
  pthread->stack_top = (uint32_t *)((uint32_t)pthread + PG_SIZE);
  pthread->ticks = ticks_get(prio); // 优先级和可执行的ticks相关
  pthread->pg_dir = NULL;
  pid_alloc(pthread);
  pthread->stack_magic = 0x19980820;
}

void thread_stack_init(tcb *pthread, thread_func func, void *func_arg) {
  uint32_t kstack_tmp = (uint32_t)(pthread->stack_top);
  /* 此处是预留出位置给用户进程 */
  kstack_tmp -= sizeof(struct intr_stack);
  kstack_tmp -= sizeof(struct thread_stack);
  pthread->stack_top = (uint32_t *)kstack_tmp;
  struct thread_stack *kthread_stack =
      (struct thread_stack *)pthread->stack_top;
  kthread_stack->eip = kernel_thread_entry;
  kthread_stack->func = func;
  kthread_stack->func_arg = func_arg;
  kthread_stack->save_flag = FAST_SAVE_FLAG;
  kthread_stack->ebp = 0;
  kthread_stack->ebx = 0;
  kthread_stack->esi = 0;
  kthread_stack->edi = 0;
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

/* init main thread as idle thread */
static void idle_thread_make() {
  RUNNING_THREAD = running_thread_get();
  cur_scheduler->idle_thread = RUNNING_THREAD;
  thread_tcb_init(cur_scheduler->idle_thread, "idle", 31);
  list_push_back(&cur_scheduler->all_list, &RUNNING_THREAD->all_list_tag);
}

void thread_all_init() {
  put_str("thread_all_init begin\n");
  scheduler_init();
  idle_thread_make();
  put_str("thread_all_init done\n");
}

// 阻塞现在的线程。
void thread_block(enum task_status tsk_status) {
  ASSERT(tsk_status == TASK_BLOCKED || tsk_status == TASK_WAITING ||
         tsk_status == TASK_HANGING);
  enum intr_status int_save = intr_disable();
  RUNNING_THREAD->status = tsk_status;
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
  cur_scheduler->need_schedule = true;
  schedule();
  intr_set_status(int_save);
}

// add thread to ready list
void thread_unblock(tcb *thread) {
  ASSERT(thread->status == TASK_BLOCKED || thread->status == TASK_WAITING ||
         thread->status == TASK_HANGING);
  enum intr_status int_save = intr_disable();
  thread->status = TASK_READY;
  scheduler_rq_jump(thread);
  intr_set_status(int_save);
}

void schedule() {
  if (!need_schedule()) {
    return;
  }
  /* save context */
  task_trap(RUNNING_THREAD);
  return;
}

static inline void thread_prio_down(tcb *thread) {
  thread->priority = (thread->priority + 1) % PRIO_NUM;
}

static inline uint32_t highest_rq_get() {
  /* 不可能出现任何队列都不存在任务的情况 */
  return (uint32_t)bitmap_scan(RQ_MASK_BITMAP, 1, 1);
}

/* pick thread with highest prio */
static inline tcb *next_thread_pick() {
  uint32_t prio = highest_rq_get();
  struct list_elem *next_tag = list_pop_front(THIS_RQ(prio));
  if (list_empty(THIS_RQ(prio))) {
    bitmap_clear(RQ_MASK_BITMAP, prio);
  }
  return ELEM2ENTRY(tcb, ready_tag, next_tag);
}

void main_schedule() {
  tcb *cur = RUNNING_THREAD;
  tcb *next = cur;

  if (need_schedule()) {
    cur_scheduler->need_schedule = false;
    /* just because of the time slice */
    if (cur->status == TASK_RUNNING) {
      thread_prio_down(cur);
      scheduler_rq_join(cur);
      cur->ticks = ticks_get(cur->priority);
      cur->status = TASK_READY;
    } else {
      /* 资源抢不到而阻塞不应该再加入ready队列 */
    }

    next = next_thread_pick();
    next->status = TASK_RUNNING;
    RUNNING_THREAD = next;
    /* 更新页目录表和更新tss0特权级的栈指针 */
    process_activate(next);
  }
  /* if not need schedule, return to original thread */
  context_load(next);
}