#include "idt.h"
#include "i8259.h"
#include "keyboard.h"
#include "tests.h"
#include "scheduling.h"
typedef struct{
    uint8_t term_id;
    uint8_t process_id;
    uint8_t status;
    uint32_t cursor[CURSOR_NUM];
    /* index 0 -- cursor x */
    /* index 1 -- cursor y */
    volatile uint8_t key_buf[KBUFF_MAX];
    uint8_t* video_mem;
}term_t;

uint8_t current_term;
term_t terms[TERM_MAX];
/* term getters. */

/* 
uint8_t term_tid_getter(uint8_t term_no)
input: uint8_t term_no
output: none
effects: get the term_id of a given terminal
*/
uint8_t term_tid_getter(uint8_t term_no){
    return terms[term_no].term_id;
} 

/* 
uint8_t term_pid_getter(uint8_t term_no)
input: uint8_t term_no
output: none
effects: get the process_id of a given terminal
*/
uint8_t term_pid_getter(uint8_t term_no){
    return terms[term_no].process_id;
}

/* 
uint8_t term_sts_getter(uint8_t term_no)
input: uint8_t term_no
output: none
effects: get the status of a given terminal
*/
uint8_t term_sts_getter(uint8_t term_no){
    return terms[term_no].status;
}

/* 
uint8_t term_cursor_getter(uint8_t term_no)
input: uint8_t term_no
output: none
effects: get the cursor of a given terminal
*/
uint32_t* term_cursor_getter(uint8_t term_no){
    return terms[term_no].cursor;
} 

/* 
uint8_t term_kbf_getter(uint8_t term_no)
input: uint8_t term_no
output: none
effects: get the key_buffer of a given terminal
*/
uint8_t* term_kbf_getter(uint8_t term_no){
    return (uint8_t*) terms[term_no].key_buf;
}

/* 
uint8_t term_vmm_getter(uint8_t term_no)
input: uint8_t term_no
output: none
effects: get the video memory address of a given terminal
*/
uint8_t* term_vmm_getter(uint8_t term_no){
    return (uint8_t*) terms[term_no].video_mem;
}
/* term setters */
/* 
uint8_t term_tid_setter(uint8_t term_no)
input: uint8_t term_no
output: none
effects: set the term_id of a given terminal
*/
void term_tid_setter(uint8_t term_no, uint8_t term_id){
    terms[term_no].term_id = term_id;
} 

/* 
uint8_t term_pid_setter(uint8_t term_no)
input: uint8_t term_no
output: none
effects: set the process_id of a given terminal
*/
void term_pid_setter(uint8_t term_no, uint8_t process_id){
    terms[term_no].process_id = process_id;
}

/* 
uint8_t term_sts_setter(uint8_t term_no)
input: uint8_t term_no
output: none
effects: set the status of a given terminal
*/
void term_sts_setter(uint8_t term_no, uint8_t status){
    terms[term_no].status = status;
}


uint8_t key_map[KEY_MAP_SIZE];
uint8_t key_buf[KBUFF_MAX];
uint8_t last_buf[KBUFF_MAX];
int number_of_key_presses;
int caps = 0;
int shift = 0;
int ctrl = 0;
int alt = 0;
uint8_t termkey_buf[TKBUFF_MAX];
int32_t tkindex;
int enter_received;
int32_t len_buf = 0;
int32_t last_len = 0;
int get_number_of_key_presses(){
    return number_of_key_presses;
}

/* 
static void clear_buf(void)
input: none
output: none
effects: clears the key buffer after hitting enter.
*/
static void clear_buf(void){
    int i;
    for (i = 0; i < KBUFF_MAX; i++){
        key_buf[i] = 0;//clear the key_buf to 0
    }
    len_buf = 0;
}

/* 
static void copy_buf(void)
input: none
output: none
effects: copies the old key buffer before clearing the new one.
*/
static void copy_buf(void){
    int i;
    for (i = 0; i < KBUFF_MAX; i++){
        last_buf[i] = key_buf[i];//copy key_buf to last_buf
    }
    last_len = len_buf;
}

/* 
void caps_init(void)
input: none
output: none
effects: initialize capital cases keymaps.
*/
void caps_init(void){
    int i;
    for (i=0; i<KEY_MAP_SIZE; i++){
        caps_map[i] = 0; //don't care
    }
    caps_map[0x02] = '1';
    caps_map[0x03] = '2';
    caps_map[0x04] = '3';
    caps_map[0x05] = '4';
    caps_map[0x06] = '5';
    caps_map[0x07] = '6';
    caps_map[0x08] = '7';
    caps_map[0x09] = '8';
    caps_map[0x0A] = '9';
    caps_map[0x0B] = '0';
    caps_map[0x0C] = '-';
    caps_map[0x0D] = '=';
    caps_map[0x10] = 'Q';
    caps_map[0x11] = 'W';
    caps_map[0x12] = 'E';
    caps_map[0x13] = 'R';
    caps_map[0x14] = 'T';
    caps_map[0x15] = 'Y';
    caps_map[0x16] = 'U';
    caps_map[0x17] = 'I';
    caps_map[0x18] = 'O';
    caps_map[0x19] = 'P';
    caps_map[0x1A] = '[';
    caps_map[0x1B] = ']';
    caps_map[0x1E] = 'A';
    caps_map[0x1F] = 'S';
    caps_map[0x20] = 'D';
    caps_map[0x21] = 'F';
    caps_map[0x22] = 'G';
    caps_map[0x23] = 'H';
    caps_map[0x24] = 'J';
    caps_map[0x25] = 'K';
    caps_map[0x26] = 'L';
    caps_map[0x27] = ';';
    caps_map[0x28] = '\'';
    caps_map[0x29] = '`';
    caps_map[0x2B] = '\\';
    caps_map[0x2C] = 'Z';
    caps_map[0x2D] = 'X';
    caps_map[0x2E] = 'C';
    caps_map[0x2F] = 'V';
    caps_map[0x30] = 'B';
    caps_map[0x31] = 'N';
    caps_map[0x32] = 'M';
    caps_map[0x33] = ',';
    caps_map[0x34] = '.';
    caps_map[0x35] = '/';
    caps_map[0x37] = '*';
    caps_map[0x39] = ' ';
    caps_map[ENTER] = '\n';
    caps_map[BACKSPACE] = '\0';
    caps_map[TAB] =' ';
}

/* 
void sh_init(void)
input: none
output: none
effects: initialize keymaps for when the shift key is pressed.
*/
void sh_init(void){
    int i;
    for (i=0; i<KEY_MAP_SIZE; i++){
        sh_map[i] = 0; //don't care
    }
    sh_map[0x02] = '!';
    sh_map[0x03] = '@';
    sh_map[0x04] = '#';
    sh_map[0x05] = '$';
    sh_map[0x06] = '%';
    sh_map[0x07] = '^';
    sh_map[0x08] = '&';
    sh_map[0x09] = '*';
    sh_map[0x0A] = '(';
    sh_map[0x0B] = ')';
    sh_map[0x0C] = '_';
    sh_map[0x0D] = '+';
    sh_map[0x10] = 'Q';
    sh_map[0x11] = 'W';
    sh_map[0x12] = 'E';
    sh_map[0x13] = 'R';
    sh_map[0x14] = 'T';
    sh_map[0x15] = 'Y';
    sh_map[0x16] = 'U';
    sh_map[0x17] = 'I';
    sh_map[0x18] = 'O';
    sh_map[0x19] = 'P';
    sh_map[0x1A] = '{';
    sh_map[0x1B] = '}';
    sh_map[0x1E] = 'A';
    sh_map[0x1F] = 'S';
    sh_map[0x20] = 'D';
    sh_map[0x21] = 'F';
    sh_map[0x22] = 'G';
    sh_map[0x23] = 'H';
    sh_map[0x24] = 'J';
    sh_map[0x25] = 'K';
    sh_map[0x26] = 'L';
    sh_map[0x27] = ':';
    sh_map[0x28] = '"';
    sh_map[0x29] = '~';
    sh_map[0x2B] = '|';
    sh_map[0x2C] = 'Z';
    sh_map[0x2D] = 'X';
    sh_map[0x2E] = 'C';
    sh_map[0x2F] = 'V';
    sh_map[0x30] = 'B';
    sh_map[0x31] = 'N';
    sh_map[0x32] = 'M';
    sh_map[0x33] = '<';
    sh_map[0x34] = '>';
    sh_map[0x35] = '?';
    sh_map[0x37] = '*';
    sh_map[0x39] = ' ';
    sh_map[ENTER] = '\n';
    sh_map[BACKSPACE] = '\0';
    sh_map[TAB] =' ';
}

/* void keyboard_init()
 * initialize keyboard with no caps lock and shift
 * Inputs: None
 * Outputs: None
 */
void keyboard_init(){
    disable_irq(KEYBOARD_IRQ);
    int i;
    for (i=0; i<KEY_MAP_SIZE; i++){
        key_map[i] = 0; //don't care
    }
    key_map[0x02] = '1';
    key_map[0x03] = '2';
    key_map[0x04] = '3';
    key_map[0x05] = '4';
    key_map[0x06] = '5';
    key_map[0x07] = '6';
    key_map[0x08] = '7';
    key_map[0x09] = '8';
    key_map[0x0A] = '9';
    key_map[0x0B] = '0';
    key_map[0x0C] = '-';
    key_map[0x0D] = '=';
    key_map[0x10] = 'q';
    key_map[0x11] = 'w';
    key_map[0x12] = 'e';
    key_map[0x13] = 'r';
    key_map[0x14] = 't';
    key_map[0x15] = 'y';
    key_map[0x16] = 'u';
    key_map[0x17] = 'i';
    key_map[0x18] = 'o';
    key_map[0x19] = 'p';
    key_map[0x1A] = '[';
    key_map[0x1B] = ']';
    key_map[0x1E] = 'a';
    key_map[0x1F] = 's';
    key_map[0x20] = 'd';
    key_map[0x21] = 'f';
    key_map[0x22] = 'g';
    key_map[0x23] = 'h';
    key_map[0x24] = 'j';
    key_map[0x25] = 'k';
    key_map[0x26] = 'l';
    key_map[0x27] = ';';
    key_map[0x28] = '\'';
    key_map[0x29] = '`';
    key_map[0x2B] = '\\';
    key_map[0x2C] = 'z';
    key_map[0x2D] = 'x';
    key_map[0x2E] = 'c';
    key_map[0x2F] = 'v';
    key_map[0x30] = 'b';
    key_map[0x31] = 'n';
    key_map[0x32] = 'm';
    key_map[0x33] = ',';
    key_map[0x34] = '.';
    key_map[0x35] = '/';
    key_map[0x37] = '*';
    key_map[0x39] = ' ';
    key_map[ENTER] = '\n';
    key_map[BACKSPACE] = '\0';
    key_map[TAB] =' ';
    // unmask keyboard irq on PIC
    len_buf = 0;
    for (i = 0; i < KBUFF_MAX; i++){
        key_buf[i] = 0;
    }
    caps_init();
    sh_init();
    enable_irq(KEYBOARD_IRQ);
}

/* 
static int temp_check(uint8_t key)
input: uint8_t key: keycode pressed
output: 1 -> if legal
        0 -> otherwise
effect: check whether a keypress is legal.
*/
static int temp_check(uint8_t key){
    if (key == ENTER || key == BACKSPACE) return 1;
    if (key == CAPS_ON || key == CAPS_OFF) return 1;
    if (key == RSHIFT_ON || key == RSHIFT_OFF) return 1;
    if (key == LSHIFT_ON || key == LSHIFT_OFF) return 1;
    if (key == CTRL_ON || key == CTRL_OFF) return 1;
    /* the following is all legal keycode ranges. */
    if (key >= 0x02 && key <= 0x0D) return 1;
    if (key >= 0x10 && key <= 0x1B) return 1;
    if (key >= 0x1E && key <= 0x1F) return 1;
    if (key >= 0x20 && key <= 0x29) return 1;
    if (key >= 0x2B && key <= 0x2F) return 1;
    if (key >= 0x30 && key <= 0x35) return 1;
    if (key == 0x37 || key == 0x39) return 1;
    if (key == TAB || key == ALT_ON || key == ALT_OFF) return 1;
    if (key == F1 || key == F2 || key == F3) return 1;
    return 0;
}

/* void keyboard_interrupt_handler()
 * read from keyport, get the key from key map, and send the interrupt. 
 * Inputs: None
 * Outputs: None
 */
void keyboard_interrupt_handler(){
    cli();
    uint8_t input_keycode = inb(KEY_PORT); //load the keycode
    uint8_t input_key = key_map[input_keycode];  // load the key from reading the key_map
    /* temp check, to be removed in later checkpoints .*/
    if (temp_check(input_keycode) == 0){
        sti();
        send_eoi(KEYBOARD_IRQ);
    }
    
    /* turn caps flag on whenver the caps lock is pressed. */
    if (input_keycode == CAPS_ON) {
        caps ^= 1;
    }

    /* turn shift flag on whenver the caps lock is pressed. */
    if ((input_keycode == RSHIFT_ON) || (input_keycode == LSHIFT_ON)){
        shift = 1;
    }

    /* turn shift flag on whenver the caps lock is released. */
    if ((input_keycode == RSHIFT_OFF) || (input_keycode == LSHIFT_OFF)){
        shift = 0;
    }

    /* reserve space for alt, will be implemented by tonight. */
    if ((input_keycode == ALT_ON)){
        alt = 1;
    }

    /* turn control flag off whenver the control key is released. */
    if ((input_keycode == ALT_OFF)) {
        alt = 0;
    }

    /* turn control flag on whenver the control key is pressed. */
    if ((input_keycode == CTRL_ON)){
        ctrl = 1;
    }

    /* turn control flag off whenver the control key is released. */
    if ((input_keycode == CTRL_OFF)) {
        ctrl = 0;
    }

    switch (input_keycode){
        case ENTER: {
            /* set cursor to new line */
            key_buf[len_buf] = input_key;
            len_buf += 1;
	        enter_received = 1;
            enter();
            copy_buf();
            clear_buf();
            break;
        }

        case BACKSPACE: {
            /* clear digit, set last character to 0. */
            if (len_buf > 0){
                key_buf[len_buf] = 0;
                len_buf -= 1;
                clear_digit(len_buf);
            }
            break;
        }

        case F1: {
            if (alt == 1){
                send_eoi(KEYBOARD_IRQ);
                term_swap(current_term, 0);
            }
            break;
        }

        case F2: {
            if (alt == 1){
                send_eoi(KEYBOARD_IRQ);
                term_swap(current_term, 1);
            }
            break;
        }

        case F3: {
            if (alt == 1){
                send_eoi(KEYBOARD_IRQ);
                term_swap(current_term, 2);
            }
            break;
        }

        default: {
            if (input_key != 0 && len_buf < (KBUFF_MAX - 1)){
                /* switching different keymaps based on flag */
                if (caps == 1){
                    input_key = caps_map[input_keycode];
                }
                if (shift == 1){
                    input_key = sh_map[input_keycode];
                }
                if (ctrl == 1) {
                    if (input_key == 'L' || input_key == 'l'){
                        clear();
                        // clear_buf();
                        break;
                    }
                }
                putc(input_key);
                key_buf[len_buf] = input_key;
                len_buf += 1;
            }
            
        }
    }
    send_eoi(KEYBOARD_IRQ); //send to end the interrupt
    sti();
}


/* 
void term_init(void)
input: None
output: None
effect: initialize the termkey_buf, (no actual use in cp2)
*/
void term_init(void){
    int i, j;
    tkindex = 0;
    for (i = 0; i < TKBUFF_MAX; i++){
        termkey_buf[i] = 0;
    }

    /* do some preparation work by initializing the terms array. */
    for (i = 0; i < TERM_MAX; i++){
        terms[i].term_id = i;
        terms[i].process_id = 0;
        terms[i].cursor[0] = 0;
        terms[i].cursor[1] = 0;
        terms[i].status = 0;

        for (j = 0; j < TKBUFF_MAX; j++){
            terms[i].key_buf[j] = 0;
        }
        term_video_mapping(0xB9000 + 0x1000 * i, i);
        /* initialize video memory for each terminal. */
        terms[i].video_mem= (uint8_t*) (0xB9000 + 0x1000 * i);
        for (j = 0; j < ROWS * COLS; j++){
            *(uint8_t *)(terms[i].video_mem + (j << 1)) = ' ';
            /* we choose to color each terminal differently, hahahaha. */
            switch (i){
                case 0: {
                    /* white background, black text. */
                    *(uint8_t *)(terms[i].video_mem + (j << 1) + 1) = ATTVID1;
                    break;
                }
                case 1: {
                    /* black background, white text. */
                    *(uint8_t *)(terms[i].video_mem + (j << 1) + 1) = ATTVID2;
                    break;
                }
                case 2: {
                    /* white background, blue text.*/
                    *(uint8_t *)(terms[i].video_mem + (j << 1) + 1) = ATTVID3;
                    break;
                }
            }
        }

    } 
    /* execute the shell */
    current_term = 0;
    // terms[current_term].status = 1;
    // execute((uint8_t*)"shell");
}

/* -1: fails; 0: currently running; 1: new */ 
int term_start(uint8_t term_id){
    if (term_id >= TERM_MAX){
        return -1;
    }

    /* if current process is running. */
    if (terms[term_id].status == 1){
        return 0;
    }
    
    /* Otherwise, we need to create a new process. */
    terms[term_id].status = 1;
    current_term = term_id;
    memcpy((uint8_t*)VIDEO, terms[term_id].video_mem, 2 * ROWS * COLS);
    set_cursor(terms[term_id].cursor[0], terms[term_id].cursor[1]);
    set_current_process(-1);
    execute((uint8_t*)"shell");
    return 1;
}

/* 
void term_swap(uint8_t src, uint8_t dst)
input: uint8_t src, uint8_t dst
output: none
effects: save the source terminal and restore the destination terminal
*/
void term_swap(uint8_t src, uint8_t dst){
    if (src == dst){
        return;
    }
    term_save(src);
    term_restore(dst);
    return;
}

/* 
void term_save(uint8_t term_id)
input: uint8_t term_id
output: none
effects: save the key_buf and cursor and video mem to the given terminal 
*/
void term_save(uint8_t term_id){
    int i;
    for (i = 0; i < KBUFF_MAX; i++){
        terms[term_id].key_buf[i] = key_buf[i];
    }
    terms[term_id].cursor[0] = get_cursor_x();
    terms[term_id].cursor[1] = get_cursor_y();
    /* copy B8... to B9... + idx */
    memcpy(terms[term_id].video_mem, (uint8_t*)VIDEO, 2 * ROWS * COLS);
    return;
}

/* 
void term_restore(uint8_t term_id)
input: uint8_t term_id
output: none
effects: restore the key_buf and cursor and video mem of the given terminal, getting from previous saved infomation
*/
void term_restore(uint8_t term_id){
    int i;
    if (terms[term_id].status == 0){
        term_start(term_id);
        return;
    }
    for (i = 0; i < KBUFF_MAX; i++){
        key_buf[i] = terms[term_id].key_buf[i]; 
    }
    memcpy((uint8_t*)VIDEO, terms[term_id].video_mem, 2 * ROWS * COLS);
    set_cursor(terms[term_id].cursor[0], terms[term_id].cursor[1]);
    current_term = term_id;
    set_current_process(fetch_latest_process(term_id));
    return;
}

/* 
    int term_open(const uint8_t * filename)
    input: filename
    output: None
    effect: None, (no actual use in cp2)
*/
int term_open(const uint8_t * filename){
    /* does nothing for cp2. */
    return 0;
}

/* 
int term_open(int32_t fd)
input: fd
output: None
effect: None, (no actual use in cp2)
*/
int term_close(int32_t fd){
    /* does nothing for cp2. */
    return 0;
}

/* 
int term_read(int32_t fd, uint8_t buf[], unsigned int length)
input: fd -> no meaning here in cp2
       buf -> the destination for the keyboard buffer data.
       length -> no actual use here in cp2.
output: None
effect: Reads the keyboard buffer whenever enter key is pressed.
*/
int term_read(int32_t fd, void* buf, int32_t length){
    /* sanity check. */
    if (buf == NULL) return -1;
    /* polling the enter interrupt. */
	enter_received = 0;
	while (1)
	{
		if (enter_received == 1)
			break;
	}
    uint8_t* temp_buf = (uint8_t*) buf;
    /* read the keyboard buffer. */
    int i;
    for (i = 0; i < last_len; i++){
        temp_buf[i] = last_buf[i];
    }
	return last_len;
}

/* 
int term_write(uint8_t buf[], unsigned int length)
input:  buf -> the source for the text to be written
       length -> no actual use here in cp2.
output: None
effect: write the data specified by the buffer.
*/
int term_write(int32_t fd, const void* buf, int32_t length){
    /* failure: invalid buf */
    if (!buf || length < 0){
        return -1;
    }

    uint8_t* temp_buf = (uint8_t*) buf;
    /* iterate over char in buf and put it on screen */
    int32_t idx;
    for (idx = 0; idx < length; idx++){
        //If nbytes is bigger than len(buf)
        if (!temp_buf[idx]) continue;
        // display
        putc(temp_buf[idx]);
    }
    return length;
}

/* 
uint8_t get_current_term(void
input:  none
output: current_term
effect: get the terminal index
*/
uint8_t get_current_term(void){
    return current_term;
}
