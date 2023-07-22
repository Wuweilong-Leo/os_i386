#ifndef TSS_H
#define TSS_H
#include "thread.h"
void tss_esp0_update(tcb *thread);
void tss_init();
#endif