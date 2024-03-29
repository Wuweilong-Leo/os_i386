#include "ioqueue.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "stdint.h"

void ioqueue_init(struct ioqueue *q) {
  mutex_init(&q->mtx);
  q->producer = NULL;
  q->consumer = NULL;
  q->head = 0;
  q->tail = 0;
}

INLINE uint32_t next_pos_get(uint32_t pos) { return (pos + 1) % BUF_SIZE; }

bool ioq_full(struct ioqueue *q) { return next_pos_get(q->head) == q->tail; }

bool ioq_empty(struct ioqueue *q) { return q->head == q->tail; }

static void ioq_wait(tcb **waiter) {
  *waiter = RUNNING_THREAD;
  thread_block(TASK_BLOCKED);
}

static void ioq_signal(tcb **waiter) {
  thread_unblock(*waiter);
  *waiter = NULL;
}

// 外部保证关中断
char ioq_getchar(struct ioqueue *q) {
  // 如果缓冲区为空，先阻塞此线程。
  while (ioq_empty(q)) {
    mutex_lock(&q->mtx);
    ioq_wait(&q->consumer);
    mutex_unlock(&q->mtx);
  }

  char byte = q->buf[q->tail];
  q->tail = next_pos_get(q->tail);

  // 不是空，说明有线程阻塞。
  if (q->producer != NULL) {
    ioq_signal(&q->producer);
  }

  return byte;
}

// 外部保证关中断
void ioq_putchar(struct ioqueue *q, char byte) {
  while (ioq_full(q)) {
    mutex_lock(&q->mtx);
    ioq_wait(&q->producer);
    mutex_unlock(&q->mtx);
  }
  q->buf[q->head] = byte;
  q->head = next_pos_get(q->head);

  if (q->consumer != NULL) {
    ioq_signal(&q->consumer);
  }
}
