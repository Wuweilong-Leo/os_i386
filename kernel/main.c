#include "debug.h"
#include "init.h"
#include "interrupt.h"
#include "print.h"
int main() {
  put_str("I AM KERNEL\n");
  init_all();
  ASSERT(1 == 2);
  while (1)
    ;
}