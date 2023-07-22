#ifndef SYNC_H
#define SYNC_H
#include "debug.h"
#include "interrupt.h"
#include "list.h"
#include "stdint.h"

// 信号量
struct semaphore {
  uint8_t val;
  struct list waiters;
};

struct lock {
  void *holder; /* 本来是tcb类型的指针，但是防止头文件循环包含，选择void* */
  struct semaphore sem;
};

void lock_init(struct lock *lck);
void lock_acquire(struct lock *lck);
void lock_release(struct lock *lck);

#endif