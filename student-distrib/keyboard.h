#include "lib.h"
#include "paging.h"
#define KEYBOARD_IRQ 1
#define KEY_MAP_SIZE 255
#define KEY_PORT 0x60
#define NUM_KEYS 128
#define KEYBOARD_VEC 0x21
#define KBUFF_MAX 128
#define ENTER     0x1C
#define BACKSPACE 0x0E
#define CAPS_ON 0x3A
#define CAPS_OFF 0xBA
#define RSHIFT_ON 0x36
#define RSHIFT_OFF 0xB6
#define LSHIFT_ON 0x2A
#define LSHIFT_OFF 0xAA
#define CTRL_ON 0x1D
#define CTRL_OFF 0x9D
#define TKBUFF_MAX 128
#define TAB 0x0F
#define ALT_ON 0x38
#define ALT_OFF 0xB8
#define TERM_MAX 3
#define CURSOR_NUM 2
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D
#define MAX_PROC_PT 4

extern int get_number_of_key_presses();

void keyboard_init();
extern void keyboard_interrupt_handler();
uint8_t caps_map[KEY_MAP_SIZE];
uint8_t sh_map[KEY_MAP_SIZE];
void caps_init(void);
void sh_init(void);

void term_init(void);
int term_start(uint8_t term_id);
void term_swap(uint8_t src, uint8_t dst);
void term_save(uint8_t term_id);
void term_restore(uint8_t term_id);
int term_open(const uint8_t * filename);
int term_close(int32_t fd);
int term_read(int32_t fd, void* buf, int32_t length);
int term_write(int32_t fd, const void* buf, int32_t length);

/* term getters and setters. */
uint8_t term_tid_getter(uint8_t term_no);
uint8_t term_pid_getter(uint8_t term_no);
uint32_t* term_cursor_getter(uint8_t term_no);
uint8_t* term_kbf_getter(uint8_t term_no);
uint8_t* term_vmm_getter(uint8_t term_no);
void term_tid_setter(uint8_t term_no, uint8_t term_id);
void term_pid_setter(uint8_t term_no, uint8_t process_id);
uint8_t term_sts_getter(uint8_t term_no);
/* current terminal getter. */
uint8_t get_current_term(void);
