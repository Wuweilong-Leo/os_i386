#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "ioqueue.h"
#include "stdint.h"

#ifndef bool
#define bool uint8_t
#define true 1
#define false 0
#endif

#define KBD_BUF_PORT 0x60

#define ESC '\033'
#define BACKSPACE '\b'
#define TAB '\t'
#define ENTER '\r'
#define DELETE '\177'

#define CHAR_INVISIBLE 0
#define CTRL_LEFT_CHAR CHAR_INVISIBLE
#define CTRL_RIGHT_CHAR CHAR_INVISIBLE
#define SHIFT_LEFT_CHAR CHAR_INVISIBLE
#define SHIFT_RIGHT_CHAR CHAR_INVISIBLE
#define ALT_LEFT_CHAR CHAR_INVISIBLE
#define ALT_RIGHT_CHAR CHAR_INVISIBLE
#define CAPS_LOCK_CHAR CHAR_INVISIBLE

enum ctrl_key_make_code {
  SHIFT_LEFT_MAKE = 0x002a,
  SHIFT_RIGHT_MAKE = 0x0036,
  ALT_LEFT_MAKE = 0x0038,
  ALT_RIGHT_MAKE = 0xe038,
  CTRL_LEFT_MAKE = 0x001d,
  CTRL_RIGHT_MAKE = 0xe01d,
  CAPS_LOCK_MAKE = 0x003a,
};

enum ctrl_key_break_code {
  ALT_RIGHT_BREAK = 0x002a,
  CTRL_RIGHT_BREAK = 0xe09d,
};

/*
 * breakcode 就是 makecode 第8位置1
 */
static inline bool break_code(uint16_t scan_code) {
  return (scan_code & 0x0080) != 0;
}

static inline uint16_t break_code2make_code(uint16_t break_code) {
  return break_code & 0xff7f;
}

void keyboard_init();

#define KEYMAP_MAX_INDEX() ((sizeof(keymap) / (sizeof(char) * 2)) - 1)

extern struct ioqueue kbd_buf;

#endif