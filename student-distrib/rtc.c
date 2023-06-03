/* rtc.c - Functions to interact with the RTC device
 * vim:ts=4 noexpandtab
 */

#include "rtc.h"
#include "lib.h"

#define RTC_REGISTER_PORT   0x70                //Ports 
#define RTC_CMOS_PORT       0x71
#define RTC_INT_NUM         8                   //RTC IRQ num is 8

#define RTC_REG_A           0x0A                //RTC Registers
#define RTC_REG_B           0x0B                //Not using all of them yet
#define RTC_REG_C           0x0C            
#define RTC_REG_D           0x0D
#define DISABLE_NMI         0x80                //IDK if this is needed
#define PIE_MASK            0x40                //Bit to enable periodic interrupt enable

#define MAX_FREQ            1024               
#define MIN_FREQ            2
#define MAX_RATE            15
#define A_RATE_MASK         0x0F                //Masks for setting the base frequency of Register A
#define A_PREV_MASK         0xF0

rtc_t terminal_rtc[MAX_TERMINALS];

/* void rtc_init(void)
 * Inputs: void
 * Return Value: void
 * Function: initializes the RTC by enabling periodic interrupts */
void rtc_init(void) {
    int i;
    for(i = 0; i < MAX_TERMINALS; i++){          // initializing virtual freq
        terminal_rtc[i].rtc_int_check = 0;
        terminal_rtc[i].rtc_virtual_freq = MAX_FREQ/MIN_FREQ;
        terminal_rtc[i].rtc_virtual_counter = MAX_FREQ/MIN_FREQ;
    }

    outb(RTC_REG_B, RTC_REGISTER_PORT);		// select register B
    char prev = inb(RTC_CMOS_PORT);	        // read the current value of register B
    outb(RTC_REG_B, RTC_REGISTER_PORT);		// set the index again (a read will reset the index to register D)
    outb(prev | PIE_MASK, RTC_CMOS_PORT);	// write the previous value ORed with 0x40. This enables perioic interrupts
    enable_irq(RTC_INT_NUM);                // Enable interrupt requests

    rtc_change_freq(MAX_FREQ);              //Set RTC to max frequency
}

/* void rtc_handler(void)
 * Inputs: void
 * Return Value: void
 * Function: handles rtc interrupts */
void rtc_handler(void) {
    outb(RTC_REG_C, RTC_REGISTER_PORT);     //Must read Reg C in order to have another interrupt
    inb(RTC_CMOS_PORT);

    int i;
    for(i = 0; i < MAX_TERMINALS; i++){
        terminal_rtc[i].rtc_virtual_counter--;
        if(terminal_rtc[i].rtc_virtual_counter == 0){           //Virtualization loop, count down until counter is 0, then update interrupt check flag
            terminal_rtc[i].rtc_int_check = 1;
            terminal_rtc[i].rtc_virtual_counter = terminal_rtc[i].rtc_virtual_freq;     //Reset virtualization counter
        }     
    }

    send_eoi(RTC_INT_NUM);                  //RTC interrupt number is 8
}

/* int32_t rtc_open(const uint8_t* filename)
 * Inputs: const uint8_t* filename 
 * Return Value: returns a file descriptor - in this case, always 0
 * Function: Open function for RTC driver, sets virtual frequency to 2Hz */
int32_t rtc_open(const uint8_t* filename){
    int rtc_cur_terminal = get_current_terminal();
    terminal_rtc[rtc_cur_terminal].rtc_virtual_freq = MAX_FREQ/MIN_FREQ;       //Change virtual frequency to 2Hz
    return 0;
}

/* int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd - File descriptor
 *         buf - Buffer containing frequency to be written
 *         nbytes - number of bytes to be written
 * Return Value: 0 on success, -1 on fail
 * Function: Changes the virtual frequency of the RTC */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    int32_t frequency;
    int rtc_cur_terminal = get_current_terminal();

    if(buf == NULL || nbytes != sizeof(int32_t)){
        return -1;
    }

    frequency = *((int32_t*)buf);                   //Convert buf to int32_t

    if(frequency > MAX_FREQ || frequency < MIN_FREQ){
        return -1;
    }

    terminal_rtc[rtc_cur_terminal].rtc_virtual_freq = MAX_FREQ / frequency;        //Virtualization, set virtual frequency to "frequency"                
    return 0;
}

/* int32_t rtc_read(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd - File descriptor
 *         buf - Buffer containing frequency to be written to
 *         nbytes - number of bytes to be read
 * Return Value: 0 on success, -1 on fail
 * Function: Wait until an RTC interrupt has occurred */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    int rtc_cur_terminal = get_current_terminal();
    terminal_rtc[rtc_cur_terminal].rtc_int_check = 0;          //Set interrupt check flag to 0 to force waiting for interrupt
    while(terminal_rtc[rtc_cur_terminal].rtc_int_check == 0); //Loop here and do nothing until the correct number of interrupts have happened
    return 0;
}

/* int32_t rtc_close(int32_t fd)
 * Inputs: fd - the file descriptor
 * Return Value: 0 on success
 * Function: Close function for RTC driver, resets virtual frequency to 2Hz */
int32_t rtc_close(int32_t fd){
    int rtc_cur_terminal = get_current_terminal();
    terminal_rtc[rtc_cur_terminal].rtc_virtual_freq = MAX_FREQ/MIN_FREQ;   //Change virtual frequency to 2Hz
    return 0;
}

/* int32_t rtc_change_freq(int32_t frequency)
 * Inputs: frequency - the frequency to be set
 * Return Value: returns 0 on success, -1 on fail
 * Function: Changes the base frequency of the RTC device, NOT the virtual frequency */
int32_t rtc_change_freq(int32_t frequency){
    if(frequency > MAX_FREQ || frequency < MIN_FREQ || (frequency & (frequency - 1))){    //Make sure desired frequency is in range
        return -1;
    }

    char rate = MAX_RATE - log_base_2(frequency) + 1;   //Calculate rate as used by OSDev

    rate &= A_RATE_MASK;                                //OSDev stuff

    disable_irq(RTC_INT_NUM);                           //Given code from OSDev
    outb(RTC_REG_A, RTC_REGISTER_PORT);                 
    char prev = inb(RTC_CMOS_PORT);                     //Save previous settings
    outb(RTC_REG_A, RTC_REGISTER_PORT);
    outb((prev & A_PREV_MASK) | rate, RTC_CMOS_PORT);
    enable_irq(RTC_INT_NUM);                            
    return 0;
}

/* char log_base_2(int32_t num)
 * Inputs: num - the number to perform the log operation on
 * Return Value: returns ans on success, -1 on fail
 * Function: Calculates the log base 2 of num */
char log_base_2(int32_t num){
    char ans = 0;
    if((num & (num - 1))){  //One line formula to check if a number is a power of 2
        return -1;
    }

    while(num != 1){        //Loop to calculate the log
        num /= 2;
        ans++;
    }

    return ans;
}

