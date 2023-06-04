#include "stdint.h"
#include "global.h"
#include "interrupt.h"
#include "io.h"
#define IDT_DESC_CNT 0X21

struct gate_desc
{
    uint16_t func_offset_low_word;
    uint16_t selector;
    uint8_t dcount;
    uint8_t attribute;
    uint16_t func_offset_high_word;
};

static struct gate_desc idt[IDT_DESC_CNT];

extern intr_handler intr_entry_table[IDT_DESC_CNT];

static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attr, intr_handler func)
{
    p_gdesc->func_offset_low_word = (uint32_t)func & 0x0000ffff;
    p_gdesc->selector = SELECTOR_K_CODE;
    p_gdesc->dcount = 0;
    p_gdesc->attribute = attr;
    p_gdesc->func_offset_high_word = ((uint32_t)func & 0xffff0000) >> 16;
}

static void idt_desc_init()
{
    uint32_t i;
    for (i = 0; i < IDT_DESC_CNT; i++)
    {
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
    }
    put_str("   idt_desc_init done\n");
}

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

static void pic_init()
{
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
    put_str("   pic_init done\n");
}
void idt_init()
{
    put_str("idt_init start\n");
    idt_desc_init();
    pic_init();

    uint64_t idt_base_addr = ((uint64_t)idt << 16) & 0xffffffff0000;
    uint64_t idt_limit = sizeof(idt) - 1;
    uint64_t idt_oper = idt_base_addr | idt_limit;
    asm volatile("lidt %0" ::"m"(idt_oper));
    put_str("idt_init done\n");
}
