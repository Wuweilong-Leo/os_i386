#include "process.h"
#include "bitmap.h"
#include "console.h"
#include "global.h"
#include "memory.h"
#include "thread.h"
#include "tss.h"

extern void intr_exit();

/*
 * 用户程序入口，如果是用户进程，会通过kernel_thread_entry进入此函数, filename
 * 作为参数
 */
void process_entry(void *filename) {
  tcb *running_thread = cur_scheduler->running_thread;
  void *func = filename;
  uint32_t stack = (uint32_t)running_thread->self_kstack;
  stack += sizeof(struct thread_stack);
  running_thread->self_kstack = (uint32_t *)stack;
  struct intr_stack *proc_stack =
      (struct intr_stack *)running_thread->self_kstack;
  proc_stack->edi = 0;
  proc_stack->esi = 0;
  proc_stack->ebp = 0;
  proc_stack->esp_dummy = 0;
  proc_stack->ebx = 0;
  proc_stack->edx = 0;
  proc_stack->ecx = 0;
  proc_stack->eax = 0;
  proc_stack->gs = 0;
  proc_stack->ds = SELECTOR_U_DATA;
  proc_stack->es = SELECTOR_U_DATA;
  proc_stack->fs = SELECTOR_U_DATA;
  proc_stack->eip = func;
  proc_stack->cs = SELECTOR_U_CODE;
  proc_stack->eflags = EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1;
  /* 分配栈给用户进程 */
  proc_stack->esp = target_vaddr_malloc(PF_USER, USER_STACK3_VADDR) + PG_SIZE;
  proc_stack->ss = SELECTOR_U_DATA;
  /* 此时还在内核态，通过中断出口去进入用户态 */
  asm volatile("movl %0, %%esp; jmp intr_exit" ::"g"(proc_stack) : "memory");
}

/* 加载页目录地址到cr3 */
void page_dir_activate(tcb *thread) {
  uint32_t page_dir_phy_addr;
  if (thread->pg_dir != NULL) {
    page_dir_phy_addr = addr_v2p((uint32_t)thread->pg_dir);
  } else {
    page_dir_phy_addr = KERNEL_PAGE_DIR_BASE_ADDR;
  }
  asm volatile("movl %0, %%cr3" ::"r"(page_dir_phy_addr) : "memory");
}

/* 更新页目录表且把0特权级esp填入tss中，从而硬件可以自动圧栈 */
void process_activate(tcb *thread) {
  page_dir_activate(thread);
  if (thread->pg_dir != NULL) {
    tss_esp0_update(thread);
  }
}

/* 从内核空间申请一块内存作为页目录 */
static uint32_t *page_dir_create() {
  /* 申请一页内存作为页目录表 */
  uint32_t *page_dir_vaddr = kernel_pages_malloc(1);
  if (page_dir_vaddr == NULL) {
    return NULL;
  }

  /* 复制页目录的768项，保证用户页表高1g能映射到内核 */
  memcpy((uint32_t *)((uint32_t)page_dir_vaddr + 0x300 * 4),
         (uint32_t *)(0xfffff000 + 0x300 * 4), 1024);

  /* 最后一项写入页目录物理地址 */
  uint32_t page_dir_phy_addr = addr_v2p((uint32_t)page_dir_vaddr);
  page_dir_vaddr[1023] = page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1;
  return page_dir_vaddr;
}

/* 创建用户进程的虚拟地址池的位图 */
void user_vaddr_bitmap_create(tcb *task) {
  task->user_vaddr_pool.addr_base = USER_VADDR_START;
  uint32_t user_free_pages_num = (0xc0000000 - USER_VADDR_START) / PG_SIZE;
  uint32_t bitmap_pages_need = ((user_free_pages_num / 8 + 1) / PG_SIZE) + 1;
  void *user_vaddr_bitmap_base = kernel_pages_malloc(bitmap_pages_need);
  bitmap_init(&task->user_vaddr_pool.btmp, user_vaddr_bitmap_base,
              user_free_pages_num);
}

void process_run(void *filename, char *name) {
  tcb *task = kernel_pages_malloc(1);
  thread_init(task, name, 31, process_entry, filename);
  user_vaddr_bitmap_create(task);
  task->pg_dir = page_dir_create();

  enum intr_status int_save = intr_disable();
  scheduler_rq_join(task);
  list_push_back(&cur_scheduler->all_list, &task->all_list_tag);
  intr_set_status(int_save);
}
