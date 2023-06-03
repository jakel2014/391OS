.globl iret_setup, ret_halt


.data
    USER_ESP = 0x83FFFFC         # 128MB + 4MB - 4B
    USER_DS = 0x002B
    USER_CS = 0x0023
    EIP_ADDR = 0x8048018         # Address of EIP is always the same

# iret_setup
#
# Interface: register based arguments
#    Inputs: none
#   Outputs: none
#   Purpose: iret setup for going from execute to user program
iret_setup:
    movl 4(%esp), %edx          # eip in edx
    pushl $USER_DS
    pushl $USER_ESP  
    pushfl
    pushl $USER_CS
    pushl (EIP_ADDR)                  # push eip
    iret

# reg_halt
#
# Interface: register based arguments
#   Inputs: none
#   Outputs: none
#   Purpose: preparing halt for return
ret_halt:                       
    movl 4(%esp), %eax           
    movl 8(%esp), %ebp
    movl 12(%esp), %esp
    leave
    ret 
