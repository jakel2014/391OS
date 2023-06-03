/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "keyboard.h"

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define RIGHT_CORNER 2000
#define END         0xEE

// Color arrays {Light red, light blue, ligth green}
char ATTRIB[3] = {12, 11, 10}; // holds the color values for the 3 terminals
char default_ATTRIB[3] = {12, 11, 10}; // holds the default color values for the 3 terminals

static int screen_x[3]; // holds the x coordinates for all 3 terminals  
static int screen_y[3]; // holds the y coordinates for all 3 terminals
static char* video_mem = (char *)VIDEO;


/* void change_color(char color);
 * Inputs: color character
 * Return Value: none
 * Function: Changes the color in ATTRIB for the current terminal*/
void change_color(char color) {
    ATTRIB[get_current_terminal()] = color;
}

/* char get_default_color();
 * Inputs: void
 * Return Value: Returns the  default terminal color.
 * Function: Returns the  default terminal color */
char get_default_color() {
    return default_ATTRIB[get_current_terminal()];
}

/* void color_init(void);
 * Inputs: void
 * Return Value: none
 * Function: Sets all the terminals to the default color */
void color_init(void) {
    int i;
    for(i = 0; i < MAX_TERMINALS; i++) {
        int j = 0;
        for (j = 0; j < NUM_ROWS * NUM_COLS; j++) {
            int offset = (1 + i) * size_4kb;                                            // caluclates terminal buffer offset 
            *(uint8_t *)(video_mem + offset + (j << 1)) = ' ';
            *(uint8_t *)(video_mem + offset + (j << 1) + 1) = default_ATTRIB[i];        // sets the terminal specific color to default
        }
    }
    
}

/* void get_terminal_screen_x(int terminal);
 * Inputs: terminal index
 * Return Value: screen x of terminal
 * Function: Getter for the screen x position of the terminal*/
int get_terminal_screen_x(int terminal) {
    return screen_x[terminal];
}

/* void get_terminal_screen_y(int terminal);
 * Inputs: terminal index
 * Return Value: screen y of terminal
 * Function: Getter for the screen y position of the terminal*/
int get_terminal_screen_y(int terminal) {
    return screen_y[terminal];
}


/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    int clear_display_terminal = get_displayed_terminal();
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = default_ATTRIB[clear_display_terminal];
    }

    screen_x[clear_display_terminal] = 0;                                                           // resets the cursor to the top left corner when cleared
    screen_y[clear_display_terminal] = 0;
    updateCursor(clear_display_terminal);
}

/* void updateCursor(void);
 * Inputs: void
 * Return Value: none
 * Function: sets the cursor position on screen to be the current x and y position */
void updateCursor(int terminal) {
    uint16_t pos = screen_y[terminal] * NUM_COLS + screen_x[terminal];                          // gets current cursor positon on screen

    outb(0x0F, 0x3D4);                                                      // 0x0F is the cursor data and 0x3D4 is the port
    outb((uint8_t) (pos & 0xFF), 0x3D5);                                    // outb to ports the cursor positon data

    outb(0x0E, 0x3D4);                                                      // outb to ports the cursor positon data
    outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);                             // 0x0E is the cursor data and 0x3D4/0x3D5 is the port
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}


/* void putc_keyboard(uint8_t c)
 * Inputs: ASCII int character to be printed to display screen
 * Return Value: none
 * Function: Prints the specific character to the display screen called only by the keyboard interrupts.
 */
void putc_keyboard(uint8_t c) {
    cli();
    int display_terminal = get_displayed_terminal();
    if (c == '\n' || c == '\r') {                                                                               // test case for enter
        if (screen_y[display_terminal] == NUM_ROWS - 1) {                                                     // if enters on last row, will scroll
            scrolling_keyboard(END);
            screen_x[display_terminal] = 0;
        } else {                                                                                            // wont scroll enter if not last row
            screen_y[display_terminal]++;
            screen_y[display_terminal] %= NUM_ROWS;
            screen_x[display_terminal] = 0;
        }
    } else if (screen_y[display_terminal] == NUM_ROWS - 1 && screen_x[display_terminal] == NUM_COLS - 1 ){ // if y = 24 and x = 79     // this test case is for bottom right of screen
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[display_terminal] + screen_x[display_terminal]) << 1)) = c;              // outs character to screen in video memory
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[display_terminal] + screen_x[display_terminal]) << 1) + 1) = ATTRIB[display_terminal];
        scrolling_keyboard(END);                                                                            // calls scrolling to start new input on new line
        // screen_y = NUM_ROWS - 1;
        screen_x[display_terminal] = 0;
        // screen_x++;
        //screen_x %= NUM_COLS;
        screen_y[display_terminal] = (screen_y[display_terminal] + (screen_x[display_terminal] / NUM_COLS)) % NUM_ROWS;                           // reset x and y value
    } else if (screen_x[display_terminal] == NUM_COLS - 1 && c == TAB) {                                      // test case for tab at edge of screen
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[display_terminal] + screen_x[display_terminal]) << 1)) = ' ';
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[display_terminal] + screen_x[display_terminal]) << 1) + 1) = ATTRIB[display_terminal];
        screen_y[display_terminal]++;
        screen_x[display_terminal] = 0;
        //screen_x %= NUM_COLS;
        screen_y[display_terminal] = (screen_y[display_terminal] + (screen_x[display_terminal] / NUM_COLS)) % NUM_ROWS;

        int i;
        for (i = 0; i < 3; i++) {                                                                           // calls putc 3 times to finish the tab
            putc_keyboard(' ');
        }
    } else if (screen_x[display_terminal] == NUM_COLS - 1) {                                                // test case for if at edge of screen
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[display_terminal] + screen_x[display_terminal]) << 1)) = c;              // will print character then set cursor
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[display_terminal] + screen_x[display_terminal]) << 1) + 1) = ATTRIB[display_terminal];     // to new line
        screen_y[display_terminal]++;
        screen_x[display_terminal] = 0;
        //screen_x %= NUM_COLS;
        screen_y[display_terminal] = (screen_y[display_terminal] + (screen_x[display_terminal] / NUM_COLS)) % NUM_ROWS;
    } else if (c == TAB){                                                                                   // tab recursivly calls putc for a space 4 times
        int i;
        for (i = 0; i < 4; i++ ) { putc_keyboard(' ');}
    } else {                                                                                                // putc character to screen
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[display_terminal] + screen_x[display_terminal]) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[display_terminal] + screen_x[display_terminal]) << 1) + 1) = ATTRIB[display_terminal];
        screen_x[display_terminal]++;
        screen_x[display_terminal] %= NUM_COLS;
        screen_y[display_terminal] = (screen_y[display_terminal] + (screen_x[display_terminal] / NUM_COLS)) % NUM_ROWS;
    }
    updateCursor(display_terminal);             
    sti();
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console */
void putc(uint8_t c) {
    cli();                                                                  
    int current_putc_terminal = get_current_terminal();
    int displayed_putc_terminal = get_displayed_terminal();
    int offset = 0;

    if(current_putc_terminal != displayed_putc_terminal){                                                           // calculates the terminal puffer offset from display
        offset = (current_putc_terminal + 1) * size_4kb;
    }

    if (c == '\n' || c == '\r') {                                                                                   // test case for enter
        if (screen_y[current_putc_terminal] == NUM_ROWS - 1) {                                                      // if enters on last row, will scroll
            scrolling(END);
            screen_x[current_putc_terminal] = 0;
        } else {                                                                                                    // wont scroll enter if not last row
            screen_y[current_putc_terminal]++;
            screen_y[current_putc_terminal] %= NUM_ROWS;
            screen_x[current_putc_terminal] = 0;
        }
    } else if (screen_y[current_putc_terminal] == NUM_ROWS - 1 && screen_x[current_putc_terminal] == NUM_COLS - 1 ){ // if y = 24 and x = 79     // this test case is for bottom right of screen
        *(uint8_t *)(video_mem + offset + ((NUM_COLS * screen_y[current_putc_terminal] + screen_x[current_putc_terminal]) << 1)) = c;              // outs character to screen in video memory
        *(uint8_t *)(video_mem + offset + ((NUM_COLS * screen_y[current_putc_terminal] + screen_x[current_putc_terminal]) << 1) + 1) = ATTRIB[current_putc_terminal];
        scrolling(END);                                                                                             // calls scrolling to start new input on new line
        // screen_y = NUM_ROWS - 1;
        screen_x[current_putc_terminal] = 0;
        // screen_x++;
        //screen_x %= NUM_COLS;
        screen_y[current_putc_terminal] = (screen_y[current_putc_terminal] + (screen_x[current_putc_terminal] / NUM_COLS)) % NUM_ROWS;                           // reset x and y value
    } else if (screen_x[current_putc_terminal] == NUM_COLS - 1 && c == TAB) {                                      // test case for tab at edge of screen
        *(uint8_t *)(video_mem + offset + ((NUM_COLS * screen_y[current_putc_terminal] + screen_x[current_putc_terminal]) << 1)) = ' ';
        *(uint8_t *)(video_mem + offset + ((NUM_COLS * screen_y[current_putc_terminal] + screen_x[current_putc_terminal]) << 1) + 1) = ATTRIB[current_putc_terminal];
        screen_y[current_putc_terminal]++;
        screen_x[current_putc_terminal] = 0;
        //screen_x %= NUM_COLS;
        screen_y[current_putc_terminal] = (screen_y[current_putc_terminal] + (screen_x[current_putc_terminal] / NUM_COLS)) % NUM_ROWS;

        int i;
        for (i = 0; i < 3; i++) {                                                                                   // calls putc 3 times to finish the tab
            putc(' ');
        }
    } else if (screen_x[current_putc_terminal] == NUM_COLS - 1) {                                                  // test case for if at edge of screen
        *(uint8_t *)(video_mem + offset + ((NUM_COLS * screen_y[current_putc_terminal] + screen_x[current_putc_terminal]) << 1)) = c;              // will print character then set cursor
        *(uint8_t *)(video_mem + offset + ((NUM_COLS * screen_y[current_putc_terminal] + screen_x[current_putc_terminal]) << 1) + 1) = ATTRIB[current_putc_terminal];     // to new line
        screen_y[current_putc_terminal]++;
        screen_x[current_putc_terminal] = 0;
        //screen_x %= NUM_COLS;
        screen_y[current_putc_terminal] = (screen_y[current_putc_terminal] + (screen_x[current_putc_terminal] / NUM_COLS)) % NUM_ROWS;
    } else if (c == TAB){                                                                                           // tab recursivly calls putc for a space 4 times
        int i;
        for (i = 0; i < 4; i++ ) { putc(' ');}
    } else {                                                                                                        // putc character to screen
        *(uint8_t *)(video_mem + offset + ((NUM_COLS * screen_y[current_putc_terminal] + screen_x[current_putc_terminal]) << 1)) = c;
        *(uint8_t *)(video_mem + offset + ((NUM_COLS * screen_y[current_putc_terminal] + screen_x[current_putc_terminal]) << 1) + 1) = ATTRIB[current_putc_terminal];
        screen_x[current_putc_terminal]++;
        screen_x[current_putc_terminal] %= NUM_COLS;
        screen_y[current_putc_terminal] = (screen_y[current_putc_terminal] + (screen_x[current_putc_terminal] / NUM_COLS)) % NUM_ROWS;
    }
    if(current_putc_terminal == displayed_putc_terminal){
        updateCursor(current_putc_terminal);                                                                         // updates cursor position
    }
    sti();
}

/* void scrolling(uint8_t key);
 * Inputs: uint_8 t = test condition key
 * Return Value: void
 *  Function: Scrolling of video memory on screen  */
void scrolling(uint8_t key){
    int i, j;

    int current_scrolling_terminal = get_current_terminal();
    int displayed_scrolling_terminal = get_displayed_terminal();
    int offset = 0;

    if(current_scrolling_terminal != displayed_scrolling_terminal){                             // calculates the terminal buffer offset from display
        offset = (current_scrolling_terminal + 1) * size_4kb;
    }

    if (key == END) {                                                                           // checks condition is end of screen
        for (j = 0; j < NUM_ROWS - 1; j++){                                                     // itterates through rows and coloumns
            for (i = 0; i < NUM_COLS; i++) {
                // iterates through all positions on screen in video memory
                    *(uint8_t *)(video_mem + offset + ((NUM_COLS * (j) + i ) << 1)) =                    // Sets row memory to equal the one after
                        *(uint8_t *)(video_mem + offset + ((NUM_COLS * (j+1) + i) << 1));
                    *(uint8_t *)(video_mem + offset + ((NUM_COLS * (j) + i) << 1) + 1) = *(uint8_t *)(video_mem + offset + ((NUM_COLS * (j+1) + i) << 1) + 1);

                if (j == NUM_ROWS - 2) {                                                        // checks if at bottom of screen, will clear the bottom line
                    *(uint8_t *)(video_mem + offset + ((NUM_COLS * (j+1) + i) << 1)) = ' ';
                    *(uint8_t *)(video_mem + offset + ((NUM_COLS * j + i) << 1) + 1) = *(uint8_t *)(video_mem + offset + ((NUM_COLS * (j+1) + i) << 1) + 1);
                }
            }
        }
    }

}

/* void scrolling_keyboard(uint8_t key);
 * Inputs: uint_8 t = test condition key
 * Return Value: void
 *  Function: Scrolling of video memory on screen (for keyboard) */
void scrolling_keyboard(uint8_t key){
    int i, j;

    if (key == END) {                                                                           // checks condition is end of screen
        for (j = 0; j < NUM_ROWS - 1; j++){                                                     // itterates through rows and coloumns
            for (i = 0; i < NUM_COLS; i++) {
                // iterates through all positions on screen in video memory
                    *(uint8_t *)(video_mem + ((NUM_COLS * (j) + i ) << 1)) =                    // Sets row memory to equal the one after
                        *(uint8_t *)(video_mem + ((NUM_COLS * (j+1) + i) << 1));
                    *(uint8_t *)(video_mem + ((NUM_COLS * (j) + i) << 1) + 1) = *(uint8_t *)(video_mem + ((NUM_COLS * (j+1) + i) << 1) + 1);

                if (j == NUM_ROWS - 2) {                                                        // checks if at bottom of screen, will clear the bottom line
                    *(uint8_t *)(video_mem + ((NUM_COLS * (j+1) + i) << 1)) = ' ';
                    *(uint8_t *)(video_mem + ((NUM_COLS * j + i) << 1) + 1) = *(uint8_t *)(video_mem + ((NUM_COLS * (j+1) + i) << 1) + 1);
                }
            }
        }
    }

}

/* void backspace(void);
 * Inputs: void
 * Return Value: void
 *  Function: backspaces character and removes video memory on screen*/
void backspace(void) {
    int b_displayed_terminal = get_displayed_terminal();
    uint16_t pos = screen_y[b_displayed_terminal] * NUM_COLS + screen_x[b_displayed_terminal];                                          // gets current screen position
    if (pos == screen_y[b_displayed_terminal] * NUM_COLS){                                                        // if screen_x = 0 sets y to previous line
        screen_x[b_displayed_terminal] = 80;                                                                      // sets x to last column position    
        screen_y[b_displayed_terminal]--;
        if (screen_y[b_displayed_terminal] < 0) {                                                                 // condition on backspace in top left corner
            screen_y[b_displayed_terminal] = 0;                                                                   // doesnt alter video memory if backspace 
            screen_x[b_displayed_terminal] = 0;
            return;             
        }
        pos = screen_y[b_displayed_terminal] * NUM_COLS + screen_x[b_displayed_terminal];
        if (pos != 0){                                                                      // if not in top left corner
            screen_x[b_displayed_terminal] --;                                                                    // backspaces in video memory, 
            screen_x[b_displayed_terminal] %= NUM_COLS;                                                           // removes printed character
            // screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[b_displayed_terminal] + screen_x[b_displayed_terminal]) << 1)) = ' ';
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[b_displayed_terminal] + screen_x[b_displayed_terminal]) << 1) + 1) = ATTRIB[b_displayed_terminal];
        }
    } else { 
        if (screen_y[b_displayed_terminal] < 0) {                                                                 // checks that doesnt go past the first row
            screen_y[b_displayed_terminal] = 0;
            screen_x[b_displayed_terminal] = 0;
            return;
        }

        pos = screen_y[b_displayed_terminal] * NUM_COLS + screen_x[b_displayed_terminal];

        if (pos != 0){                                                                      // backsaces in video memory 
            screen_x[b_displayed_terminal] --;    
            screen_x[b_displayed_terminal] %= NUM_COLS;
            // screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[b_displayed_terminal] + screen_x[b_displayed_terminal]) << 1)) = ' ';
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[b_displayed_terminal] + screen_x[b_displayed_terminal]) << 1) + 1) = ATTRIB[b_displayed_terminal];
        }
    }
    updateCursor(b_displayed_terminal);                                                                         // updates cursor position 
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}





