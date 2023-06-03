#define ASM     1

#include "interrupt_linkage.h"
#include "system_call.h"

.globl sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn
.globl sys_call_handler

saved_eax: 
    .long 0

# INTR_LINK(name, func);
#
# Interface: register based arguments
#    Inputs: name: name of linkage function
#            func: name of handler
#   Outputs: Interrupt linkage

#define INTR_LINK(name, func) \
    .global name             ;\
    name:                    ;\
        pushal               ;\
        pushfl               ;\
        call func            ;\
        popfl                ;\
        popal                ;\
        iret                 ;\

INTR_LINK(rtc_handler_link, rtc_handler);
INTR_LINK(keyboard_handler_link, keyboard_input);

# sys_call_handler;
#
# Interface: register based arguments
#    Inputs: none
#   Outputs: none
#   Purpose: system call handler

sys_call_handler:
    sti
    pushal
    pushfl
    cmpl $1, %eax               # if call is less than 0
    jl invalid
    cmpl $10, %eax              # if call is greater than 10
    jg invalid
    pushl %edx
    pushl %ecx
    pushl %ebx
    call *sys_call_table(, %eax, 4)     # Determine which system call
    popl %ebx 
    popl %ecx
    popl %edx
    movl %eax, saved_eax                # save eax before pop all
    popfl
    popal
    movl saved_eax, %eax
    iret

invalid: # invalid system call 
    popfl
    popal
    movl $-1, %eax
    iret

sys_call_table: # system call jump table
        .long 0, sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn

