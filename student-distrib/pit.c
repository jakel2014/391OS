#include "types.h"
#include "lib.h"
#include "pit.h"

/* void pit_init(void);
 * Inputs: void
 * Return Value: none
 * Function: Initializes the PIT */
void pit_init(void){
    cli();

    int freq_MHZ = MAX_HZ / 100;                        // sets the process switching to frequency of 100Hz
    outb(PIT_CMD_PORT, PIT_CMD_REG);                           // Maps the pit device and sets the frequency of the device
    outb(freq_MHZ & 0xFF, PIT_DATA_PORT); // low byte
    outb(freq_MHZ >> 8, PIT_DATA_PORT); // high byte

    sti();

    enable_irq(PIT_IRQ_NUM);                            // enables PIT interrupts
    return;
}
