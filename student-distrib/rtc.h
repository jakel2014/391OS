#ifndef _RTC_H
#define _RTC_H

#ifndef ASM

#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "system_call.h"

typedef struct rtc {
    volatile int32_t rtc_int_check;
    int32_t rtc_virtual_freq;
    volatile int32_t rtc_virtual_counter;
} rtc_t;

/* Initialize RTC */
void rtc_init(void);

/* Interrupt handler for RTC */
void rtc_handler(void);

int32_t rtc_open(const uint8_t* filename);

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

int32_t rtc_close(int32_t fd);

int32_t rtc_change_freq(int32_t frequency);

char log_base_2(int32_t num);

#endif

#endif /* _RTC_H */

