#include "lib.h"
#include "i8259.h"
#include "paging.h"
#include "keyboard.h"
#include "syscall.h"

#define PIT_FREQ 1193180 / 100
#define PIT_IRQ 0x0
#define PIT_CH0 0x40
#define PIT_COMM_REG 0x43
#define PIT_SQUARE_WAV 0x36
#define MSK 0xFF
#define UPPER_SHIFT 8

uint8_t term_pointer;
/*Initlization*/
void pit_init(void);
/*handle the pit interrupt*/
void pit_interrupt_handler(void);
