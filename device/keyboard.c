#include "keyboard.h"
#include "global.h"
#include "interrupt.h"
#include "io.h"
#include "ioqueue.h"
#include "print.h"

struct ioqueue kbd_buf;

static char keymap[][2] = {
    {0, 0},
    {ESC, ESC},
    {'1', '!'},
    {'2', '@'},
    {'3', '#'},
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {BACKSPACE, BACKSPACE},
    {TAB, TAB},
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {ENTER, ENTER},
    {CTRL_LEFT_CHAR, CTRL_LEFT_CHAR},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'},
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '"'},
    {'`', '~'},
    {SHIFT_LEFT_CHAR, SHIFT_LEFT_CHAR},
    {'\\', '|'},
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {SHIFT_RIGHT_CHAR, SHIFT_RIGHT_CHAR},
    {'*', '*'},
    {ALT_LEFT_CHAR, ALT_LEFT_CHAR},
    {' ', ' '},
    {CAPS_LOCK_CHAR, CAPS_LOCK_CHAR},
};

static bool ctrl_status = false, shift_status = false, alt_status = false,
            caps_lock_status = false;

static uint16_t scan_code_get() {
  // 记录是不是有额外的扫描码
  static bool ext_scan_code = false;
  uint8_t scan_code = inb(KBD_BUF_PORT);
  if (ext_scan_code) {
    ext_scan_code = false;
    return (0xe000 | (uint16_t)scan_code);
  }
  if (scan_code == 0xe0) {
    ext_scan_code = true;
  }
  return (uint16_t)scan_code;
}

// 如果scan_code对应的不是控制字符，返回false。
static bool ctrl_key_proc(uint16_t scan_code) {
  bool status_to_set = true;
  if (break_code(scan_code)) {
    status_to_set = false;
    // 如果是断码，要先转化为通码方便统一处理。
    scan_code = break_code2make_code(scan_code);
  }

  switch (scan_code) {
  case CTRL_LEFT_MAKE:
    ctrl_status = status_to_set;
    return true;
  case CTRL_RIGHT_MAKE:
    ctrl_status = status_to_set;
    return true;
  case ALT_LEFT_MAKE:
    alt_status = status_to_set;
    return true;
  case ALT_RIGHT_MAKE:
    alt_status = status_to_set;
    return true;
  case SHIFT_LEFT_MAKE:
    shift_status = status_to_set;
    return true;
  case SHIFT_RIGHT_MAKE:
    shift_status = status_to_set;
    return true;
  case CAPS_LOCK_MAKE:
    // 只有是caps_lock 弹起，才能翻转caps状态。
    if (status_to_set == false) {
      caps_lock_status = !caps_lock_status;
    }
    return true;
  default:
    break;
  }
  return false;
}

static bool normal_key_proc(uint16_t scan_code) {
  // 如果是一般的字符，断码不作处理。
  if (break_code(scan_code)) {
    return true;
  }
  bool shift = false;
  if (shift_status || caps_lock_status) {
    shift = true;
  }
  uint8_t index = scan_code & 0xff;
  if (index > KEYMAP_MAX_INDEX()) {
    return false;
  }
  ioq_putchar(&kbd_buf, keymap[index][shift]);
  return true;
}

static void intr_keyboard_handler() {
  bool ret = true;
  uint16_t scan_code = scan_code_get();
  // 如果有额外的扫描码，直接返回接受下一个扫描码。
  if (scan_code == 0x00e0) {
    return;
  }
  ret = ctrl_key_proc(scan_code);
  if (ret) {
    return;
  }
  ret = normal_key_proc(scan_code);
  if (ret) {
    return;
  }
  put_str("unknown key\n");
}

void keyboard_init() {
  put_str("keyboard init start\n");
  ioqueue_init(&kbd_buf);
  intr_handler_register(INT_KEYBOARD, intr_keyboard_handler);
  put_str("keyboard init done\n");
}