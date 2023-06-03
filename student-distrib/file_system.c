#include "file_system.h"

/*
 * file_system_init
 *   DESCRIPTION: Initializes file_system
 *   INPUTS: uint32_t start - starting address of boot block
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets boot block address, start of inodes address, and start of data blocks address
 */
void file_system_init(uint32_t start){
    boot_block = (boot_block_t*) start;
    inode_start = (inode_t*)(boot_block+1);
    db_start = (uint8_t*)(inode_start + boot_block->inode_count);
}

/*
 * read_dentry_by_name
 *   DESCRIPTION: Looks through directory entries and finds by name
 *   INPUTS: fname - name of file, dentry - empty dentry 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Puts found dentry into the passed in dentry
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    if(dentry == NULL){ // null checks
        return -1;
    }
    if(fname == NULL){
        return -1;
    }

    int i;
    uint32_t index;
    uint32_t len = strlen((int8_t*)fname); // getting length of inputted fname

    // returns -1 for invalid name
    if(len > MAX_NAME_LENGTH || len == 0){
        return -1;
    }

    // iterate across the file directory
    for(i = 0; i < DIR_ENTRIES; i++){
        if(strlen((int8_t*)boot_block->dir_entries[i].file_name) > len){
            len = strlen((int8_t*)boot_block->dir_entries[i].file_name); // update len if file name in directory is longer
        }   
        if(len > MAX_NAME_LENGTH){ // if len is greater than 32, set it to 32
            len = MAX_NAME_LENGTH;
        }
        
        // if current location in directory matches fname, set index and break
        if(strncmp((int8_t*)boot_block->dir_entries[i].file_name, (int8_t*)fname, len) == 0){
            index = i;
            *dentry = boot_block->dir_entries[i];
            break;
        }else if(i == DIR_ENTRIES - 1){
            return -1;
        }
    }

    return 0;
    
}

/*
 * read_dentry_by_index
 *   DESCRIPTION: Looks through directory entries and finds by index
 *   INPUTS: index - name of file, dentry - empty dentry 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Puts found dentry into the passed in dentry
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    // returns -1 for invalid index 
    if(index < 0 || index > DIR_ENTRIES){
        return -1;
    }

    if(dentry == NULL){
        return -1;
    }

    // set file_name, file_type, and inode_number for dentry
    strncpy((int8_t*)dentry->file_name, (int8_t*)boot_block->dir_entries[index].file_name, MAX_NAME_LENGTH); 
    dentry->file_type = boot_block->dir_entries[index].file_type;
    dentry->inode_number = boot_block->dir_entries[index].inode_number;

    return 0;

}

/*
 * get_inode_length(uint32_t inode)
 *   DESCRIPTION: Getter for getting b_length within a certain inode block
 *   INPUTS: inode: inode index
 *   OUTPUTS: b_length for a certain inode block
 *   RETURN VALUE: b_length
 *   SIDE EFFECTS: none
 */
int32_t get_inode_length(uint32_t inode){
    return inode_start[inode].b_length;
}

/*The last routine works much like the read system call, reading up to
length bytes starting from position offset in the file with inode number inode and returning the number of bytes
read and placed in the buffer. A return value of 0 thus indicates that the end of the file has been reached.*/

/*
 * read_data
 *   DESCRIPTION: Copy contents of the file/inode to buf
 *   INPUTS: inode - index of inode, offset - offset in the file to start reading from, length - # of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: buf is filled with contents of the certain file and certain position and length within the file
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    int inode_length, nth, byte;
    
    int32_t bytes_read = 0;
    
    // Calculate which block and where in block based on offset
    nth = offset / BLOCK_SIZE;
    byte = offset % BLOCK_SIZE;

    if(inode < 0 || inode > MAX_INODES - 1){ 
        return 0;
    }

    inode_t* cur_inode = &inode_start[inode];
    inode_length = cur_inode->b_length;

    // If offset is out of bounds
    if(offset > inode_length) 
        return 0;

    // First data block
    uint8_t* current_DB = &(db_start[(cur_inode->data_blocks[nth])*(BLOCK_SIZE)]);

    if(length > inode_length) 
        length = inode_length;

    if(inode_length - offset < length){
        length = inode_length - offset;
    }

    while (bytes_read < length) {
        if (byte >= BLOCK_SIZE) {
            byte = 0;
            nth++;
            // update data block
            current_DB = &(db_start[(cur_inode->data_blocks[nth])*(BLOCK_SIZE)]);
        } else {
            memcpy(buf, current_DB + byte, 1);
            buf++;
            byte++;
            bytes_read++;
        }
    }

    return bytes_read;
    
}
