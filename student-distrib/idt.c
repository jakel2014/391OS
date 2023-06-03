#include "idt.h"

/*
 * initialize_idt()
 *   DESCRIPTION: Initializes IDT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Fills in idt table and sets entrys for exceptions, interrupts, and system calls
 */
void initialize_idt(){
    int i;
    // setting all IDT entries in IDT table
    for(i = 0; i < NUM_VEC; i++){ 
        idt[i].seg_selector = KERNEL_CS;
        idt[i].present = 1;
        idt[i].dpl = 0;
        idt[i].reserved0 = 0;
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;
        idt[i].reserved4 = 0;

        if (i == SYSTEM_CALL_IDT) {idt[i].dpl = 3;} // dpl value of 3 is user level
    }

    // setting Interrupt IDT entries (0x20 - 0x30)
    for(i = 0x20; i < 0x30; i++){
        idt[i].seg_selector = KERNEL_CS;
        idt[i].present = 1;
        idt[i].dpl = 0;
        idt[i].reserved0 = 0;
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 1;
        idt[i].reserved4 = 0;
    }

    // Setting run time parameters for exceptions in IDT table
    SET_IDT_ENTRY(idt[0], divide_by_zero);
    SET_IDT_ENTRY(idt[1], debug);
    SET_IDT_ENTRY(idt[2], NMI_Interrupt);
    SET_IDT_ENTRY(idt[3], Breakpoint);
    SET_IDT_ENTRY(idt[4], Overflow);
    SET_IDT_ENTRY(idt[5], Bounds_range_exceeded);
    SET_IDT_ENTRY(idt[6], Invalid_opcode);
    SET_IDT_ENTRY(idt[7], Device_not_avaliable);
    SET_IDT_ENTRY(idt[8], Double_fault);
    SET_IDT_ENTRY(idt[9], Coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[10], Invalid_TSS);
    SET_IDT_ENTRY(idt[11], Segment_not_present);
    SET_IDT_ENTRY(idt[12], Stack_segment_present);
    SET_IDT_ENTRY(idt[13], General_protection_fault);
    SET_IDT_ENTRY(idt[14], Page_fault);

    SET_IDT_ENTRY(idt[16], x87_FPU_error);
    SET_IDT_ENTRY(idt[17], Alignment_check);
    SET_IDT_ENTRY(idt[18], Machine_check);
    SET_IDT_ENTRY(idt[19], SIMD_Floating_Point_Exception);
    SET_IDT_ENTRY(idt[20], Virtualization_Exception);
    SET_IDT_ENTRY(idt[21], Control_Protection_Exception);
   
    SET_IDT_ENTRY(idt[28], Hypervisor_Injection_Exception);
    SET_IDT_ENTRY(idt[29], VMM_Communication_Exception);
    SET_IDT_ENTRY(idt[30], Security_Exception);

    
    // Setting run time parameters for interrupts in IDT table
    SET_IDT_ENTRY(idt[RTC_IDT], rtc_handler_link);
    SET_IDT_ENTRY(idt[KEYBOARD_IDT], keyboard_handler_link);
    SET_IDT_ENTRY(idt[PIT_IDT], pit_handler);


    // Setting run time parameters for system calls in IDT table
    SET_IDT_ENTRY(idt[SYSTEM_CALL_IDT], sys_call_handler);
}

