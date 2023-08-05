#include "init.h"
#include "console.h"
#include "interrupt.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "syscall_init.h"
#include "thread.h"
#include "timer.h"
#include "tss.h"

void init_all() {
  put_str("init_all\n");
  idt_init();
  mem_init();
  thread_all_init();
  timer_init();
  console_init();
  keyboard_init();
  tss_init();
  syscall_init();
}