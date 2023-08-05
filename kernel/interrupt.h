#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "stdint.h"
typedef void *intr_handler;
void idt_init();

enum intr_status { INTR_OFF, INTR_ON };

enum intr_status intr_enable();
enum intr_status intr_disable();
enum intr_status intr_set_status(enum intr_status status);
enum intr_status intr_get_status();
void intr_handler_register(uint8_t vec_no, intr_handler func);

enum intr_num {
  EXP_DIVIDE_ERROR = 0x00,
  EXP_DEBUG,
  EXP_NMI_INTERRUPT,
  EXP_BREAK_POINT,
  EXP_OVER_FLOW,
  EXP_BOUND_RANGE_EXCEEDED,
  EXP_INVALID_OPCODE,
  EXP_DEVICE_NOT_AVAILABLE,
  EXP_DOUBLE_FAULT,
  EXP_COPROCESSOR_SEGMENT_OVERRUN,
  EXP_INVALID_TSS,
  EXP_SEGMENT_NOT_PRESENT,
  EXP_STACK_SEGMENT_FAULT,
  EXP_GENERAL_PROTECTION,
  EXP_PAGE_FAULT,
  EXP_FLOATING_POINT_ERROR = 0x10,
  EXP_ALIGNMENT_CHECK,
  EXP_MACHINE_CHECK,
  EXP_SIMD_FLOATING_POINT_EXCEPTION,
  INT_TIMER = 0x20,
  INT_KEYBOARD = 0x21,
  INT_SYSCALL = 0x80,
};

#endif