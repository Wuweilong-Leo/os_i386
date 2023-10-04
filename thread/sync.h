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

struct mutex {
  void *holder; /* 本来是tcb类型的指针，但是防止头文件循环包含，选择void* */
  struct semaphore sem;
};

void mutex_init(struct mutex *mtx);
void mutex_lock(struct mutex *mtx);
void mutex_unlock(struct mutex *mtx);

#endif