#include "timer.h"
#include "debug.h"
#include "interrupt.h"
#include "io.h"
#include "print.h"
#include "thread.h"

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE (INPUT_FREQUENCY / IRQ0_FREQUENCY) // 初始计数值
#define COUNTER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

uint32_t ticks = 0;

static void freq_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl,
                     uint8_t counter_mode, uint16_t counter_value) {
  outb(PIT_CONTROL_PORT,
       (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
  outb(counter_port, (uint8_t)counter_value);
  outb(counter_port, (uint8_t)(counter_value >> 8));
}

void intr_timer_handler() {
  ASSERT(RUNNING_THREAD->stack_magic == 0x19980820);
  RUNNING_THREAD->elapsed_ticks++;
  ticks++;
  if (RUNNING_THREAD->ticks == 0) {
    cur_scheduler->need_schedule = true;
  } else {
    RUNNING_THREAD->ticks--;
  }
}

void timer_init() {
  put_str("timer_init start\n");
  freq_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE,
           COUNTER0_VALUE);
  // 注册时钟中断处理函数
  intr_handler_register(0x20, intr_timer_handler);
  put_str("timer_init done\n");
}
