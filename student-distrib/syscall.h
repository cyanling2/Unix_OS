#include "rtc.h"
#include "keyboard.h"
#include "idt.h"
#include "lib.h"
#include "paging.h"
#define MAX_FD 8
#define RES_FD_MIN 2
#define RTYPE 0
#define DTYPE 1
#define FTYPE 2
#define ARGMAXLEN 128 
#define ELFLEN 4
#define MAX_PROC 6
#define PROG_IMGSTART 0x08048000
#define PCB_SIZE 0x2000
#define KERNEL_END 0x800000

#define BOOT_RESERVED   52
#define DENTRY_RESERVED 24
#define FNAME_LENGTH    32
#define FILE_NUM        63
#define NUM_DATABLOCK   1023
#define MAX_FNAME       32
#define BLOCK_SIZE      4096
#define DATA_BLK_IDX_SIZE 32
#define FDA_SIZE        8
#define BOOTHEAD_SIZE   64


typedef struct OPtable{
	int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* filename);
	int32_t (*close)(int32_t fd);
}OPtable_t;

/* heavily used in cp3 */
typedef struct fd{
    OPtable_t* OPtable;
    uint32_t inode;
    uint32_t filepos;
    uint32_t flags;
}fd_t;

typedef struct {
	fd_t fds[FDA_SIZE]; 
	uint32_t parent_ksp;
	uint32_t parent_kbp;
    uint32_t current_ksp;
	uint32_t current_kbp;
	int8_t process_number;
    int8_t parent_process;
	uint8_t argbuf[ARGMAXLEN];
} pcb_t;
void save_parent(pcb_t* p);
// OPtable_t rtc_optable = {rtc_read,rtc_write,rtc_open,rtc_close};
// OPtable_t dir_optable = {readdir,writedir, opendir, closedir};
// OPtable_t file_optable = {readfile,writefile,openfile,closefile};

int32_t open(const uint8_t * filename);
int32_t close(int32_t fd);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);


int32_t getargs(uint8_t* prompt, uint32_t nbytes);
int32_t vidmap(uint8_t ** screen_start);
int32_t p_avail(uint8_t);
int32_t err_open(const uint8_t* filename);
int32_t err_read(int32_t fd, void* buf, int32_t nbytes);
int32_t err_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t err_close(int32_t fd);
void swap_user(uint32_t eip);
int32_t empty(void);
int32_t halt(uint8_t status);
int32_t execute(const uint8_t * prompt);
uint8_t reboot_term(pcb_t*);


typedef struct dentry{
    uint8_t file_name[FNAME_LENGTH];
    uint32_t file_type;
    uint32_t inode;
    uint8_t reserved_dentry[DENTRY_RESERVED];
}dentry_t;

typedef struct bootblock{
    uint32_t num_entries;
    uint32_t num_inodes;
    uint32_t num_datablocks;
    uint8_t  reserved[BOOT_RESERVED];
    dentry_t dir_entries[FILE_NUM];

}bootblock_t;

typedef struct inode{
    uint32_t B_length;
    uint32_t data_block[NUM_DATABLOCK];
}inode_t;

fd_t* get_current_fd(void);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buff, uint32_t length);

int32_t readfile (int32_t fd, void* buf, int32_t nbytes);

int32_t writefile (int32_t fd, const void* buf, int32_t nbytes);

int32_t openfile (const uint8_t* filename);

int32_t closefile (int32_t fd);

int32_t readdir (int32_t fd, void* buf, int32_t nbytes);

int32_t writedir (int32_t fd, const void* buf, int32_t nbytes);

int32_t opendir (const uint8_t* filename);

int32_t closedir (int32_t fd);

void filesys_init(uint32_t* start_addr);

void set_current_process(uint8_t p);

uint8_t fetch_latest_process(uint8_t);

bootblock_t* bootblk;

fd_t fda[FDA_SIZE];

