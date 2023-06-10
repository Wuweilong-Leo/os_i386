#ifndef DEBUG_H
#define DEBUG_H
#include "interrupt.h"
#include "print.h"
void panic_spin(char *filename, int line, const char *func,
                const char *condition);
#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)
#ifdef NDEBUG
#define ASSERT(condition) ((void)0)
#else
#define ASSERT(condition)                                                      \
  do {                                                                         \
    if (condition) {                                                           \
      ;                                                                        \
    } else {                                                                   \
      PANIC(#condition);                                                       \
    }                                                                          \
  } while (0);
#endif

#endif