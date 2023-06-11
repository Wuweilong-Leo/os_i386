#include "bitmap.h"
#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "stdint.h"
#include "string.h"

void bitmap_init(struct bitmap *btmp) { memset(btmp, 0, btmp->btmp_bytes_len); }

bool bitmap_scan_test(struct bitmap *btmp, uint32_t bit_idx) {
  uint32_t byte_off = bit_idx / 8;
  uint32_t bit_off = bit_idx % 8;
  uint8_t *base = btmp->bits;
  return base[byte_off] & (BITMAP_MASK << bit_off);
}

// 连续申请cnt个位，返回开始索引
int32_t bitmap_scan(struct bitmap *btmp, uint32_t cnt) {
  uint32_t cur_idx = 0;
  while (cur_idx < (btmp->btmp_bytes_len * 8)) {
    uint32_t cnt_tmp = 0;
    while (bitmap_scan_test(btmp, cur_idx)) {
      cur_idx++;
    }
    while (cur_idx < (btmp->btmp_bytes_len * 8) &&
           !bitmap_scan_test(btmp, cur_idx)) {
      cnt_tmp++;
      if (cnt_tmp == cnt) {
        return cur_idx - cnt + 1;
      }
      cur_idx++;
    }
  }
  return -1;
}

void bitmap_set(struct bitmap *btmp, uint32_t idx, bool val) {
  uint32_t byte_off = idx / 8;
  uint32_t bit_off = idx % 8;
  uint8_t *base = btmp->bits;
  if (val == 1) {
    base[byte_off] |= BITMAP_MASK << bit_off;
  } else {
    base[byte_off] &= ~(BITMAP_MASK << bit_off);
  }
}