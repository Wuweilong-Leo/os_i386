#include "interrupt.h"
#include "global.h"
#include "io.h"
#include "print.h"
#include "stdint.h"
#define IDT_DESC_CNT 0X81

#ifndef NULL
#define NULL (void *)0
#endif

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

#define EFLAGS_IF ((uint32_t)0x200)
#define GET_EFLAGS(eflags_var) asm volatile("pushf; popl %0" : "=g"(eflags_var))

extern uint32_t syscall_entry();

struct gate_desc {
  uint16_t func_offset_low_word;
  uint16_t selector;
  uint8_t dcount;
  uint8_t attribute;
  uint16_t func_offset_high_word;
};

// 中断描述符表
static struct gate_desc idt[IDT_DESC_CNT] = {0};

// 存储各中断入口函数的地址，定义在kernel.s中
extern intr_handler intr_entry_table[IDT_DESC_CNT];

// 中断名
char *intr_name[IDT_DESC_CNT] = {"#DE",   "#DB",
                                 "#NMI",  "#BP",
                                 "#OF",   "#BR",
                                 "#UD",   "#NM",
                                 "#DF",   "#CSO",
                                 "TS",    "NP",
                                 "SS",    "GP",
                                 "PF",    NULL,
                                 "MF",    "AC",
                                 "MC",    "XF",
                                 NULL,    NULL,
                                 NULL,    NULL,
                                 NULL,    NULL,
                                 NULL,    NULL,
                                 NULL,    NULL,
                                 NULL,    NULL,
                                 "TIMER", [0x80] = "SYSCALL"};

// 存放真正的中断处理函数，中断入口会跳转到此处
intr_handler idt_table[IDT_DESC_CNT] = {0};

// 中断描述符制作。
static void idt_desc_make(struct gate_desc *p_gdesc, uint8_t attr,
                          intr_handler func) {
  p_gdesc->func_offset_low_word = (uint32_t)func & 0x0000ffff;
  p_gdesc->selector = SELECTOR_K_CODE;
  p_gdesc->dcount = 0;
  p_gdesc->attribute = attr;
  p_gdesc->func_offset_high_word = ((uint32_t)func & 0xffff0000) >> 16;
}

// 把中断入口写入idt中
static void idt_desc_init() {
  put_str("   idt_desc_init begin\n");
  for (uint32_t i = 0; i < IDT_DESC_CNT; i++) {
    idt_desc_make(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
  }
  idt_desc_make(&idt[INT_SYSCALL], IDT_DESC_ATTR_DPL3, syscall_entry);
  put_str("   idt_desc_init done\n");
}

static void pic_init() {
  outb(PIC_M_CTRL, 0x11);
  outb(PIC_M_DATA, 0x20);
  outb(PIC_M_DATA, 0x04);
  outb(PIC_M_DATA, 0x01);
  outb(PIC_S_CTRL, 0x11);
  outb(PIC_S_DATA, 0x28);
  outb(PIC_S_DATA, 0x02);
  outb(PIC_S_DATA, 0x01);
  outb(PIC_M_DATA, 0xfe);
  outb(PIC_S_DATA, 0xff);
  outb(PIC_M_DATA, 0xfc);
  outb(PIC_S_DATA, 0xff);
  put_str("   pic_init done\n");
}

// 如果没有注册中断处理函数，则使用是个默认中断处理函数。
static void general_intr_handler(uint8_t vec_nr) {
  if (vec_nr == 0x27 || vec_nr == 0x2f) {
    return;
  }
  set_cursor(0);
  uint32_t cursor_pos = 0;
  while (cursor_pos < 320) {
    put_char(' ');
    cursor_pos++;
  }
  set_cursor(0);
  put_str("!!!!!!!  exception message begin !!!!!!!\n");
  set_cursor(88);
  put_str(intr_name[vec_nr]);
  if (vec_nr == EXP_PAGE_FAULT) {
    uint32_t page_fault_vaddr = 0;
    asm volatile("movl %%cr2, %0" : "=r"(page_fault_vaddr));
    put_str("\npage fault vaddr is ");
    put_int(page_fault_vaddr);
  }
  put_str("\n!!!!!!!  exception message end !!!!!!!\n");
  while (1)
    ;
}

static void exception_init() {
  for (uint32_t i = 0; i < IDT_DESC_CNT; i++) {
    idt_table[i] = general_intr_handler;
  }
}

void idt_init() {
  put_str("idt_init start\n");
  idt_desc_init();
  exception_init();
  pic_init();
  uint64_t idt_base_addr = (((uint64_t)idt) << 16) & 0xffffffff0000;
  uint64_t idt_limit = sizeof(idt) - 1;
  uint64_t idt_oper = idt_base_addr | idt_limit;
  asm volatile("lidt %0" ::"m"(idt_oper));
  put_str("idt_init done\n");
}

enum intr_status intr_get_status() {
  uint32_t eflags = 0;
  GET_EFLAGS(eflags);
  return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

enum intr_status intr_disable() {
  if (INTR_ON == intr_get_status()) {
    asm volatile("cli" ::: "memory");
    return INTR_ON;
  } else {
    return INTR_OFF;
  }
}

enum intr_status intr_enable() {
  if (INTR_OFF == intr_get_status()) {
    asm volatile("sti");
    return INTR_OFF;
  } else {
    return INTR_ON;
  }
}

enum intr_status intr_set_status(enum intr_status status) {
  return (status == INTR_ON) ? intr_enable() : intr_disable();
}

void intr_handler_register(uint8_t vec_no, intr_handler func) {
  idt_table[vec_no] = func;
}

/* handle hardware interrupt tail */
void hwi_tail_handle() { main_schedule(); }