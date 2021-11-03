#include "scheduling.h"
/* 
void pit_init(void)
input:  None
output: None
effect: Initialize the pit functionality
*/
/* Referenced from: https://wiki.osdev.org/Programmable_Interval_Timer */
void pit_init(void){
    outb(PIT_SQUARE_WAV, PIT_COMM_REG);
    outb((PIT_FREQ & MSK), PIT_CH0);
    outb((PIT_FREQ >> UPPER_SHIFT), PIT_CH0);
    enable_irq(PIT_IRQ);
    term_pointer = 0;
    return;
}
/*
 *Input: None
 *Output: None
 *Effect: handle the pit interrupt
*/

void pit_interrupt_handler(void){
    uint8_t current_term, next_term, next_pid, pointer_pid;
    pcb_t* pointer_pcb;
    pcb_t* next_pcb;
    /* Send EOI for the PIT IRQ line. */
    send_eoi(PIT_IRQ);
    cli();
    
    /* find the next terminal to process. */
    current_term = get_current_term();
    next_term = (term_pointer + 1) % TERM_MAX;
    pointer_pid = fetch_latest_process(term_pointer);
    next_pid = fetch_latest_process(next_term);
    pointer_pcb = (pcb_t*) (PROG_PHYSTART - (pointer_pid + 1) * PCB_SIZE);
    next_pcb = (pcb_t*) (PROG_PHYSTART - (next_pid + 1) * PCB_SIZE);
    if (term_sts_getter(term_pointer) != 0){
        /* save into current kbp or other things. */
        asm volatile(
            "movl %%ebp, %%eax;"
            "movl %%esp, %%ebx;"
            /* there is no input here */
            :"=a"(pointer_pcb->current_kbp), "=b"(pointer_pcb->current_ksp)
        );
    }
    
    /* checkpoint 1: check whether all three terminals bootup: Passed!*/ 
    if(term_sts_getter(next_term) == 0) 
    {   
        term_save(term_pointer);
        term_pointer = next_term;
        term_start(next_term); 
        return;
    }

    
    /* checkpoint 2: check whether sync works well. */
    /* process switch. (buggy code!) */
    term_pointer = next_term;
    // pcb_mapping(PROG_PHYSTART + next_pid * PROG_PAGE_SIZE, PROG_VIRSTART);
    // if (get_current_term() != term_pointer){
    //     syscall_video_mapping((uint32_t) term_vmm_getter(next_term));
    //     set_vidmem((char*) term_vmm_getter(next_term));
    // }
    // else {
    //     syscall_video_mapping(0xB8000);
    //     set_vidmem((char*) 0xB8000);
    // }
    // // There might be cases the process that the pit handler executes is not the current process that we see. 
    // // Modify TSS 
    // tss.ss0 = KERNEL_DS;
    // // 8MB - PCB_numbr * 8KB - 4 
    // tss.esp0 = 0x800000 - next_pid * 0x2000 - 0x4;

    // asm volatile(
    //     "movl %0, %%ebp;"
    //     "movl %1, %%esp;"
    //     :/* there is no output here */ 
    //     :"r"(next_pcb->current_kbp),"r"(next_pcb->current_ksp)
    // ); 
    return;
}
