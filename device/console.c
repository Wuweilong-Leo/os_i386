#include "console.h"
#include "print.h"
#include "stdint.h"
#include "sync.h"
#include "thread.h"

static struct mutex console_mutex;

void console_init() { mutex_init(&console_mutex); }

void console_acquire() { mutex_lock(&console_mutex); }

void console_release() { mutex_unlock(&console_mutex); }

void console_put_str(char *str) {
  console_acquire(&console_mutex);
  put_str(str);
  console_release(&console_mutex);
}

void console_put_char(char c) {
  console_acquire(&console_mutex);
  put_char(c);
  console_release(&console_mutex);
}

void console_put_int(int num) {
  console_acquire(&console_mutex);
  put_int(num);
  console_release(&console_mutex);
}
