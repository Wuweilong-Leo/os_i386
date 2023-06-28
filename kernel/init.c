#include "init.h"
#include "interrupt.h"
#include "memory.h"
#include "print.h"
#include "thread.h"
#include "timer.h"
void init_all() {
  put_str("init_all\n");
  idt_init();
  timer_init();
  mem_init();
  thread_all_init();
}