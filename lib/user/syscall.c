#include "syscall.h"
#include "stdint.h"

/* 无参数系统调用 */
static inline uint32_t syscall0(uint32_t num) {
  uint32_t ret;
  asm volatile("int $0x80" : "=a"(ret) : "a"(num) : "memory");
  return ret;
}

/* 单参数系统调用*/
static inline void syscall1(uint32_t num, uint32_t arg1) {
  uint32_t ret;
  asm volatile("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1) : "memory");
  (void)ret;
}

/* 双参数系统调用*/
static inline void syscall2(uint32_t num, uint32_t arg1, uint32_t arg2) {
  uint32_t ret;
  asm volatile("int $0x80"
               : "=a"(ret)
               : "a"(num), "b"(arg1), "c"(arg2)
               : "memory");
  (void)ret;
}

/* 三参数系统调用*/
static inline void syscall3(uint32_t num, uint32_t arg1, uint32_t arg2,
                            uint32_t arg3) {
  uint32_t ret;
  asm volatile("int $0x80"
               : "=a"(ret)
               : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
               : "memory");
  (void)ret;
}

uint32_t pid_get() { return syscall0(PID_GET); }