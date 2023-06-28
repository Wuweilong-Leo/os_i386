#ifndef _PRINT_H
#define _PRINT_H
#include "stdint.h"
void set_cursor(uint16_t target);
uint16_t get_cursor();
void put_char(uint8_t char_asci);
void put_str(char *message);
void put_int(uint32_t num);
#endif