/* terminal_driver.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

#define OS_SIZE 6 // size of "391OS>"
#define LENGTH_COUNTER 7

int start[MAX_TERMINALS]; // global variables for start and end
int end[MAX_TERMINALS];

/* function     : terminal_init
 * input        : nothing
 * output       : nothing
 * Description  : Clears screen and initializes terminal global variables
 * return       : nothing
 */
void terminal_init(void){
    clear(); // clear screen and initializes start/end to 0
    int i;
    for(i = 0; i < MAX_TERMINALS; i++){
        start[i] = 0;
        end[i] = 0;
    }
  
}

/* function     : terminal_read
 * input        : fd, buf, nbytes
 * output       : passed in buffer is filled
 * Description  : Reads keyboard buffer and fills passed in buffer
 * return       : number of bytes read
 */
int terminal_read(int32_t fd, void* buf, int32_t nbytes){
    int i;
    int t_cur_terminal = get_current_terminal();
    int endflag = 0; // indicates end of buffer
    if (fd != 0) { return -1;} // null checks for parameter
    if (buf == NULL) { return -1; }

    pcb_t* pcb_to_clear = (pcb_t*)get_PCB_addr_terminal(t_cur_terminal); 
    if (strncmp((int8_t*)pcb_to_clear->file, (int8_t*)"counter", LENGTH_COUNTER) == 0){ // 7 is length of counter string
        counter_flag = 0;
    }

    while(1){
        for (i = 0; i < keyIndex[t_cur_terminal]; i++) { 
            if (keyIndex[t_cur_terminal] < KEY_BUFF_SIZE && keyboard_buffer[t_cur_terminal][i] != '\n') // checking if enter key was pressed
            {
                ((char*)buf)[i] = keyboard_buffer[t_cur_terminal][i];
            }
            if (keyboard_buffer[t_cur_terminal][i] == '\n') {
                end[t_cur_terminal] = i;
                endflag = 1;
                break;
            }
        }
        
        if (endflag) {break;} 
    }
    int ret = end[t_cur_terminal]; // set ret and start after loop
    start[t_cur_terminal] += ret + 1;
    counter_flag = 1;
    return ret;

}

/* function     : terminal_write
 * input        : fd, buf, nbytes
 * output       : buffer is echoed in terminal
 * Description  : Takes in buffer and writes it to the screen
 * return       : number of bytes written
 */
int terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    int i;
    int color[6] = {12, 14, 10, 9, 13, 15}; // color indexes
    if (fd != 1) { return -1;} // null checks for parameters
    if (buf == NULL) { return -1; }

    pcb_t* pcb_to_clear = (pcb_t*)get_PCB_addr_terminal(get_current_terminal()); 
    if (strncmp((int8_t*)pcb_to_clear->file, (int8_t*)"counter", LENGTH_COUNTER) == 0){ // 7 is length of counter string
        counter_flag = 1;
    }

    char OS_BUF[OS_SIZE] = "391OS>"; // 391OS> buf

    for(i = 0; i < OS_SIZE; i++){ // checking if buf is equal to '391OS>'
        if(((char*)buf)[i] != OS_BUF[i]){
            break;
        }
    }

    if (get_terminal_screen_x(get_current_terminal()) != 0 && i == 6){ // partial fix to spamming bug
        putc(ENTER);
    }

    if(i == 6) // if buf is equal to '391OS>' print a new line
        color_flag[get_current_terminal()] = 1;
        
    
    for (i = 0; i < nbytes; i++){ // write buf to screen
        if (color_flag[get_current_terminal()]) {change_color(color[i]);}
        putc((uint8_t) ((char*)buf)[i]);
    }

    if (color_flag[get_current_terminal()]) {
        change_color(get_default_color());
        color_flag[get_current_terminal()] = 0;
    }

    resetBuff(get_current_terminal()); // reset the keyboard buffer
    return nbytes;
}

/* function     : terminal_open
 * input        : filename
 * output       : 0
 * Description  : Opens terminal (does nothing for checkpoint 2)
 * return       : 0
 */
int terminal_open(const uint8_t* filename){
    if(filename == NULL)
        return -1;
    return 0;
}

/* function     : terminal_close
 * input        : fd
 * output       : 0
 * Description  : Closes terminal and resets keyboard buffer
 * return       : 0
 */
int terminal_close(int32_t fd){
    if(fd < 0)
        return -1;
    resetBuff(get_current_terminal()); // resets keyboard buffer
    return 0;
}

