#include "x86_desc.h"
#include "lib.h"
#define SYS_VEC 0x80
#define RTC_VEC 0x28
#define PIT_VEC 0x20
// fills in the interrupt vector.
extern void idt_fill();

