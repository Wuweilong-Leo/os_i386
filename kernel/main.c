#include "console.h"
#include "debug.h"
#include "init.h"
#include "interrupt.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "thread.h"

void kthread1(void *);
void kthread2(void *);
int main() {
  put_str("I AM KERNEL\n");
  init_all();
  thread_run("kthread1", 31, kthread1, "arg1_");
  thread_run("kthread2", 31, kthread2, "arg2_");

  intr_enable();

  while (1)
    ;
}

void kthread1(void *arg) {
  while (1) {
    enum intr_status int_save = intr_disable();
    if (!ioq_empty(&kbd_buf)) {
      console_put_str((char *)arg);
      console_put_char(ioq_getchar(&kbd_buf));
      console_put_char(' ');
    }
    intr_set_status(int_save);
  }
}

void kthread2(void *arg) {
  while (1) {
    enum intr_status int_save = intr_disable();
    if (!ioq_empty(&kbd_buf)) {
      console_put_str((char *)arg);
      console_put_char(ioq_getchar(&kbd_buf));
      console_put_char(' ');
    }
    intr_set_status(int_save);
  }
}