#ifndef BIT_MAP_H
#define BIT_MAP_H
#define BITMAP_MASK 1
#include "stdint.h"
struct bitmap {
  uint32_t btmp_bytes_len;
  uint8_t *bits;
};
void bitmap_init(struct bitmap *btmp);
bool bitmap_scan_test(struct bitmap *btmp, uint32_t bit_idx);
int32_t bitmap_scan(struct bitmap *btmp, uint32_t cnt);
void bitmap_set(struct bitmap *btmp, uint32_t idx, bool val);
#endif