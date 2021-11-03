#define ASM 1
#include "idt_linkage.h"
.globl pit_handler_linkage, keyboard_handler_linkage, rtc_handler_linkage, system_handler

keyboard_handler_linkage:
    pushl   %fs
    pushl   %es
    pushl   %ds
    pushl   %ebp
    pushl   %edi
    pushl   %esi
    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    pushl   %eax
    call   keyboard_interrupt_handler
    popl   %eax
    popl   %ebx
    popl   %ecx
    popl   %edx
    popl   %esi
    popl   %edi
    popl   %ebp
    popl   %ds
    popl   %es
    popl   %fs
    iret

pit_handler_linkage:
    pushl   %fs
    pushl   %es
    pushl   %ds
    pushl   %ebp
    pushl   %edi
    pushl   %esi
    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    pushl   %eax
    call   pit_interrupt_handler
    popl   %eax
    popl   %ebx
    popl   %ecx
    popl   %edx
    popl   %esi
    popl   %edi
    popl   %ebp
    popl   %ds
    popl   %es
    popl   %fs
    iret

rtc_handler_linkage:
    pushl   %fs
    pushl   %es
    pushl   %ds
    pushl   %ebp
    pushl   %edi
    pushl   %esi
    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    pushl   %eax
    call   rtc_interrupt_handler
    popl   %eax
    popl   %ebx
    popl   %ecx
    popl   %edx
    popl   %esi
    popl   %edi
    popl   %ebp
    popl   %ds
    popl   %es
    popl   %fs
    iret

system_handler:
    pushl   %fs
    pushl   %es
    pushl   %ds
    pushl   %ebp
    pushl   %edi
    pushl   %esi
    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    pushfl                # flag variable will be changed so we need to push old flags onto the stack.

    pushl %edx            # push arguments.
    pushl %ecx
    pushl %ebx

    cmpl $1, %eax
    jl invalid
    cmpl $10, %eax
    jg invalid

    call *jump_table(, %eax, 4)       # if valid, then call the corresponding functions.
    jmp end

invalid:
    movl $-1, %eax
end:
    popl %ebx             # pop arguments.
    popl %ecx
    popl %edx

    popfl
    popl   %ebx
    popl   %ecx
    popl   %edx
    popl   %esi
    popl   %edi
    popl   %ebp
    popl   %ds
    popl   %es
    popl   %fs
    iret

jump_table:
    .long empty
    .long halt
    .long execute
    .long read 
    .long write
    .long open
    .long close 
    .long getargs
    .long vidmap
    .long empty
    .long empty




