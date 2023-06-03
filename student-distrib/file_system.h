#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "x86_desc.h"
#include "lib.h"
#include "types.h"

#define DATA_BLOCKS 1023
#define DIR_ENTRIES 63
#define MAX_NAME_LENGTH 32
#define MAX_INODES 62
#define BLOCK_SIZE 4096
#define TOTAL_BYTES (DATA_BLOCKS * BLOCK_SIZE)

/*
 * Dentry struct
 * file_name: name of file, file_type: 0-2, inode_number: inode index, reserved: 24Breserved
 */
typedef struct dentry {
    uint8_t file_name[MAX_NAME_LENGTH];
    uint32_t file_type;
    uint32_t inode_number;
    uint8_t reserved[24];   // 24B
} dentry_t;

/*
 * inode struct
 * b_length: length of inode, data_blocks: array of data_blocks
 */
typedef struct inode {
    uint32_t b_length;
    uint32_t data_blocks[DATA_BLOCKS];
} inode_t;

/*
 * Boot block struct
 * dir_count: # of directory entries, inode_count: # of inodes, 
 * data_count: # of data blocks, reserved: 52Breserved, 
 * dir_entries: array of directory entries
 */
typedef struct boot_block {
    uint32_t dir_count;
    uint32_t inode_count;
    uint32_t data_count;
    uint8_t reserved[52];   // 52B
    dentry_t dir_entries[DIR_ENTRIES];
} boot_block_t;

boot_block_t* boot_block;
inode_t* inode_start;
uint8_t* db_start;

extern void file_system_init(uint32_t start);
extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
extern int32_t get_inode_length(uint32_t inode);

#endif






