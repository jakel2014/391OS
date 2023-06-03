/* keyboard.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

/* Flags for the special character and indexes to tabing and ctrl*/
int capsChar;
int shiftChar;
int ctrlFlag;
int altFlag;
int ctrlIndex;
unsigned char keyboard_buffer[MAX_TERMINALS][KEY_BUFF_SIZE]; // expanded keyboard_buffer and keyIndex for 3 terminals
int keyIndex[MAX_TERMINALS];
int enterPress;
int shiftCount;
int tabIndex; 
int capsSpecialFlag;
int specialFlag;
//int counter_flag;


/* This scanLetter Matrix is used for shift and capslocks. We itterate though it each keyboard char press to set the capsSpecialFlag
 * to 1 if it is a special character and 0 if it is a letter. The capsSpecialFlag is used for the shift and caps logic
 */
unsigned char scanLetter[LETTER_NUM][2] = { 
        {'q', 'Q'},
        {'w', 'W'},
        {'e', 'E'},
        {'r', 'R'}, 
        {'t', 'T'},
        {'y', 'Y'},
        {'u', 'U'},
        {'i', 'I'},
        {'o', 'O'},
        {'p', 'P'}, 
        {'a', 'A'},
        {'s', 'S'},
        {'d', 'D'},
        {'f', 'F'},
        {'g', 'G'},
        {'h', 'H'},
        {'j', 'J'},
        {'k', 'K'},
        {'l', 'L'},  
        {'z', 'Z'},
        {'x', 'X'},
        {'c', 'C'},   
        {'v', 'V'},
        {'b', 'B'},
        {'n', 'N'},  
        {'m', 'M'},
};



/* 
 * The scanCode 2D array is filled, on the left, with the ASCII character that corresponds to its index. 
 * The index value is equivalent to the universal scancode 1 keyboard. For Checkpoint 1, to get the 
 * lowercase values and numbers, only the scanCode[ASCII Index][0] is in use, the [][1] index is unused. 
 */

unsigned char scanCode[SCANCODE_SIZE][2] = {
        {'0', '0'},
        {ESC, ESC},
        {'1', '!'},
        {'2', '@'},
        {'3', '#'},
        {'4', '$'},
        {'5', '%'},
        {'6', '^'},
        {'7', '&'},
        {'8', '*'},
        {'9', '('},
        {'0', ')'},
        {'-', '_'},
        {'=', '+'},
        {BACKSPACE, BACKSPACE},
        {TAB, TAB}, 
        {'q', 'Q'},
        {'w', 'W'},
        {'e', 'E'},
        {'r', 'R'}, 
        {'t', 'T'},
        {'y', 'Y'},
        {'u', 'U'},
        {'i', 'I'},
        {'o', 'O'},
        {'p', 'P'},      
        {'[', '{'}, 
        {']', '}'},
        {ENTER, ENTER},
        {CTRL, CTRL},
        {'a', 'A'},
        {'s', 'S'},
        {'d', 'D'},
        {'f', 'F'},
        {'g', 'G'},
        {'h', 'H'},
        {'j', 'J'},
        {'k', 'K'},
        {'l', 'L'},
        {';', ':'},
        {0x27, '"'},  // 0x27 is the hex code value for ' since it breaks the char
        {'`', '~'},
        {LSHIFT, LSHIFT},
        {92, '|'},   // 92 is the ascii code value for \ since it breaks the char
        {'z', 'Z'},
        {'x', 'X'},
        {'c', 'C'},   
        {'v', 'V'},
        {'b', 'B'},
        {'n', 'N'},  
        {'m', 'M'},
        {',', '<'},
        {'.', '>'},   
        {'/', '?'},
        {RSHIFT, RSHIFT},
        {NULL, NULL},  
        {L_ALT, L_ALT},
        {' ', ' '},  
        {CAPSL, CAPSL},
};

/* function     : keyboard_init
 * input        : nothing
 * output       : nothing
 * Description  : This function enables the use of the keyboard on the i8259 pic. Allows for keyboard interrupts.
 * return       : nothing
 */
void keyboard_init(void){
    capsChar = 0;                                           // Initializing all the flags and index to 0 at start of interrupts
    shiftChar = 0;
    tabIndex = 0;
    capsSpecialFlag = 0;
    specialFlag = 0;
    ctrlFlag = 0;
    altFlag = 0;
    ctrlIndex = 0;
    enterPress = 0;

    int i; // Initialize keyIndex and keyboard_buffer for every terminal
    for(i = 0; i < MAX_TERMINALS; i++){
        keyIndex[i] = 0;
        keyboard_buffer[i][KEY_BUFF_SIZE] = ENTER;
    }

    enable_irq(KEYBOARD_IRQ);                               // enables an interrupt to be read on the PIC at the keyboard IRQ 
}

/* function     : keyboard_input
 * input        : nothing
 * output       : ASCII character to the screen.
 * Description  : This function outputs the corresponding ASCII char onto the screen that was typed by the user on the keyboard.
 *                It reads from the keyboard data port, searches for the corresponding char in the scancode dictionary, then 
 *                prints it to the screen.
 * return       : nothing
 */
/* Handle Keyboard Inputs */
void keyboard_input(void){  
    cli();
    int i;
    int f_flag; 

    uint8_t key_pressed = inb(KEYBOARD_DATA_PORT) & 0xFF;   // the data port with 1111 1111 to keep the last 8 bit value. 

    // get the number of terminal that is currently being displayed
    int k_cur_terminal = get_displayed_terminal();

    if (key_pressed == NULL){                                                                   // exists if invalid key press
        return;
    } else if (key_pressed == SHIFT_RIGHT_RELEASE || key_pressed == SHIFT_LEFT_RELEASE){        // changes shift flags upon release of shift key
        shiftChar = 0;
    } else if (key_pressed == CTRL_RELEASE){                                                    // changes ctrl flag upon ctrl key release
        ctrlFlag = 0;
    } else if (key_pressed == ALT_RELEASE){
        altFlag = 0;
    }

    int terminal_write_flag = get_keyboard_interrupt(k_cur_terminal);

    if ( terminal_write_flag == 0){                                           // this checks if the terminal is allowed to be written to on the process running.
        if (keyIndex[k_cur_terminal] <= KEY_BUFF_SIZE) {                                        // checks that the index is within buffer size
            if (key_pressed <= F3) {                                                              // check valid key in scancode
                if (key_pressed == ENTER_PRESS) {                                                          // sets enter flag, writes enter to buffer
                    enterPress = 1;
                    keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal]] = '\n';
                    putc_keyboard('\n');                                                                     // calls putc for enter 
                    keyIndex[k_cur_terminal]++;
                } else if (key_pressed == BACKSPACE && keyIndex[k_cur_terminal] > 0) {                                              // calls backspace, function , reset the key index
                    if (keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal] - 1] == TAB) {                                                     // if tab was right before backspace, it backspaces on the tab
                        for (i = 0; i < TAB_SIZE; i++) {
                            backspace(); 
                        }
                    } else {                                                                        // regular backspace
                        backspace(); 
                    }
                    keyIndex[k_cur_terminal]--;
                } else if (keyIndex[k_cur_terminal] < KEY_BUFF_SIZE) {
                    if (key_pressed == L_ALT || key_pressed == R_ALT){                                                     // sets alt flag
                        altFlag = 1;
                    } else if (key_pressed == LSHIFT || key_pressed == RSHIFT) {                        //sets shift flag if shift is pressed
                        shiftChar = 1;
                    } else if (key_pressed == CAPSL) {                                                  //toggles caps flag on and off 
                        if (capsChar == 1) { capsChar = 0; } else { capsChar = 1; }
                    } else if (key_pressed == ESC) {                                                    //esc doesn't do anything, sets alt flag
                        altFlag = 1;
                    } else if (key_pressed == TAB) {                                                    // calls tab in putc, keys track of the index in buffer
                        tabIndex = keyIndex[k_cur_terminal];
                        keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal]] = TAB;
                        keyIndex[k_cur_terminal]++;
                        putc_keyboard(TAB);                                                                      // calls the tab in putc
                    } else if (key_pressed == CTRL) {                                                   // sets the ctrl index and flag
                        ctrlFlag = 1;
                        ctrlIndex = keyIndex[k_cur_terminal];
                    } else {

                        for (i = 0; i < LETTER_NUM; i++){                                                       // iterrates through the letter scan code 
                            if (scanLetter[i][1] == scanCode[key_pressed][1]){                          
                                capsSpecialFlag = 0;                                                    // sets cpasSpecialFlag to 1 if not a letter
                                break;
                            } 
                            capsSpecialFlag = 1;
                        }

                        if (ctrlFlag && key_pressed == L_KEY && ctrlIndex == keyIndex[k_cur_terminal]) {                                     // sets ctrl+l : clears the screen an resets cursor
                            keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal]] = scanCode[key_pressed][0];                                           // adds l to buffer, but clears the keyboard buffer after
                            clear();
                            resetBuff(k_cur_terminal);                                                                                    // reset buffer and flags
                            ctrlFlag = 0;
                            keyIndex[k_cur_terminal] = 0;
                        } else if (ctrlFlag && key_pressed == L_KEY && ctrlIndex == keyIndex[k_cur_terminal] && shiftChar) {                 // case: ctrl+shift+l aka ctrl+L
                            keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal]] = scanCode[key_pressed][0];
                            clear();
                            resetBuff(k_cur_terminal);
                            ctrlFlag = 0;
                            keyIndex[k_cur_terminal] = 0;
                        } else if (altFlag && (key_pressed == F1 || key_pressed == F2 || key_pressed == F3)) { // case where alt and F1/F2/F3 are pressed
                            if(key_pressed == F1 && k_cur_terminal != 0){ // terminal switches to terminal 0 if current terminal is not 0
                                send_eoi(KEYBOARD_IRQ);
                                terminal_switch(0);
                                sti();
                                return;
                            }else if(key_pressed == F2 && k_cur_terminal != 1){ // terminal switches to terminal 1 if current terminal is not 1
                                send_eoi(KEYBOARD_IRQ);
                                terminal_switch(1);
                                sti();
                                return;
                            }else if(key_pressed == F3 && k_cur_terminal != 2){ // terminal switches to terminal 2 if current terminal is not 2
                                send_eoi(KEYBOARD_IRQ);
                                terminal_switch(2);
                                sti();
                                return;
                            }
                        } else if (!specialFlag && key_pressed != BACKSPACE){   
                            if (key_pressed == F1 || key_pressed == F2 || key_pressed == F3){                                       // No printing to screen for F1/2/3
                                f_flag = 0;
                                keyIndex[k_cur_terminal]--;
                            } else if (capsChar && shiftChar && !capsSpecialFlag) {                                                // caps logic for shift and unshift combinations for all keyboard keys
                                keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal]] = scanCode[key_pressed][0];                                       // does not allow special characters if both caps and shift held
                                putc_keyboard(scanCode[key_pressed][0]); 
                                capsSpecialFlag = 0;
                            } else if (capsChar && shiftChar && capsSpecialFlag) {                                          // allows for special chars if cpas is on but shift is held down
                                keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal]] = scanCode[key_pressed][1];
                                putc_keyboard(scanCode[key_pressed][1]);
                                capsSpecialFlag = 0;
                            } else if (capsChar && capsSpecialFlag && !shiftChar) {                                         // does not allow special characters if caps but not shift
                                keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal]] = scanCode[key_pressed][0];
                                putc_keyboard(scanCode[key_pressed][0]);
                                capsSpecialFlag = 0;
                            } else if (capsChar || shiftChar) {                                                             // prints upper case on letters
                                keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal]] = scanCode[key_pressed][1];
                                putc_keyboard(scanCode[key_pressed][1]);
                            } else {                                                                                        //prints lower case
                                keyboard_buffer[k_cur_terminal][keyIndex[k_cur_terminal]] = scanCode[key_pressed][0];
                                putc_keyboard(scanCode[key_pressed][0]);
                            }
                            keyIndex[k_cur_terminal]++;
                        }
                    }
                }
            }
        }
    } else { // in the case of terminal switching when keyboard interrupts outside of alt + F1/F2/F3 are masked (pingpong and fish)
        if (key_pressed == L_ALT || key_pressed == R_ALT){                                                     // sets alt flag
                    altFlag = 1;
        }

        if (altFlag && (key_pressed == F1 || key_pressed == F2 || key_pressed == F3)) {                         // allows terminal switching even if you can't type to current terminal
            if(key_pressed == F1 && k_cur_terminal != 0){
                send_eoi(KEYBOARD_IRQ);
                sti();
                terminal_switch(0);
                return;
            }else if(key_pressed == F2 && k_cur_terminal != 1){
                send_eoi(KEYBOARD_IRQ);
                sti();
                terminal_switch(1);
                return;
            }else if(key_pressed == F3 && k_cur_terminal != 2){
                send_eoi(KEYBOARD_IRQ);
                sti();
                terminal_switch(2);
                return;
            }
        }
    }
    
    send_eoi(KEYBOARD_IRQ);                                 // send eoi to the interrupt controller
    sti();
}

/* function     : resetBuff
 * input        : terminal - terminal we want to reset
 * output       : keyboard buffer for terminal is cleared
 * Description  : Helper function to reset the keyboard_buffer for a certain terminal. Also sets the
 *                keyIndex for that terminal to 0
 * return       : nothing
 */
void resetBuff(int terminal){                                     
    enterPress = 0;
    int i;
    for (i = 0; i < KEY_BUFF_SIZE; i++){                    // fills buffer with 0
        keyboard_buffer[terminal][i] = 0;
    }
    keyIndex[terminal] = 0;                                           // reset keyIndex to 0
}




