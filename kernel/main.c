#include "debug.h"
#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
int main() {
  put_str("I AM KERNEL\n");
  init_all();
  void *addr = kernel_malloc(3);
  put_str("\n kernel_malloc start vaddr is ");
  put_int((uint32_t)addr);
  put_str("\n");
  while (1)
    ;
}