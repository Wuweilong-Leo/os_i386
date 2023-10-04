#include "sync.h"
#include "thread.h"

void sem_init(struct semaphore *sem, uint8_t val) {
  sem->val = val;
  list_init(&sem->waiters);
}

void mutex_init(struct mutex *mtx) {
  mtx->holder = NULL;
  sem_init(&mtx->sem, 1);
}

void sem_down(struct semaphore *sem) {
  enum intr_status int_save = intr_disable();
  // 用while防止虚假唤醒
  while (sem->val == 0) {
    if (!list_elem_find(&sem->waiters, &RUNNING_THREAD->general_tag)) {
      list_push_back(&sem->waiters, &RUNNING_THREAD->general_tag);
    }
    // 这里切换到其它线程了。
    thread_block(TASK_BLOCKED);
  }
  sem->val--;
  intr_set_status(int_save);
}

void sem_up(struct semaphore *sem) {
  enum intr_status int_save = intr_disable();
  ASSERT(sem->val == 0);
  if (!list_empty(&sem->waiters)) {
    tcb *thread_blocked =
        ELEM2ENTRY(tcb, general_tag, list_pop_front(&sem->waiters));
    thread_unblock(thread_blocked);
  }
  sem->val++;
  intr_set_status(int_save);
}

void mutex_lock(struct mutex *mtx) {
  sem_down(&mtx->sem);
  mtx->holder = RUNNING_THREAD;
}

void mutex_unlock(struct mutex *mtx) {
  sem_up(&mtx->sem);
  mtx->holder = NULL;
}
