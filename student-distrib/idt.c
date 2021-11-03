#include "idt.h"
#include "rtc.h"
#include "idt_linkage.h"
#include "keyboard.h"
#include "syscall.h"

#define exp(acronym, message)\
void acronym() {\
cli();\
printf("%s\n", #message);\
halt(0);\
while(1);\
sti();\
}\

//define all exceptions

exp(DE, "Divide Error Exception");
exp(DB, "Debug Exception");
exp(NMI, "NMI Interrupt");
exp(BP, "Breakpoint Exception");
exp(OF, "Overflow Exception");
exp(BR, "BOUND Range Exceeded Exception");
exp(UD, "Invalid Opcode Exception");
exp(NM, "Device Not Available Exception");
exp(DF, "Double Fault Exception");
exp(CSO, "Coprocessor Segment Overrun");
exp(TS, "Invalid TSS Exception");
exp(NP, "Segment Not Present");
exp(SS, "Stack Fault Exception");
exp(GP, "General Protection Exception");
exp(PF, "Page-Fault Exception");
exp(MF, "x87 FPU Floating-Point Error");
exp(AC, "Alignment Check Exception");
exp(MC, "Machine-Check Exception");
exp(XF, "SIMD Floating-Point Exception");
exp(UDF, "Undefined Interrupt.");

/* referenced from page 156 of the ia-32 document for struct members. */
/* referenced from page 145 of the ia-32 document for system exception vectors. */

/* void idt_fill()
 *fill in the idt table to display. 
 * Inputs: None
 * Outputs: None
 */
void idt_fill(){
    int i;
    for (i = 0; i < NUM_VEC; i++){
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        if (i >= 0x20){
            idt[i].reserved3 = 0;
        }
        else idt[i].reserved3 = 1;// set reserved bit 1 and 2 to 1, and bit 3 to 1 when defined, 0 when undefined, and set bit 4 to 0
        idt[i].reserved2 = 1;     //according to ia-32 document 
        idt[i].reserved1 = 1;
        idt[i].size = 1;          // gate size is 32 bit.
        idt[i].reserved0 = 0;
        if (i == SYS_VEC){
            idt[i].dpl = 3;
            /* system vector belongs to trap gate. */
            idt[i].reserved3 = 1;
        }
        else idt[i].dpl = 0;
        idt[i].present = 1;

        if (i >= 0x20){
            SET_IDT_ENTRY(idt[i], UDF);// if >= 0x20, we set the entry to undefined
        }
        
    }
    //set interrupt entries.
    SET_IDT_ENTRY(idt[0] , DE);  /* Interrupt 0:  Divide Error Exception */
    SET_IDT_ENTRY(idt[1] , DB);  /* Interrupt 1:  Debug Exception */
    SET_IDT_ENTRY(idt[2] , NMI); /* Interrupt 2:  NMI Interrupt */
    SET_IDT_ENTRY(idt[3] , BP);  /* Interrupt 3:  Breakpoint Exception */
    SET_IDT_ENTRY(idt[4] , OF);  /* Interrupt 4:  Overflow Exception */
    SET_IDT_ENTRY(idt[5] , BR);  /* Interrupt 5:  BOUND Range Exceeded Exception */
    SET_IDT_ENTRY(idt[6] , UD);  /* Interrupt 6:  Invalid Opcode Exception */
    SET_IDT_ENTRY(idt[7] , NM);  /* Interrupt 7:  Device Not Available Exception */
    SET_IDT_ENTRY(idt[8] , DF);  /* Interrupt 8:  Double Fault Exception */
    SET_IDT_ENTRY(idt[9] , CSO); /* Interrupt 9:  Coprocessor Segment Overrun */
    SET_IDT_ENTRY(idt[10], TS);  /* Interrupt 10: Invalid TSS Exception */
    SET_IDT_ENTRY(idt[11], NP);  /* Interrupt 11: Segment Not Present */
    SET_IDT_ENTRY(idt[12], SS);  /* Interrupt 12: Stack Fault Exception */
    SET_IDT_ENTRY(idt[13], GP);  /* Interrupt 13: General Protection Exception */
    SET_IDT_ENTRY(idt[14], PF);  /* Interrupt 14: Page-Fault Exception */
                                 /* Interrupt 15: Reserved by Intel */
    SET_IDT_ENTRY(idt[16], MF);  /* Interrupt 16: x87 FPU Floating-Point Error */
    SET_IDT_ENTRY(idt[17], AC);  /* Interrupt 17: Alignment Check Exception */
    SET_IDT_ENTRY(idt[18], MC);  /* Interrupt 18: Machine-Check Exception */
    SET_IDT_ENTRY(idt[19], XF);  /* Interrupt 19: SIMD Floating-Point Exception */

    SET_IDT_ENTRY(idt[RTC_VEC], rtc_handler_linkage);
    SET_IDT_ENTRY(idt[KEYBOARD_VEC], keyboard_handler_linkage);
    SET_IDT_ENTRY(idt[SYS_VEC], system_handler);
    SET_IDT_ENTRY(idt[PIT_VEC], pit_handler_linkage);
    //load idt.
    lidt(idt_desc_ptr);
    return;
}




