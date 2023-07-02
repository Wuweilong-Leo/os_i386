#ifndef SYNC_H
#define SYNC_H
#include "debug.h"
#include "interrupt.h"
#include "list.h"
#include "stdint.h"
#include "thread.h"

// 信号量
struct semaphore {
  uint8_t val;
  struct list waiters;
};

struct lock {
  tcb *holder;
  struct semaphore sem;
};

#endif