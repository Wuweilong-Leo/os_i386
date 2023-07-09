#ifndef IOQUEUE_H
#define IOQUEUE_H
#include "stdint.h"
#include "sync.h"
#include "thread.h"

#define BUF_SIZE 64

struct ioqueue {
  struct lock lck;
  tcb *producer;
  tcb *consumer;
  char buf[BUF_SIZE];
  uint32_t head;
  uint32_t tail;
};
void ioq_putchar(struct ioqueue *q, char byte);
char ioq_getchar(struct ioqueue *q);
void ioqueue_init(struct ioqueue *q);
bool ioq_empty(struct ioqueue *q);
#endif