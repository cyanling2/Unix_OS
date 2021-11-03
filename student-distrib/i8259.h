#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT    0x20
#define SLAVE_8259_PORT     0xA0
#define MASTER_8259_DATA    0x21
#define SLAVE_8259_DATA     0xA1

/* Slave IRQ line. */
#define SLAVE_LINE 2
#define MASTER_THRESHOLD 8
/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11        //select pic, interrupt mask reg cleared, IR7 priority 7, slave mode address = 7
#define ICW2_MASTER         0x20        //IR0-7 mapped to 0x20-0x27
#define ICW2_SLAVE          0x28        //IR0-7 mapped to 0x28-0x2F
#define ICW3_MASTER         0x04        //master's IR2 have a slave (bit:100)
#define ICW3_SLAVE          0x02        //slave is IR2 on master (number: 2)
#define ICW4                0x01        //set to enable different modes, support for AEOI

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
