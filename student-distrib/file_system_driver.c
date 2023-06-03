#include "file_system_driver.h"

int dir_count; // counter to track which directory current process is in

/* int32_t file_open(uint8_t* file_name)
 * Inputs: file_name
 * Return Value: inode number for success, -1 for failure
 * Function: opens file by calling read_dentry_by_name */
int32_t file_open(const uint8_t* file_name){
    dentry_t file_dentry; // temporary dentry to pass into read_dentry_by_name
    if(file_name == NULL){ // null check for parameter
        return -1;
    }
    // call to read_dentry_by_name to fill in file_dentry
    int res = read_dentry_by_name(file_name, &file_dentry); 

    if(res == -1) // means either file is invalid or doesn't exist
        return -1;

    return file_dentry.inode_number; // return inode number to be used in system_call
}

/* int32_t file_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd: file descriptor
 *         buf: buffer
 *         bytes: bytes to write
 * Return Value: -1
 * Function: write does nothing for read-only system */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    // null check for parameters
    if(fd < 0)
        return -1;
    if(buf == NULL)
        return -1;
    if(nbytes < 0)
        return -1;

    return -1;
}

/* int32_t file_read(int32_t fd, int32_t count, uint32_t inode, uint32_t offset)
 * Inputs: fd: file descriptor
 *         count: number of bytes to read
 *         inode: index node
 *         offset: offset from start of file
 * Return Value: number of bytes read
 * Function: reads contents of file using read_data */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    // null check for parameters
    if(fd < 0)
        return -1;
    if(buf == NULL)
        return -1;
    if(nbytes < 0)
        return -1;

    fd_t fd_temp; // temp file descriptor and pcb
    pcb_t* pcb_temp;

    pcb_temp = get_cur_PCB(); // get current pcb

    fd_temp = pcb_temp->file_descriptor[fd]; // get current file descriptor entry

    // read data will fill global variable file_buf
    int bytes_read = read_data(fd_temp.inode, fd_temp.file_position, buf, nbytes);

    return bytes_read;
}

/* int32_t file_close(int32_t fd)
 * Inputs: fd: file descriptor
 * Return Value: 0 if file_dentry is successfully set to 0 
 * Function: closes a specific file descriptor */
int32_t file_close(int32_t fd){
    // null check for parameters
    if(fd < 0) 
        return -1;
    
    if(fd == 0 || fd == 1) // can't close stdin or stdout
        return -1;

    return 0;
}

/* int32_t directory_open(uint8_t* file_name)
 * Inputs: file_name
 * Return Value: inode number for success, -1 for failure
 * Function: opens directory entry by calling read_dentry_by_name (also resets dir_count)*/
int32_t directory_open(const uint8_t* file_name){
    if(file_name == NULL){ // null check
        return -1;
    }

    dentry_t dentry_temp;
    int res = read_dentry_by_name(file_name, &dentry_temp); // fill dentry_temp

    if(res == -1) // file name was invalid
        return -1;

    dir_count = 0; // reset dir_count for next ls

    return dentry_temp.inode_number; // returns the inode number
}

/* int32_t directory_close(int32_t fd)
 * Inputs: fd: file descriptor
 * Return Value: 0 for success, -1 for failure
 * Function: closes directory */
int32_t directory_close(int32_t fd){
    // null check for parameters
    if(fd < 0) 
        return -1;

    dir_count = 0; // reset dir_count (not 100% sure this is needed)

    return 0;
}

/* int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd: file descriptor
 *         buf: buffer
 *         nbytes: bytes to write
 * Return Value: -1
 * Function: write does nothing for read-only system */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
    // null check for parameters
    if(fd < 0)
        return -1;
    if(buf == NULL)
        return -1;
    if(nbytes < 0)
        return -1;

    return -1;
}

/* int32_t directory_read(int32_t fd, int32_t count, uint32_t inode, uint32_t offset)
 * Inputs: fd: file descriptor 
 *         count: number of bytes to read
 *         buf: buffer that is filled with a directory name
 *         nbytes: number of bytes to fill in the buffer
 * Return Value: number of bytes read
 * Function: reads contents of directory entry using read_dentry_by_index */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    // sanity null checks for parameters
     if(fd < 0)
        return -1;
    if(nbytes < 0)
        return -1;

    int j;
    int byte_count = 0; // counter for number of bytes read
    dentry_t dentry_temp;
    int res = read_dentry_by_index(dir_count, &dentry_temp); // fill dentry_temp for current file
    if(res != -1 && dentry_temp.file_name[0] != '\0'){ // check to make sure file exists
        for(j = 0; j < nbytes; j++){ // fill in buf
            if(dentry_temp.file_name[j] == '\0') // breaks when reaches end of name
                break;
            ((uint8_t*)buf)[j] = dentry_temp.file_name[j];
            byte_count++; // keep track of bytes read
        }
    }
    dir_count++;                    // update current location in directory
    return byte_count;

}


