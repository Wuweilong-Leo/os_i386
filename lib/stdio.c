#include "stdio.h"
#define VA_START(ap, v) ap = (va_list)&v
#define VA_ARG(ap, t) (*(((t *)(ap))++))
#define VA_END(ap) ap = NULL

/* 外部保证str足够大 */
void itoa(int num, char *str, uint8_t base) {
  if (num == 0) {
    str[0] = '0';
    str[1] = 0;
    return;
  }
  bool is_neg = false;
  if (num < 0 && base == 10) {
    is_neg = true;
    num = -num;
  }
  int i = 0;
  while (num != 0) {
    int rem = num % base;
    str[i++] = (rem > 9) ? (rem - 10 + 'a') : (rem + '0');
    num /= base;
  }

  if (is_neg) {
    str[i++] = '-';
  }

  str[i] = 0;
  int j = 0;
  while (j < i / 2) {
    char tmp = str[j];
    str[j] = str[i - j - 1];
    str[i - j - 1] = tmp;
    j++;
  }
}
