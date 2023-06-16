#ifndef MEMORY_H
#define MEMORY_H
#include "stdint.h"
#include "bitmap.h"
struct virtual_addr{
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};
void mem_init();
#endif