#include "string.h"
#include "debug.h"
#include "global.h"

void memset(void *const dst, uint8_t value, uint32_t size) {
  ASSERT(dst != NULL);
  uint8_t *dst_tmp = (uint8_t *)dst;
  for (uint32_t i = 0; i < size; i++) {
    dst_tmp[i] = value;
  }
}

void memcpy(void *dst, const void *src, uint32_t size) {
  ASSERT(src != NULL && dst != NULL);
  uint8_t *dst_tmp = (uint8_t *)dst;
  uint8_t *src_tmp = (uint8_t *)src;
  for (uint32_t i = 0; i < size; i++) {
    dst_tmp[i] = src_tmp[i];
  }
}

int32_t memcmp(void *s1, void *s2, uint32_t size) {
  ASSERT(s1 != NULL && s2 != NULL);
  const char *s1_tmp = (const char *)s1;
  const char *s2_tmp = (const char *)s2;
  for (uint32_t i = 0; i < size; i++) {
    if (s1_tmp[i] != s2_tmp[i]) {
      return s1_tmp[i] > s2_tmp[i] ? 1 : -1;
    }
  }
  return 0;
}

char *strcpy(char *dst, const char *src) {
  ASSERT(dst != NULL && src != NULL);
  while (*src != 0) {
    *dst = *src;
    dst++;
    src++;
  }
}

uint32_t strlen(const char *str) {
  ASSERT(str != NULL);
  const char *p = str;
  while (*p++)
    ;
  return p - str - 1;
}

int32_t strcmp(const char *s1, const char *s2) {
  ASSERT(s1 != NULL && s2 != NULL);
  while (*s1 != 0 && *s2 == *s1) {
    s1++;
    s2++;
  }
  return (*s1 < *s2) ? -1 : (*s1 > *s2);
}

char *strchr(const char *str, const uint8_t ch) {
  ASSERT(str != NULL);
  while (*str != 0) {
    if (*str == ch) {
      return str;
    }
    str++;
  }
  return NULL;
}

char *strrchr(const char *str, const uint8_t ch) {
  ASSERT(str != NULL);
  const char *last_char = NULL;
  while (*str != 0) {
    if (*str == ch) {
      last_char = str;
    }
    str++;
  }
  return last_char;
}

char *strcat(char *dst, const char *src) {
  ASSERT(dst != NULL && src != NULL);
  char *str = dst;
  while (*str++)
    ;
  --str;
  while (*str++ = *src++)
    ;
  return dst;
}

uint32_t strchrs(const char *str, uint8_t ch) {
  ASSERT(str != NULL);
  uint32_t ch_cnt = 0;
  const char *p = str;
  while (*p != 0) {
    if (*p == ch) {
      ch_cnt++;
    }
    p++;
  }
  return ch_cnt;
}
