/* terminal_driver.h - Defines used in interactions with the keyboard interrupts
 * vim:ts=4 noexpandtab
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "system_call.h"

void terminal_init(void);
int terminal_read(int32_t fd, void* buf, int32_t nbytes);
int terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int terminal_open(const uint8_t* filename);
int terminal_close(int32_t fd);

#endif /* _TERMINAL_H */
