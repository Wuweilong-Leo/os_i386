#ifndef SYSCALL_H
#define SYSCALL_H
#include "stdint.h"
enum syscall {
  PID_GET = 0,
};
uint32_t pid_get();

#endif