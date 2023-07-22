#ifndef PROCESS_H
#define PROCESS_H
#include "thread.h"
/* 用户栈一般的内存起始虚拟地址 */
#define USER_STACK3_VADDR (0xc0000000 - 0x1000)
#define USER_VADDR_START 0x8048000
void process_activate(tcb *thread);
#define KERNEL_PAGE_DIR_BASE_ADDR 0x100000
void process_run(void *filename, char *name);
#endif