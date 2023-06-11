#ifndef BIT_MAP_H
#define BIT_MAP_H
#include "global.h"
#define BITMAP_MASK 1
struct bitmap {
  uint32_t btmp_bytes_len;
  uint8_t *bits;
};

#endif