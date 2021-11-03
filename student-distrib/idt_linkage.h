#ifndef idt_linkage_H
#define idt_linkage_H
#ifndef ASM
extern void rtc_handler_linkage();
extern void keyboard_handler_linkage(); 
extern void pit_handler_linkage(); 
extern void system_handler(); 
#endif
#endif



