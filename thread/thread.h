#ifndef THREAD_H
#define THREAD_H
#include "bitmap.h"
#include "list.h"
#include "memory.h"
#include "stdint.h"
#include "sync.h"

#define MAX_THREAD_NUM 128
#define PRIO_NUM 32

#ifndef PG_SIEE
#define PG_SIZE 4096
#endif

#define PID2TCB(pid) (thread_table[pid])

#define RUNNING_THREAD (cur_scheduler->running_thread)
#define THIS_RQ(prio) (&cur_scheduler->rq[prio])
#define RQ_MASK_BITMAP (&cur_scheduler->rq_mask)

typedef void (*thread_func)(void *);
typedef uint16_t pid_t;

enum task_status {
  TASK_RUNNING,
  TASK_READY,
  TASK_BLOCKED,
  TASK_WAITING,
  TASK_HANGING,
  TASK_DIED
};

// 发生中断之后的栈
struct intr_stack {
  uint32_t vec_no;
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp_dummy;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t gs;
  uint32_t fs;
  uint32_t es;
  uint32_t ds;
  uint32_t err_code;
  void (*eip)(void);
  uint32_t cs;
  uint32_t eflags;
  void *esp;
  uint32_t ss;
};

struct thread_stack {
  uint32_t ebp;
  uint32_t ebx;
  uint32_t edi;
  uint32_t esi;
  void (*eip)(thread_func, void *); // ret 会把这个函数作为返回地址执行
  void *unused_ret_addr;
  thread_func func;
  void *func_arg;
};

// 线程控制块
typedef struct thread_control_block {
  uint32_t *stack_top;
  pid_t pid;
  enum task_status status;
  uint8_t priority;
  char name[16];
  uint32_t ticks;
  uint32_t elapsed_ticks;
  struct list_elem general_tag;
  struct list_elem all_list_tag;
  uint32_t *pg_dir;
  struct pool user_vaddr_pool;
  uint32_t stack_magic;
} tcb;

struct scheduler {
  tcb *running_thread;
  tcb *main_thread;
  tcb *thread_table[MAX_THREAD_NUM];
  struct bitmap rq_mask;
  /* ready queue */
  struct list rq[PRIO_NUM];
  struct list all_list;
  struct lock pid_lock;
} scheduler;

extern struct scheduler *cur_scheduler;
void scheduler_rq_join(tcb *thread);
void scheduler_rq_jump(tcb *thread);
tcb *thread_run(char *name, uint8_t prio, thread_func func, void *func_arg);
void thread_all_init();
tcb *running_thread_get();
void thread_block(enum task_status tsk_status);
void thread_unblock(tcb *tsk);
void thread_tcb_init(tcb *pthread, char *name, uint8_t prio);
void thread_stack_init(tcb *pthread, thread_func func, void *func_arg);
void thread_init(tcb *pthread, char *name, uint8_t prio, thread_func func,
                 void *func_arg);

static inline uint32_t ticks_get(uint32_t prio) { return prio / 2 + 1; }

#endif