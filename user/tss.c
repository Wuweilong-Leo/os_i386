#include "global.h"
#include "print.h"
#include "string.h"
#include "thread.h"

/*
 *  TR寄存器存储TSS段的选择子，GDT里存储TSS段的描述符，描述符里有tss的基地址
 */
#define GDT_ENTRY_SIZE 8
const void *gdt_base_addr = (const void *)0xc0000900;

struct tss {
  uint32_t backlink;
  uint32_t *esp0;
  uint32_t ss0;
  uint32_t *esp1;
  uint32_t ss1;
  uint32_t *esp2;
  uint32_t ss2;
  uint32_t cr3;
  uint32_t (*eip)(void);
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t es;
  uint32_t cs;
  uint32_t ss;
  uint32_t ds;
  uint32_t fs;
  uint32_t gs;
  uint32_t ldt;
  uint32_t trace;
  uint32_t io_base;
};

static struct tss tss;

/* 写入0特权级的栈指针, 即内核栈最高处 => 栈底, 发生中断以后自动找到内核栈*/
void tss_esp0_update(tcb *thread) {
  tss.esp0 = (uint32_t *)((uint32_t)thread + PG_SIZE);
}

static void gdt_desc_make(struct gdt_desc *desc, uint32_t *desc_addr,
                          uint32_t limit, uint8_t attr_low, uint8_t attr_high) {
  uint32_t desc_base = (uint32_t)desc_addr;
  desc->limit_low_word = limit & 0xffff;
  desc->base_low_word = desc_base & 0xffff;
  desc->base_mid_byte = ((desc_base & 0x00ff0000) >> 16);
  desc->attr_low_byte = (uint8_t)attr_low;
  desc->limit_high_attr_high =
      ((limit & 0x000f0000) >> 16) + (uint8_t)attr_high;
  desc->base_high_byte = desc_base >> 24;
}

/*
 *  在gdt中增加tss段，用户代码段，用户数据段。
 */
void tss_init() {
  put_str("tss init begin\n");
  uint32_t tss_size = sizeof(tss);
  memset(&tss, 0, tss_size);
  tss.ss0 = SELECTOR_K_STACK;
  tss.io_base = tss_size;
  struct gdt_desc *gdt_base = (struct gdt_desc *)gdt_base_addr;
  /* 不用硬件的tss机制，只用一个tss */
  gdt_desc_make(&gdt_base[4], (uint32_t *)&tss, tss_size - 1, TSS_ATTR_LOW,
                TSS_ATTR_HIGH);
  gdt_desc_make(&gdt_base[5], 0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3,
                GDT_ATTR_HIGH);
  gdt_desc_make(&gdt_base[6], 0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3,
                GDT_ATTR_HIGH);
  uint64_t gdt_base_addr_oper = ((uint64_t)gdt_base_addr) << 16;
  /* 8 * 7 - 1 = 55 */
  uint64_t gdt_limit_oper = 55;
  uint64_t gdt_oper = gdt_base_addr_oper | gdt_limit_oper;

  asm volatile("lgdt %0" ::"m"(gdt_oper));
  asm volatile("ltr %w0" ::"r"(SELECTOR_TSS));
  put_str("tss_init done\n");
}