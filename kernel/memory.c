#include "memory.h"
#include "debug.h"
#include "print.h"
#include "thread.h"

#define PG_SIZE 4096
#define MEM_BITMAP_BASE 0XC009A000
#define K_HEAP_START 0XC0100000

#define PAGE_DIR_START_ADDR 0X100000
#define PAGE_DIR_CNT 1
#define PAGE_TAB_CNT 255

// 物理内存池
struct pool kernel_phy_pool, user_phy_pool;
// 虚拟内存池
struct pool kernel_vir_pool;

static inline uint32_t btmp_bytes_len4pages(uint32_t pool_size_pg) {
  return (pool_size_pg % 8 == 0) ? (pool_size_pg / 8) : (pool_size_pg / 8 + 1);
}

static void pool_init(struct pool *p, uint32_t btmp_base, uint32_t pool_base,
                      uint32_t pool_size_pg) {
  bitmap_init(&p->btmp, btmp_base, pool_size_pg);
  p->addr_base = pool_base;
  p->pool_size = PG_SIZE * pool_size_pg;
}

static void mem_pool_init(uint32_t all_mem) {
  put_str(" mem_pool_init start\n");
  // 一共是255个页表 + 1个页目录
  uint32_t page_table_size = PG_SIZE * (PAGE_DIR_CNT + PAGE_TAB_CNT);
  uint32_t used_mem = page_table_size + PAGE_DIR_START_ADDR;
  uint32_t free_mem = all_mem - used_mem;
  uint32_t all_free_pages = free_mem / PG_SIZE;
  // 用户和内核各用一半物理内存
  uint32_t kernel_free_pages = all_free_pages / 2;
  uint32_t user_free_pages = all_free_pages - kernel_free_pages;
  // 位图每一位代表一页=4K
  uint32_t kbm_len = btmp_bytes_len4pages(kernel_free_pages);
  uint32_t ubm_len = btmp_bytes_len4pages(user_free_pages);
  uint32_t kp_start = used_mem;
  uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;

  /* 初始化内核和用户物理内存池 */
  pool_init(&kernel_phy_pool, MEM_BITMAP_BASE, kp_start, kernel_free_pages);
  pool_init(&user_phy_pool, MEM_BITMAP_BASE + kbm_len, up_start,
            user_free_pages);
  lock_init(&kernel_phy_pool.lck);
  lock_init(&user_phy_pool.lck);

  put_str(" kernel_pool_bitmap_start: ");
  put_int((int)kernel_phy_pool.btmp.base_addr);
  put_str(" kernel_pool_phy_addr_start: ");
  put_int((int)kernel_phy_pool.addr_base);
  put_str("\n");
  put_str(" user_pool_bitmap_start: ");
  put_int((int)user_phy_pool.btmp.base_addr);
  put_str(" user_pool_phy_addr_start: ");
  put_int((int)user_phy_pool.addr_base);
  put_str("\n");

  /* 初始化内核虚拟内存池 */
  pool_init(&kernel_vir_pool, MEM_BITMAP_BASE + kbm_len + ubm_len, K_HEAP_START,
            kernel_free_pages);
  put_str(" mem_pool_init done\n");
}

void mem_init() {
  put_str("mem_init start\n");
  uint32_t mem_byte_total = (*(uint32_t *)0xb00);
  mem_pool_init(mem_byte_total);
  put_str("mem_init done\n");
}

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

/* 获取可用的虚拟地址页 */
static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
  uint32_t vaddr_start = 0;
  int32_t bit_idx_start = -1;
  uint32_t cnt = 0;
  if (pf == PF_KERNEL) {
    bit_idx_start = bitmap_scan(&kernel_vir_pool.btmp, pg_cnt);
    if (bit_idx_start == -1) {
      return NULL;
    }
    while (cnt < pg_cnt) {
      bitmap_set(&kernel_vir_pool.btmp, bit_idx_start + cnt++);
    }
    vaddr_start = kernel_vir_pool.addr_base + bit_idx_start * PG_SIZE;
  } else {
    /* 用户虚拟内存分配 */
    bit_idx_start = bitmap_scan(&running_thread->user_vaddr_pool.btmp, pg_cnt);
    if (bit_idx_start == -1) {
      return NULL;
    }
    while (cnt < pg_cnt) {
      bitmap_set(&running_thread->user_vaddr_pool.btmp, bit_idx_start + cnt++);
    }
    vaddr_start =
        running_thread->user_vaddr_pool.addr_base + bit_idx_start * PG_SIZE;
  }
  return (void *)vaddr_start;
}

/* 获取本虚拟地址的页表项的虚拟地址 */
static inline uint32_t *pte_vaddr_get(uint32_t vaddr) {
  uint32_t *pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) +
                               PTE_IDX(vaddr) * 4);
  return pte;
}

/* 获取本虚拟地址的页目录项的虚拟地址 */
static inline uint32_t *pde_vaddr_get(uint32_t vaddr) {
  uint32_t *pde = (uint32_t *)(0xfffff000 + PDE_IDX(vaddr) * 4);
  return pde;
}

/* 获取可用的一页物理内存 */
static void *paddr_get(struct pool *mem_pool) {
  int32_t bit_idx = bitmap_scan(&mem_pool->btmp, 1);
  if (bit_idx == -1) {
    return NULL;
  }
  bitmap_set(&mem_pool->btmp, bit_idx);
  uint32_t page_phyaddr = ((bit_idx * PG_SIZE) + mem_pool->addr_base);
  return (void *)page_phyaddr;
}

/* 把虚拟地址映射到物理地址。 原函数： page_table_add */
static void v2p_mapping(void *vaddr, void *paddr) {
  uint32_t vaddr_tmp = (uint32_t)vaddr;
  uint32_t paddr_tmp = (uint32_t)paddr;
  uint32_t *pde = pde_vaddr_get(vaddr_tmp);
  uint32_t *pte = pte_vaddr_get(vaddr_tmp);
  /*
   * 先判断页目录项是否存在,
   * 如果页目录项已存在，那么对应页表肯定存在，只用改页表项就行了。
   */
  if ((*pde) & 0x1) {
    ASSERT(!((*pte) & 0x1));
    if (!((*pte) & 0x1)) {
      *pte = (paddr_tmp | PG_US_U | PG_RW_W | PG_P_1);
    } else {
      PANIC("pte repeat");
      *pte = (paddr_tmp | PG_US_U | PG_RW_W | PG_P_1);
    }
  } else {
    /* 如果页目录项不存在，说明没对应页表，先申请4K物理内存作为页表。*/
    uint32_t pt_paddr = (uint32_t)paddr_get(&kernel_phy_pool);
    /* 把页表物理地址写入页目录, 一旦写入，可以通过pte来访问页表项了。*/
    *pde = pt_paddr | PG_US_U | PG_RW_W | PG_P_1;
    /* 把整张页表全部初始化为0 */
    memset((void *)((uint32_t)pte & 0xfffff000), 0, PG_SIZE);
    ASSERT(!((*pte) & 0x1));
    *pte = paddr_tmp | PG_US_U | PG_RW_W | PG_P_1;
  }
}

void *malloc_page(enum pool_flags pf, uint32_t pg_cnt) {
  /* 保证申请的内存小于15M, 因为物理内存最大32M，内核用户各分一半。*/
  ASSERT(pg_cnt > 0 && pg_cnt < 3840);
  void *vaddr_start = vaddr_get(pf, pg_cnt);
  if (vaddr_start == NULL) {
    return NULL;
  }
  uint32_t vaddr = (uint32_t)vaddr_start;
  uint32_t cnt = pg_cnt;
  struct pool *phy_pool = pf & PF_KERNEL ? &kernel_phy_pool : &user_phy_pool;
  while (cnt-- > 0) {
    void *paddr_start = paddr_get(phy_pool);
    if (paddr_start == NULL) {
      return NULL;
    }
    v2p_mapping((void *)vaddr, paddr_start);
    vaddr += PG_SIZE;
  }
  return vaddr_start;
}

/* 原函数： get_kernel_pages */
void *kernel_pages_malloc(uint32_t pg_cnt) {
  void *vaddr = malloc_page(PF_KERNEL, pg_cnt);
  if (vaddr == NULL) {
    return NULL;
  }
  return vaddr;
}

/* 分配pg_cnt页内存给用户 */
void *user_pages_malloc(uint32_t pg_cnt) {
  lock_acquire(&user_phy_pool.lck);
  void *vaddr = malloc_page(PF_USER, pg_cnt);
  (vaddr, 0, pg_cnt * PG_SIZE);
  lock_release(&user_phy_pool.lck);
  return vaddr;
}

/* 分配指定的一页内存，即指定一页虚拟地址分配, vaddr 需要是4K的倍数 */
void *target_vaddr_malloc(enum pool_flags pf, uint32_t vaddr) {
  struct pool *mem_pool = pf == PF_KERNEL ? &kernel_phy_pool : &user_phy_pool;
  lock_acquire(&mem_pool->lck);
  int32_t bit_idx = -1;
  /* 内核线程的tcb中页目录地址肯定为NULL */
  if (running_thread->pg_dir != NULL && pf == PF_USER) {
    bit_idx = (vaddr - running_thread->user_vaddr_pool.addr_base) / PG_SIZE;
    bitmap_set(&running_thread->user_vaddr_pool.btmp, bit_idx);
  } else if (running_thread->pg_dir == NULL && pf == PF_KERNEL) {
    bit_idx = (vaddr - kernel_vir_pool.addr_base) / PG_SIZE;
    bitmap_set(&kernel_vir_pool.btmp, bit_idx);
  } else {
    PANIC("alloc type error!");
  }
  void *page_phy_addr = paddr_get(mem_pool);
  if (page_phy_addr == NULL) {
    return NULL;
  }
  v2p_mapping(vaddr, page_phy_addr);
  lock_release(&mem_pool->lck);
  return (void *)vaddr;
}

/* 得到一页的虚拟地址映射到的物理地址 */
uint32_t addr_v2p(uint32_t vaddr) {
  uint32_t *pte = pte_vaddr_get(vaddr);
  return ((*pte & 0xfffff000) + (vaddr & 0xfff));
}