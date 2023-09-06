#include "sync.h"
#include "thread.h"

void sem_init(struct semaphore *sem, uint8_t val) {
  sem->val = val;
  list_init(&sem->waiters);
}

void lock_init(struct lock *lck) {
  lck->holder = NULL;
  sem_init(&lck->sem, 1);
}

void sem_down(struct semaphore *sem) {
  enum intr_status intr_save = intr_disable();
  // 用while防止虚假唤醒
  while (sem->val == 0) {
    list_push_back(&sem->waiters, &cur_scheduler->running_thread->general_tag);
    // 这里切换到其它线程了。
    thread_block(TASK_BLOCKED);
  }
  sem->val--;
  intr_set_status(intr_save);
}

void sem_up(struct semaphore *sem) {
  enum intr_status intr_save = intr_disable();
  ASSERT(sem->val == 0);
  if (!list_empty(&sem->waiters)) {
    tcb *thread_blocked =
        ELEM2ENTRY(tcb, general_tag, list_pop_front(&sem->waiters));
    thread_unblock(thread_blocked);
  }
  sem->val++;
  intr_set_status(intr_save);
}

void lock_acquire(struct lock *lck) {
  sem_down(&lck->sem);
  lck->holder = cur_scheduler->running_thread;
}

void lock_release(struct lock *lck) { sem_up(&lck->sem); }
