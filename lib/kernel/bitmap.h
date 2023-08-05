#ifndef BIT_MAP_H
#define BIT_MAP_H
#include "stdint.h"

struct bitmap {
  /* 位图基地址 */
  uint8_t *base_addr;
  /* 位图的位数 */
  uint32_t bit_num;
  /* 位图的字节数 */
  uint32_t byte_len;
};

#define BITMAP_MASK 1

static inline uint32_t bit_num_2_byte_len(uint32_t bit_num) {
  return (bit_num % 8) ? (bit_num / 8 + 1) : (bit_num / 8);
}

void bitmap_init(struct bitmap *btmp, uint32_t base, uint32_t bit_num);
uint32_t bitmap_get(struct bitmap *btmp, uint32_t bit_idx);
int32_t bitmap_scan(struct bitmap *btmp, uint32_t cnt);
void bitmap_set(struct bitmap *btmp, uint32_t idx);
void bitmap_clear(struct bitmap *btmp, uint32_t idx);

#endif