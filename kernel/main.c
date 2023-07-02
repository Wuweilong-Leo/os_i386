#include "console.h"
#include "debug.h"
#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "thread.h"
void kthread1(void *);
void kthread2(void *);
int main() {
  put_str("I AM KERNEL\n");
  init_all();
  thread_run("kthread1", 128, kthread1, "arg1  ");
  thread_run("kthread2", 31, kthread2, "arg2  ");

  intr_enable();

  while (1) {
    console_put_str(" main ");
  }
}

void kthread1(void *arg) {
  char *str = (char *)arg;
  while (1) {
    console_put_str(str);
  }
}

void kthread2(void *arg) {
  char *str = (char *)arg;
  while (1) {
    console_put_str(str);
  }
}