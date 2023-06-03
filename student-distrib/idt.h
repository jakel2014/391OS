#ifndef _IDT_H
#define _IDT_H

#ifndef ASM

#include "x86_desc.h"
#include "keyboard.h"
#include "rtc.h"
#include "interrupt_linkage.h"
#include "system_call.h"
#include "pit.h"


#define RTC_IDT         0x28 // secondary pic
#define KEYBOARD_IDT    0x21 // PRIMARY PIC
#define SYSTEM_CALL_IDT 0x80 // System Call Handler
#define EXCEPTION 50
#define PIT_IDT         0x20 // Pit

extern void initialize_idt();

extern void debug();

extern void divide_by_zero();

extern void NMI_Interrupt();

extern void Breakpoint();

extern void Overflow();

extern void Bounds_range_exceeded();

extern void Invalid_opcode();

extern void Device_not_avaliable();

extern void Double_fault();

extern void Coprocessor_segment_overrun();

extern void Invalid_TSS();

extern void Segment_not_present();

extern void Stack_segment_present();

extern void General_protection_fault();

extern void Page_fault();

extern void x87_FPU_error();

extern void Alignment_check();

extern void Machine_check();

extern void SIMD_Floating_Point_Exception();

extern void Virtualization_Exception();

extern void Control_Protection_Exception();

extern void Hypervisor_Injection_Exception();

extern void VMM_Communication_Exception();

extern void Security_Exception();


#endif
#endif
