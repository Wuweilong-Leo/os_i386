#include "memory.h"
#define PG_SIZE 4096
#define MEM_BITMAP_BASE 0XC009A000
#define K_HEAP_START 0XC0100000
struct pool {
  struct bitmap pool_bitmap;
  uint32_t paddr_start;
  uint32_t pool_size;
};

struct pool kernel_pool, user_pool;
struct virtual_addr kernel_vaddr;

#define PAGE_DIR_START_ADDR 0X100000
#define PAGE_DIR_CNT 1
#define PAGE_TAB_CNT 255
static void mem_pool_init(uint32_t all_mem) {
  put_str(" mem_pool_init start\n");
  uint32_t page_table_size =
      PG_SIZE * (PAGE_DIR_CNT + PAGE_TAB_CNT); // 一共是255个页表 + 1个页目录
  uint32_t used_mem = page_table_size + PAGE_DIR_START_ADDR;
  uint32_t free_mem = all_mem - used_mem;
  uint32_t all_free_pages = free_mem / PG_SIZE;
  uint32_t kernel_free_pages = all_free_pages / 2;
  uint32_t user_free_pages = all_free_pages - kernel_free_pages;
  uint32_t kbm_len = kernel_free_pages / 8;
  uint32_t ubm_len = user_free_pages / 8;
  uint32_t kp_start = used_mem;
  uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;
  kernel_pool.paddr_start = kp_start;
  user_pool.paddr_start = up_start;
  kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
  user_pool.pool_size = user_free_pages * PG_SIZE;
  kernel_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE;
  user_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE + kbm_len;

  put_str("   kernel_pool_bitmap_start:");
  put_int((int)kernel_pool.pool_bitmap.bits);
  put_str("   kernel_pool_phy_addr_start");
  put_int((int)kernel_pool.paddr_start);
  put_str("\n");
  put_str("   user_pool_bitmap_start:");
  put_int((int)user_pool.pool_bitmap.bits);
  put_str("   user_pool_phy_addr_start");
  put_int((int)user_pool.paddr_start);
  put_str("\n");

  bitmap_init(&kernel_pool.pool_bitmap);
  bitmap_init(&user_pool.pool_bitmap);

  // 虚拟内存的页数目应该和物理内存一样
  kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_len;
  kernel_vaddr.vaddr_bitmap.bits =
      (void *)(MEM_BITMAP_BASE + kbm_len + ubm_len);

  kernel_vaddr.vaddr_start = K_HEAP_START;

  bitmap_init(&kernel_vaddr.vaddr_bitmap);
  put_str("   mem_pool_init done\n");
}
