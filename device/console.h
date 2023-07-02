#ifndef CONSOLE_H
#define CONSOLE_H
void console_init();
void console_acquire();
void console_release();
void console_put_str(char *str);
void console_put_char(char c);
void console_put_int(int num);
#endif