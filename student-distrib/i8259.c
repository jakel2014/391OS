/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */ // Masks the interrupts
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* I adapted my solution from the Linux version i8259.c in course notes  */

/* void i8259_init(void)
 * Inputs: void
 * Return Value: void
 * Function: initializes both master and slave PICs using given ICW values */
void i8259_init(void) {

    /* Mask all the interrupts*/
    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);

    /* Initialize the pics - Start with ICW1 */
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    /* Initialize the pics - ICW2 */
    /* ICW2: 8259A-2 IR0-7 mapped to ISA_IRQ_VECTOR(8) */ 
    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);

    /* Initialize the pics - ICW3 */
    outb(ICW3_MASTER, MASTER_8259_PORT + 1);
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);

    /* Initialize the pics - ICW4 */
    outb(ICW4, MASTER_8259_PORT + 1);
    outb(ICW4, SLAVE_8259_PORT + 1);

    /*Enable slave interrupts*/
    enable_irq(SLAVE_8259_PORT_02); 
}

/* void enable_irq(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: void
 * Function: Enables interrupt requests for a given IRQ number by updating the appropriate bitmask */
void enable_irq(uint32_t irq_num) {

    if (irq_num > 15 || irq_num < 0) {                             // check if within bounds
        return;
    }

    if (irq_num >= 8){                              // We are dealing with slave 
        irq_num -= 8;
        unsigned int maskTemp = 1 << irq_num;       // Put 1 on irq_num bit of the mask
        slave_mask &= ~maskTemp;                    

        outb(slave_mask, SLAVE_8259_PORT + 1);      // Write the mask into the 8259
    } else {                                        // We are working with master
        unsigned int maskTemp = 1 << irq_num;
        master_mask &= ~maskTemp;

        outb(master_mask, MASTER_8259_PORT + 1);    // Write the mask into the 8259
    }

}

/* void disable_irq(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: void
 * Function: Disables interrupt requests for a given IRQ number by updating the appropriate bitmask */
void disable_irq(uint32_t irq_num) {

    if (irq_num > 15) {                             // check if within bounds
        return;
    }

    if (irq_num >= 8){                              // We are dealing with slave 
        irq_num -= 8;
        unsigned int maskTemp = 1 << irq_num;       // Put 1 on irq_num bit of the mask
        slave_mask |= maskTemp;                    

        outb(slave_mask, SLAVE_8259_PORT + 1);      // Write the mask into the 8259
    } else {                                        // We are working with master
        unsigned int maskTemp = 1 << irq_num;
        master_mask |= maskTemp;

        outb(master_mask, MASTER_8259_PORT + 1);    // Write the mask into the 8259
    }

}


/* void send_eoi(uint32_t irq_num)
 * Inputs: uint32_t irq_num
 * Return Value: void
 * Function: Signals either the master or both master and slave PICs (depending on irq_num) that the interrupt has ended */
void send_eoi(uint32_t irq_num) {
    if (irq_num > 15) {                             // check if within bounds
        return;
    }

    if (irq_num >= 8){                              // We are dealing with slave 
        irq_num -= 8;
        unsigned int maskTemp = EOI;                // Put 1 on irq_num bit of the mask
        maskTemp |= irq_num;

        outb(maskTemp, SLAVE_8259_PORT);            // Write the mask into the 8259
        send_eoi(SLAVE_8259_PORT_02);                   
    } else {                                        // We are working with master
        unsigned int maskTemp = EOI;                // Send EOI for the slave
        maskTemp |= irq_num;

        outb(maskTemp, MASTER_8259_PORT);        // Write the mask into the 8259
    }
}
