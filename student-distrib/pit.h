#ifndef _PIT_H
#define _PIT_H

#ifndef ASM

#include "types.h"
#include "lib.h"
#include "i8259.h"

//Scheduling uses pit, not rtc
#define PIT_CMD_PORT        0x36
#define PIT_DATA_PORT       0x40
#define PIT_CMD_REG         0x43
#define PIT_IRQ_NUM         0
#define MAX_HZ 1193182

/* Initialize PIC */
void pit_init(void);

/* calls scheduler */
void pit_handler(void);

#endif

#endif /* _PIT_H */
