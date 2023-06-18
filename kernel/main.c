#include "debug.h"
#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "thread.h"
void kthread1(void *);

int main() {
  put_str("I AM KERNEL\n");
  init_all();
  char *vaddr = (char *)kernel_malloc(3);
  thread_run("kthread1", 31, kthread1, "arg1  ");
  while (1)
    ;
}

void kthread1(void *arg) {
  char *str = (char *)arg;
  while (1) {
    put_str(str);
  }
}