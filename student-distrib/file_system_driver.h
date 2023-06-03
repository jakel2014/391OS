#ifndef _FILE_SYSTEM_DRIVER_
#define _FILE_SYSTEM_DRIVER_

#ifndef ASM

#include "x86_desc.h"
#include "lib.h"
#include "types.h"
#include "file_system.h"
#include "system_call.h"

extern int32_t file_open(const uint8_t* file_name);
extern int32_t file_close(int32_t fd);
extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

extern int32_t directory_open(const uint8_t* file_name);
extern int32_t directory_close(int32_t fd);
extern int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);

#endif
#endif
