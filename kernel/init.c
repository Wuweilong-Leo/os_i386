#include "init.h"
#include "console.h"
#include "interrupt.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "thread.h"
#include "timer.h"
void init_all() {
  put_str("init_all\n");
  idt_init();
  mem_init();
  thread_all_init();
  timer_init();
  console_init();
  keyboard_init();
}