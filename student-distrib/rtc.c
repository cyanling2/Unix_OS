#include "rtc.h"

/* for testing only. */
int number_of_rtc_interrupts = 0;
int interrupt_received;
int get_number_of_rtc_interrupts(){
    return number_of_rtc_interrupts;
}

/* void rtc_init()
 *initialize rtc
 * Inputs: None
 * Outputs: None
 */
void rtc_init(){
    disable_irq(IRQ_NUM);// no interrupt happen
    outb(RTC_STATUS_B, RTC_INDEX);//set the index, select register B
    char prev = inb(RTC_DATA);//read current value
    outb(RTC_STATUS_B, RTC_INDEX);//set the index again
    outb(prev | 0x40, RTC_DATA);//write previous value with 0x40, turn on bit 6 of register B
    enable_irq(IRQ_NUM);
	interrupt_received = 0;
}

/* void keyboard_interrupt_handler()
 *read from keyport, get the key from key map, and send the interrupt. 
 * Inputs: None
 * Outputs: None
 */
void rtc_interrupt_handler(){
    cli();
    outb(RTC_REG_C, RTC_INDEX);//set index, select register C
    inb(RTC_DATA);//throw away content
    interrupt_received = 1;
    send_eoi(IRQ_NUM);// send to end of interrupt
    sti();
}

/*
*	Function: rtc_open()
*	Description: This will be our main funciton to open the RTC
*	input: pointer to filename
*	output: returns 0 always
*	effects: Opens the rtc and initializes frequency to 2 HZ
*/
int32_t 
rtc_open(const uint8_t * filename){

	/* Set RTC Frequency to 2 Hz */
    rtc_set_freq(2);

    /* Always return 0 */
    return 0;
}
 

 /*
 *	Function: rtc_read()
 *	Description: This will read the contents within the RTC only after an interrupt has occured
 *	input: file descriptor, buffer to read into, and number of bytes
 *	output: returns 0 upon success
 *	effects: Reads the RTC
 */
int32_t 
rtc_read(int32_t fd, void* buf, int32_t nbytes){

	interrupt_received = 0;
	while (1)
	{
		if (interrupt_received == 1)
			break;
	}
	return 0;
 }


 /*
 *	Function: rtc_write()
 *	Description: 
 *	input: file descriptor, pointer to buffer being modified, number of bytes writing
 *	output: returns 0 always
 *	effects:
 */
int32_t 
rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    /* Local variables. */
	int32_t freq;

	/* Boundary check - ONLY 4 Bytes */	
	if (4 != nbytes || buf == NULL) 
		return -1;  /* Fail - always need to write 4 bytes) */
	else 
		freq = *((int32_t*) buf);

	/* Set the RTC Frequency to our variable freq */
	rtc_set_freq(freq);
	
	/* Return the number of bytes wrote always */
	return nbytes;   
}


/*
*   Function: rtc_set_frequency
*   Description: This function sets the frequency on the RTC to be user defined
*   input:	int freq = frequency desired
*   output: none
*   effects: sets RTC Frequency
*/
void
rtc_set_freq(int32_t freq){
    /* Local variables. */
    char rate = 0x00;

    /* Save old value of Reg A*/
    outb(RTC_STATUS_A, RTC_INDEX);
    unsigned char a_old = inb(RTC_DATA);

    /* Values defined in RTC Datasheet (pg.19) */
	if (freq == 8192 || freq == 4096 || freq == 2048) return;
    if (freq >= 1024 && freq < 2048) rate = 0x06;
    if (freq >= 512 && freq < 1024) rate = 0x07;
    if (freq >= 256 && freq < 512) rate = 0x08;
    if (freq >= 128 && freq < 256) rate = 0x09;
    if (freq >= 64 && freq < 128) rate = 0x0A;
    if (freq >= 32 && freq < 64) rate = 0x0B;
    if (freq >= 16 && freq < 32) rate = 0x0C;
    if (freq >= 8 && freq < 16) rate = 0x0D;
    if (freq >= 4 && freq < 8) rate = 0x0E;
    if (freq >= 2 && freq < 4) rate = 0x0F;
    if (rate == 0x00) return;
    /* set A[3:0] (rate) to rate */
    outb(RTC_STATUS_A, RTC_INDEX);
    outb((0xF0 & a_old) | rate, RTC_DATA);
}


 /*
 *	Function: rtc_close()
 *	Description: This is the main function to close the RTC.
 *	input: pointer to file descriptor being closed
 *	output: returns 0 always
 *	effects: closes the RTC
 */
int32_t 
rtc_close(int32_t fd){
	
	 /* Reset RTC Frequency to 2 Hz */
    rtc_set_freq(2);

    /* Always return 0 */
    return 0;
 }


