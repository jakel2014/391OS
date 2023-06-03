.text

.globl loadPagingDirectory, enablePaging
.globl paging_table, vid_paging_table, paging_directory

#define ENTRIES 1024
#define STRUCT_BYTES 4096

# void loadPagingDirectory(unsigned int*);
#
# Interface: C-style stack based argument
#    Inputs: unsigned int* paging_directory
#   Outputs: Puts paging_directory's address into cr3
#            to tell processor where to find it.
# Registers: Alters eax and cr3
.align 4
loadPagingDirectory:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%esp), %eax
    movl %eax, %cr3
    movl %ebp, %esp
    popl %ebp
    ret

# void enablePaging();
#
# Interface: Register based arguments (not C-style)
#    Inputs: None
#   Outputs: Set the 32th bit in CR0. Also, enable PSE (4MiB pages) with CR4
# Registers: Alters eax, cr4, and cr0
enablePaging:
    movl %cr4, %eax
    orl $0x00000010,  %eax                  # Set 5th bit
    movl %eax, %cr4

    movl %cr0, %eax
    orl $0x80000000, %eax                   # Set 32th bit
    movl %eax, %cr0
    ret

# Padding by alignment for paging_table, vid_paging_table, and paging_directory
.align STRUCT_BYTES 
paging_table:
    .rept ENTRIES
        .long 0
    .endr
vid_paging_table:
     .rept ENTRIES
         .long 0
     .endr
paging_directory:
    .rept ENTRIES
        .long 0
    .endr

    

