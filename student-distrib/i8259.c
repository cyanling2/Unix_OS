#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask=0xFF; /* IRQs 0-7  */
uint8_t slave_mask=0xFF;  /* IRQs 8-15 */

/* 
 * void i8259_init(void)
 * Input: none
 * Return Value: none
 * Function: Initializes the PIC
 */
void i8259_init(void)
{
	/* Interrupt Control Word 1 for master and slave PIC. */
    outb(ICW1,MASTER_8259_PORT);     
	outb(ICW1,SLAVE_8259_PORT);

	/* Interrupt Control Word 2 for master and slave PIC. */
    outb(ICW2_MASTER,MASTER_8259_DATA);
	outb(ICW2_SLAVE,SLAVE_8259_DATA);

	/* Interrupt Control Word 3 for master and slave PIC. */
    outb(ICW3_MASTER,MASTER_8259_DATA);
	outb(ICW3_SLAVE,SLAVE_8259_DATA);

	/* Interrupt Control Word 4 for master and slave PIC. */
    outb(ICW4,MASTER_8259_DATA);
    outb(ICW4,SLAVE_8259_DATA);

	/* Mask all interrupt lines on both master and slave PIC. */
    outb(master_mask,MASTER_8259_DATA);    
    outb(slave_mask,SLAVE_8259_DATA);       

	/* enable IRQ line #2. */
    enable_irq(SLAVE_LINE);      
}
/* 
 * void enable_irq(uint32_t irq_num)
 * Input: uint32_t irq_num: The irq line number.
 * Return Value: none
 * Function: Enable the urq line specified by the irq_num
 */
void enable_irq(uint32_t irq_num)
{
    uint8_t bitmask = 0x01;       
	/* sanity check, if irq number is illegal than we may return. */
    if(irq_num > 15 || irq_num < 0){
		return;
	}   
		
    if(irq_num < MASTER_THRESHOLD){
		/* set master bit indicated by the irq_num to 0 -- enabled. */
        bitmask = bitmask << irq_num;
		bitmask = ~bitmask;   
        master_mask = master_mask & bitmask;           
        outb(master_mask, MASTER_8259_DATA);
    }
    else{
		/* set slave bit indicated by the irq_num to 0 -- enabled. */
        bitmask = bitmask << (irq_num - MASTER_THRESHOLD);
		bitmask = ~bitmask;   
        slave_mask = slave_mask & bitmask;           
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/* 
 * void disable_irq(uint32_t irq_num)
 * Input: uint32_t irq_num: The irq line number.
 * Return Value: none
 * Function: Disable the urq line specified by the irq_num
 */
void disable_irq(uint32_t irq_num)
{
	uint8_t bitmask = 0x01;       
	/* sanity check, if irq number is illegal than we may return. */
    if (irq_num > 15 || irq_num < 0){
		return;
	}   
		
    if (irq_num < MASTER_THRESHOLD){
		/* set master bit indicated by the irq_num to 1 -- disabled. */
        bitmask = bitmask << irq_num;   
        master_mask = master_mask | bitmask;           
        outb(master_mask, MASTER_8259_DATA);
    }
    else{
		/* set slave bit indicated by the irq_num to 1 -- disabled. */
        bitmask = bitmask << (irq_num - MASTER_THRESHOLD);
        slave_mask = slave_mask | bitmask;           
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/* 
 * void send_eoi(uint32_t irq_num)
 * Input: uint32_t irq_num: The irq line number.
 * Return Value: none
 * Function: Send EOI signal to the IR specified by the irq_num
 */
void send_eoi(uint32_t irq_num)
{
    /* sanity check, if irq number is illegal than we may return. */
    if (irq_num > 15 || irq_num < 0){
		return;
	}   
    if (irq_num < MASTER_THRESHOLD){
		/* send EOI specified by the irq line to the master port. */
        outb(EOI + irq_num , MASTER_8259_PORT);          
    }
    else {
		/* send EOI specified by the irq line to the master port. */
		/* resend eoi for the master. */
        outb(EOI + (irq_num-8) , SLAVE_8259_PORT);       
        send_eoi(SLAVE_LINE);        
    }
}
