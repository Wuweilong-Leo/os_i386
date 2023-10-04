#ifndef MEMORY_H
#define MEMORY_H
#include "bitmap.h"
#include "list.h"
#include "stdint.h"
#include "sync.h"

struct pool {
  struct bitmap btmp;
  // 内存池管理内存的基地址
  uint32_t addr_base;
  // 内存池管理内存的字节大小
  uint32_t pool_size;
  struct mutex mtx;
};
enum pool_flags { PF_KERNEL = 1, PF_USER = 2 };
#define PG_P_1 1
#define PG_P_0 0
#define PG_RW_R 0
#define PG_RW_W 2
#define PG_US_S 0
#define PG_US_U 4
void mem_init();
void *kernel_pages_malloc(uint32_t pg_cnt);
void *user_pages_malloc(uint32_t pg_cnt);
void *target_vaddr_malloc(enum pool_flags pf, uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);
#endif