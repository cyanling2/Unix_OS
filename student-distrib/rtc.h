#include "i8259.h"
#include "lib.h"
#include "tests.h"

#define IRQ_NUM 8
#define RTC_STATUS_A 0x8A 
#define RTC_STATUS_B 0x8B
#define RTC_STATUS_C 0x8C
#define RTC_INDEX 0x70
#define RTC_DATA 0x71
#define RTC_REG_C 0x8C

extern int get_number_of_rtc_interrupts();
void rtc_init();
extern void rtc_interrupt_handler();
/* Open the RTC */
int32_t rtc_open(const uint8_t* filename);
/* Read the RTC */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
/* Write to the RTC */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
/* Set Frequency on RTC */
void rtc_set_freq(int32_t freq);
/* Close the RTC */
int32_t rtc_close(int32_t fd);
