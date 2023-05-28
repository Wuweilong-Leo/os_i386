#include "print.h"
#define asm __asm
#define CRT_ADDR_REG 0x03d4
#define CRT_DATA_REG 0x03d5
#define CUR_POS_HIGH_INDEX 0x0e
#define CUR_POS_LOW_INDEX 0x0f
#define BLK_BACK_WHT_WORD 0x07 // 黑底白字
volatile uint8_t *const video_base_addr = (volatile uint8_t *const)0xc00b8000;
uint8_t *roll_video_src_addr = (uint8_t *)0xc00b80a0;
uint8_t *roll_video_dst_addr = (uint8_t *)0xc00b8000;

static void set_cursor(uint16_t target)
{
    uint8_t high = (target >> 8) & 0xff;
    uint8_t low = target & 0xff;
    uint16_t crt_addr_reg = CRT_ADDR_REG;
    uint16_t crt_data_reg = CRT_DATA_REG;
    uint8_t cur_pos_high_index = CUR_POS_HIGH_INDEX;
    uint8_t cur_pos_low_index = CUR_POS_LOW_INDEX;
    asm volatile("outb %%al, %%dx" ::"d"(crt_addr_reg), "a"(cur_pos_high_index));
    asm volatile("outb %%al, %%dx" ::"d"(crt_data_reg), "a"(high));
    asm volatile("outb %%al, %%dx" ::"d"(crt_addr_reg), "a"(cur_pos_low_index));
    asm volatile("outb %%al, %%dx" ::"d"(crt_data_reg), "a"(low));
}

static uint16_t get_cursor()
{
    uint16_t cur_pos_low = 0, cur_pos_high = 0;
    uint16_t crt_addr_reg = CRT_ADDR_REG;
    uint16_t crt_data_reg = CRT_DATA_REG;
    uint8_t cur_pos_high_index = CUR_POS_HIGH_INDEX;
    uint8_t cur_pos_low_index = CUR_POS_LOW_INDEX;
    asm volatile("outb %%al, %%dx" ::"d"(crt_addr_reg), "a"(cur_pos_high_index));
    asm volatile("inb %%dx, %%al"
                 : "=a"(cur_pos_high)
                 : "d"(crt_data_reg));
    asm volatile("outb %%al, %%dx" ::"d"(crt_addr_reg), "a"(cur_pos_low_index));
    asm volatile("inb %%dx, %%al"
                 : "=a"(cur_pos_low)
                 : "d"(crt_data_reg));
    return ((cur_pos_high << 8) & 0xff00) | (cur_pos_low & 0x00ff);
}

static void memcpy(void *dst, void *src, uint32_t size)
{
    uint8_t *dst_u8 = (uint8_t *)dst;
    uint8_t *src_u8 = (uint8_t *)src;
    for (uint32_t i = 0; i < size; i++)
    {
        dst_u8[i] = src_u8[i];
    }
}

#define SCREEN_COL_MAX 80
#define SCREEN_ROW_MAX 25
#define SCREEN_MAX (SCREEN_ROW_MAX * SCREEN_COL_MAX)
#define ROLL_SCREEN memcpy(roll_video_dst_addr, roll_video_src_addr, 960 * 4);

static void clean_last_line()
{
    uint32_t offset = 3840;
    for (uint32_t i = 0; i < 80; i++)
    {
        video_base_addr[offset++] = ' ';
        video_base_addr[offset++] = BLK_BACK_WHT_WORD;
    }
}

#define CHECK_OUT_OF_SCREEN(next_cur_pos)               \
    do                                                  \
    {                                                   \
        if (next_cur_pos >= SCREEN_MAX)                 \
        {                                               \
            ROLL_SCREEN;                                \
            clean_last_line();                          \
            next_cur_pos = SCREEN_MAX - SCREEN_COL_MAX; \
        }                                               \
    } while (0);

void put_char(uint8_t c)
{
    uint16_t cur_pos = get_cursor();
    uint16_t next_cur_pos;
    if (c == '\r' || c == '\n')
    {
        next_cur_pos = cur_pos - cur_pos % 80 + 80;
        CHECK_OUT_OF_SCREEN(next_cur_pos);
    }
    else if (c == '\b')
    {
        uint32_t offset = (cur_pos - 1) * 2;
        video_base_addr[offset] = ' ';
        video_base_addr[offset + 1] = BLK_BACK_WHT_WORD;
        next_cur_pos = cur_pos;
    }
    else
    {
        uint32_t offset = cur_pos * 2;
        video_base_addr[offset] = c;
        video_base_addr[offset + 1] = BLK_BACK_WHT_WORD;
        next_cur_pos = cur_pos + 1;
        CHECK_OUT_OF_SCREEN(next_cur_pos);
    }
    set_cursor(next_cur_pos);
}

void put_str(char *message)
{
    char *tmp = message;
    while (*tmp != 0)
    {
        put_char(*tmp);
        tmp++;
    }
}

void put_int(uint32_t num)
{
    char for_print[9] = {0};
    uint8_t off = 7;
    uint32_t num_tmp = num;
    if (num_tmp == 0)
    {
        for_print[off--] = '0';
    }
    while (num_tmp != 0)
    {
        uint8_t low = num_tmp & 0xf;
        char p;
        if (low >= 0 && low <= 9)
        {
            p = low + '0';
        }
        else
        {
            p = low + 'A';
        }
        for_print[off] = p;
        off--;
        num_tmp >>= 4;
    }
    put_str(for_print + (++off));
}