#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "lib.h"

#define ENTRIES 1024 // Total number of entries in paging table/directory
#define VID_START 184 // Start of video memory in paging table


// Paging Directory ... Least sign to most ... 4MB and 4KB
typedef struct paging_dir {
    union {
        uint32_t val;
        struct {
            uint32_t P: 1;
            uint32_t RW: 1;
            uint32_t US: 1;
            uint32_t PWT: 1;
            uint32_t PCD: 1;
            uint32_t A: 1;
            uint32_t D: 1;
            uint32_t PS: 1;
            uint32_t G: 1;
            uint32_t AVL: 3;
            uint32_t PAT: 1;
            uint32_t index_39_32: 8;
            uint32_t RSVD: 1;
            uint32_t index_31_22: 10;
        } __attribute__ ((packed));
        struct {
            uint32_t P: 1;
            uint32_t RW: 1;
            uint32_t US: 1;
            uint32_t PWT: 1;
            uint32_t PCD: 1;
            uint32_t A: 1;
            uint32_t avl: 1;
            uint32_t PS: 1;
            uint32_t G: 1;
            uint32_t AVL: 3;
            uint32_t index_31_12: 20;
        } __attribute__ ((packed));
    };
} page_dir_t;

// array for paging directory (4096 bytes total) 
page_dir_t paging_directory[ENTRIES] __attribute__((aligned(4096)));

// Page Table
typedef union paging_table {
    uint32_t val;
    struct {
        uint32_t P: 1;
        uint32_t RW: 1;
        uint32_t US: 1;
        uint32_t PWT: 1;
        uint32_t PCD: 1;
        uint32_t A: 1;
        uint32_t D: 1;
        uint32_t PAT: 1;
        uint32_t G: 1;
        uint32_t AVL: 3;
        uint32_t index_31_12: 20;
    } __attribute__ ((packed));
} paging_table_t;

// array for paging table (4096 bytes total)
paging_table_t paging_table[ENTRIES] __attribute__((aligned(4096)));
paging_table_t vid_paging_table[ENTRIES] __attribute__((aligned(4096)));


extern void paging_init();

extern void loadPagingDirectory(unsigned int*);
extern void enablePaging();

#endif


