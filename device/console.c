#include "console.h"
#include "print.h"
#include "stdint.h"
#include "sync.h"
#include "thread.h"

static struct lock console_lock;

void console_init() { lock_init(&console_lock); }

void console_acquire() { lock_acquire(&console_lock); }

void console_release() { lock_release(&console_lock); }

void console_put_str(char *str) {
  console_acquire(&console_lock);
  put_str(str);
  console_release(&console_lock);
}

void console_put_char(char c) {
  console_acquire(&console_lock);
  put_char(c);
  console_release(&console_lock);
}

void console_put_int(int num) {
  console_acquire(&console_lock);
  put_int(num);
  console_release(&console_lock);
}
