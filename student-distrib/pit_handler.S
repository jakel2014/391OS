.globl pit_handler

# pit_handler;
#
# Interface: register based arguments
#    Inputs: none
#   Outputs: none
#   Purpose: PIT handler

pit_handler:
    pushl %ebp
    movl %esp, %ebp
    pushal
    pushfl
    call schedule # Calls schedule every PIT interrupt
    popfl
    popal
    leave
    iret
