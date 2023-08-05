#include "bitmap.h"
#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "stdint.h"
#include "string.h"

void bitmap_init(struct bitmap *btmp, uint32_t base, uint32_t bit_num) {
  btmp->base_addr = (uint8_t *)base;
  btmp->bit_num = bit_num;
  btmp->byte_len = bit_num_2_byte_len(bit_num);
  memset(btmp->base_addr, 0, btmp->byte_len);
}

uint32_t bitmap_get(struct bitmap *btmp, uint32_t bit_idx) {
  uint32_t byte_off = bit_idx / 8;
  uint32_t bit_off = bit_idx % 8;
  return (btmp->base_addr[byte_off] & (BITMAP_MASK << bit_off)) != 0;
}

/* 连续申请cnt空闲位 */
int32_t bitmap_scan(struct bitmap *btmp, uint32_t cnt) {
  uint32_t left = 0;
  uint32_t right = 0;
  while (right < btmp->bit_num) {
    if (bitmap_get(btmp, right) != 0) {
      left = right + 1;
    }
    if (right - left + 1 == cnt) {
      return left;
    }
    right++;
  }
  return -1;
}

void bitmap_set(struct bitmap *btmp, uint32_t idx) {
  uint32_t byte_off = idx / 8;
  uint32_t bit_off = idx % 8;
  btmp->base_addr[byte_off] |= (1 << bit_off);
}

void bitmap_clear(struct bitmap *btmp, uint32_t idx) {
  uint32_t byte_off = idx / 8;
  uint32_t bit_off = idx % 8;
  btmp->base_addr[byte_off] &= ~(1 << bit_off);
}