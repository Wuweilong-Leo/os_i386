#ifndef STRING_H
#define STRING_H
#include "stdint.h"
void memset(void *const dst, uint8_t value, uint32_t size);
void memcpy(void *dst, const void *src, uint32_t size);
int32_t memcmp(void *s1, void *s2, uint32_t size);
char *strcpy(char *dst, const char *src);
uint32_t strlen(const char *str);
int32_t strcmp(const char *s1, const char *s2);
char *strchr(const char *str, const uint8_t ch);
char *strrchr(const char *str, const uint8_t ch);
char *strcat(char *dst, const char *src);
uint32_t strchrs(const char *str, uint8_t ch);
#endif