#ifndef MEMORY_H
#define MEMORY_H
#include "bitmap.h"
#include "stdint.h"
struct virtual_addr {
  struct bitmap vaddr_bitmap;
  uint32_t vaddr_start;
};
void mem_init();
#endif