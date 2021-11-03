#include "lib.h"
#define NUM_DIR 1024
#define NUM_TBL 1024
#define ALIGN_SIZE 4096
#define INIT_DIR 0x02
#define INIT_TBL 0x02
#define FPAGE_SIZE 0x1000
#define RW_PRESENT 0x03
#define KERNEL_START 0x400000
#define KSIZE_MARKER 0x80
#define PAGEDIR_SIZE 0x400000
#define PROG_VIRSTART 0x8000000 //128MB
#define PROG_PHYSTART 0x800000
#define PROG_PAGE_SIZE 0x400000
#define USER_MARKER 0x4
#define VIDMAP_IDX 33
unsigned int page_dir[NUM_DIR] __attribute__((aligned (ALIGN_SIZE)));
unsigned int page_tbl[NUM_TBL] __attribute__((aligned (ALIGN_SIZE)));
unsigned int page_video_tab[NUM_TBL] __attribute__((aligned (ALIGN_SIZE)));
extern void paging_init();
void flush(void);
void pcb_mapping(uint32_t phys, uint32_t virt);
void syscall_video_mapping (uint32_t phys);
void term_video_mapping (uint32_t phys, uint8_t idx);








