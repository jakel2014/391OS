#ifndef _INTERRUPTS_LINK_H
#define _INTERRUPTS_LINK_H

#include "idt.h"

#ifndef ASM
    extern void keyboard_handler_link();
    extern void rtc_handler_link();
#endif

#endif
