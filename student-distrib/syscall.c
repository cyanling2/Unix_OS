#include "syscall.h"

uint8_t elf[ELFLEN] = {0x7f, 'E', 'L', 'F'}; 
uint8_t heads[KBUFF_MAX];
uint8_t p_flags[MAX_PROC * TERM_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t current_process = -1;
OPtable_t rtc_optable = {
    	.read = rtc_read,
    	.write = rtc_write,
    	.open = rtc_open,
    	.close = rtc_close
	};

OPtable_t dir_optable = {
    	.read = readdir,
    	.write = writedir,
    	.open = opendir,
    	.close = closedir
	};

OPtable_t file_optable = {
    	.read = readfile,
    	.write = writefile,
    	.open = openfile,
    	.close = closefile
	};

OPtable_t stdin_optable = {
    	.read = term_read,
    	.write = err_write,
    	.open = term_open,
    	.close = term_close
	};

OPtable_t stdout_optable = {
    	.read = err_read,
    	.write = term_write,
    	.open = term_open,
    	.close = term_close
	};

OPtable_t udf_optable = {
    	.read = err_read,
    	.write = err_write,
    	.open = err_open,
    	.close = err_close
	};

fd_t* get_current_fd(void){
    pcb_t* current = (pcb_t*) (PROG_PHYSTART - (current_process + 1) * PCB_SIZE);
    return current -> fds;
}
/* int32_t open(const uint8_t * filename)
 *open and set the given file as well as its directory in pcb
 * Inputs:filename
 * Outputs: i, the file descriptor
 */
int32_t open(const uint8_t * filename){
    dentry_t local;
    int32_t i;
    pcb_t* current_pcb = (pcb_t*) (PROG_PHYSTART - (current_process + 1) * PCB_SIZE);

    /* if file does not exist. */
    if (read_dentry_by_name(filename, &local)  == -1){
        return -1;
    }

    for (i = RES_FD_MIN; i < MAX_FD; i++){
        if (!current_pcb -> fds[i].flags){
            /* this fd is empty. */
            current_pcb -> fds[i].flags = 1;
            current_pcb -> fds[i].filepos = 0;
            break;
        }  
    }

    /* no available fd for this process. */
    if (i == MAX_FD) return -1;

    switch (local.file_type){
        /* RTC type */
        case RTYPE:{
            /* check whether the file can be rtc_opened */
            // if (!rtc_open(filename)) return -1;
            current_pcb -> fds[i].inode = NULL;
            current_pcb -> fds[i].OPtable = &rtc_optable;
            break;
        }

        /* Directory type */
        case DTYPE:{
            /* check whether the file can be opendired */
            // if (!opendir(filename)) return -1;
            current_pcb -> fds[i].inode = NULL;
            current_pcb -> fds[i].OPtable = &dir_optable;
            break;
        }

        /* File type */
        case FTYPE:{
             /* check whether the file can be openfile-ed */
            // if (!openfile(filename)) return -1;
            current_pcb -> fds[i].inode = local.inode;
            current_pcb -> fds[i].OPtable = &file_optable;
            break;
        }
    }

    /* return back the file descriptor that is open. */
    return i;
}

/* int32_t close(int32_t fd)
 *close the given file
 * Inputs:fd
 * Outputs: status
 */
int32_t close(int32_t fd){
    int32_t status;
    /* sanity check. */
    if (fd < RES_FD_MIN || fd >= MAX_FD){
        return -1;
    }
    
    pcb_t* current_pcb = (pcb_t*) (PROG_PHYSTART - (current_process + 1) * PCB_SIZE);
    
    /* check whether the fd IS open */
    if (!current_pcb -> fds[fd].flags) return -1;
    current_pcb -> fds[fd].flags = 0;

    /* close current process. */
    status = current_pcb -> fds[fd].OPtable -> close(fd);
    return status;
}

/* int32_t read(int32_t fd, void* buf, int32_t nbytes)
 *read the given file
 * Inputs:fd, buf, nbytes
 * Outputs: status
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){
    int32_t status;
    /* sanity check. */
    if (fd < 0 || fd >= MAX_FD){
        return -1;
    }

    if (buf == NULL || nbytes <= 0) {
        return -1;
    }

    pcb_t* current_pcb = (pcb_t*) (PROG_PHYSTART - (current_process + 1) * PCB_SIZE);
    /* check whether the fd IS open */
    if (!current_pcb -> fds[fd].flags) return -1;

    status = current_pcb -> fds[fd].OPtable -> read(fd, buf, nbytes);
    return status;
}

/* int32_t write(int32_t fd, void* buf, int32_t nbytes)
 *write the given file
 * Inputs:fd, buf, nbytes
 * Outputs: status
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    int32_t status;
    /* sanity check. */
    if (fd < 0 || fd >= MAX_FD){
        return -1;
    }

    if (buf == NULL || nbytes <= 0) {
        return -1;
    }

    pcb_t* current_pcb = (pcb_t*) (PROG_PHYSTART - (current_process + 1) * PCB_SIZE);
    /* check whether the fd IS open */
    if (!current_pcb -> fds[fd].flags) return -1;

    status = current_pcb -> fds[fd].OPtable -> write(fd, buf, nbytes);
    return status;
}

/* int32_t execute(const uint8_t * prompt)
 *given whatever in the buf, we first seperate command and the filename, and than check everything valid, map the address and set up pcb
 * and we need to swap user in the end, using iret
 * Inputs:prompt
 * Outputs: none
 */
int32_t execute(const uint8_t * prompt){
    dentry_t local;
    int32_t i, status;
    int32_t begin, cmd_done, file_begin, file_end;
    uint8_t current_char;
    uint8_t cmd[KBUFF_MAX], file[KBUFF_MAX];
    int32_t cmd_index, file_index;
    uint32_t first_ins;
    int8_t prev_process;
    
    /* sanity check. */
    if (prompt == NULL) return -1;

    /* control flags, cmd index and file index. */
    begin = 0;
    cmd_done = 0;
    file_begin = 0;
    file_end = 0;
    cmd_index = 0;
    file_index = 0;

    /* clear cmd and file buffer. */
    for (i = 0; i < KBUFF_MAX; i++){
        cmd[i] = 0;
        file[i] = 0;
    }

    /* parse arguments */
    for (i = 0; i < strlen((int8_t*) prompt); i++){
        current_char = prompt[i];
        if (begin == 0){
            /* if not begun, check current char is null, if yes then continue, else change begin flag. */
            if (current_char != ' ') {
                begin = 1;
                cmd[cmd_index] = current_char;
                cmd_index++;
                continue;
            }
            else {
                continue;
            }
        }
        
        if (cmd_done == 0){
            /* if not done recording yet, check current char is null, if yes then mark done, if no continue. */
            if (current_char != ' ') {
                cmd[cmd_index] = current_char;
                cmd_index++;
                continue;
            }
            else {
                cmd_done = 1;
                continue;
            }
        }

        if (file_begin == 0){
            /* if not started recording args, check current char is null, if yes then continue, if no then mark begin. */
             if (current_char != ' ') {
                file_begin = 1;
                file[file_index] = current_char;
                file_index++;
                continue;
            }
            else {
                continue;
            }
        }

        if (file_end == 0){
            /* if not done recording args, check current char is null, if yes then break, if no then keep recording. */
             if (current_char != ' ') {
                file[file_index] = current_char;
                file_index++;
                continue;
            }
            else {
                file_end = 1;
                break;
            }
        }

   
    }

    /* check whether file exists in dentry */
    if (read_dentry_by_name(cmd, &local) == -1){
        return -1;
    }

    
    /* check whether file is an executable by getting the first data block of the file.*/
    status = read_data(local.inode, 0, heads, KBUFF_MAX);
    if (status == -1) {
        return status;
    } 
    
    
    status = strncmp((int8_t*) heads, (int8_t*) elf, ELFLEN);
    if (status != 0){
        return -1;
    }

    prev_process = current_process;

    /* get current available process. */
    status = p_avail(get_current_term());
    if (status == -1){
        return status;
    }

    
    /* map virtual address at 128MB to the physical memory address at 8MB + pro_num * 4MB */
    pcb_mapping(PROG_PHYSTART + current_process * PROG_PAGE_SIZE, PROG_VIRSTART);

    /* find address of inode that we want. */
    inode_t* target_inode = (inode_t*)(BLOCK_SIZE*(local.inode + 1) + (uint32_t) bootblk);
    
    status = read_data(local.inode, 0, (uint8_t*) PROG_IMGSTART, target_inode -> B_length); 
    
    
    /* fails if read_data returns -1 */
    if (status == -1) {
        return status;
    }

    /* set up program pcb and save all metadata*/
    pcb_t* current_pcb = (pcb_t*) (PROG_PHYSTART - (current_process + 1) * PCB_SIZE);
    
    save_parent(current_pcb);
    current_pcb -> process_number = current_process;
    current_pcb -> parent_process = prev_process;
    /* Note: parent process is the previous process of the current process, if current_process > 0. */
    for (i = 0; i < ARGMAXLEN; i++){
        current_pcb -> argbuf[i] = 0;
    }
    
    strncpy((int8_t*) current_pcb -> argbuf, (int8_t*) file, file_index);
    
    /* initialize fds. */
    for (i = 0; i < MAX_FD; i++){
        current_pcb -> fds[i].OPtable = &udf_optable;
        current_pcb -> fds[i].inode = NULL;
        current_pcb -> fds[i].filepos = 0;
        current_pcb -> fds[i].flags = 0;
    }

    /* define stdin and stdout. */
    current_pcb -> fds[0].OPtable = &stdin_optable;
    current_pcb -> fds[0].flags = 1;
    current_pcb -> fds[1].OPtable = &stdout_optable;
    current_pcb -> fds[1].flags = 1;

    /* esp0: bottom of the program stack.*/
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (uint32_t) (KERNEL_END - current_process * PCB_SIZE - 0x4);
    /* flushing tlb to sort out the paging issue. */
    flush();

    /* calculate eip and design context swap. */
    /* starts at byte 27 - 24 */
    first_ins = (heads[27] << (3 * 8)) | (heads[26] << (2 * 8)) | (heads[25] << 8) | heads[24];
    swap_user(first_ins);
    return 0;
}
/* int32_t halt(uint8_t status)
 * halt a process and return control to the parent process 
 * Inputs: uint8_t status
 * Outputs: None 
 */
int32_t halt(uint8_t status){
    /* to be implemented */
    int32_t i;
    uint32_t parent_kbp, parent_ksp;
    pcb_t* pcb;
    status &= 0xFF;
    pcb = (pcb_t*) (PROG_PHYSTART - (current_process + 1) * PCB_SIZE);
    // printf("current_process_is: %d\n", current_process);
    //check if it is base shell
    if (reboot_term(pcb)){
        current_process = -1;
        p_flags[0 + get_current_term() * MAX_PROC_PT] = 0;
        execute((const uint8_t*)"shell");
        return status;
    }

    //close all fds
    for (i=0; i<MAX_FD; i++){
        close(i);
    }

    
    //fetch parent pointers
    parent_kbp = pcb->parent_kbp;
    parent_ksp = pcb->parent_ksp;

    //set current process to be unused
    p_flags[current_process] = 0;
    //se currently running process back to parent process
    current_process = pcb -> parent_process;
    //restore parent's paging
    pcb_mapping(PROG_PHYSTART + current_process * PROG_PAGE_SIZE, PROG_VIRSTART);
    flush();
    
    // updating current pcb to parent 
    pcb = (pcb_t*) (PROG_PHYSTART - (current_process + 1) * PCB_SIZE);
    //set SS0 to kernel data segment
    tss.ss0 = KERNEL_DS;
    //set ESP0 to base of parent's kernel stack
    tss.esp0 = (uint32_t) (KERNEL_END - current_process * PCB_SIZE - 0x4);

    asm volatile(
                 "mov %0, %%eax;"
                 "mov %1, %%esp;"
                 "mov %2, %%ebp;"
                 "leave;"
                 "ret;"
                 :
                 :"r"((uint32_t)status), "r"(parent_ksp), "r"(parent_kbp)   
                 :"eax"
                 );

    //should never reach here
    return 0;
}
/* int32_t getargs(uint8_t* prompt, uint32_t nbytes)
 * get the argument from argbuf to buf
 * Inputs: uint8_t* prompt, uint32_t nbytes
 * Outputs: 0 for valid, -1 for invalid 
 */
int32_t getargs(uint8_t* prompt, uint32_t nbytes){
	int i, buflen;
	pcb_t* current_pcb = (pcb_t*)(PROG_PHYSTART - (current_process + 1) * PCB_SIZE);
/* sanity check. */
	buflen = strlen((int8_t*)(current_pcb -> argbuf));
	if(buflen <= 0 || buflen > nbytes){
		return -1;
	}
/* fill in the prompt. */
	for(i = 0; i<buflen;i++){
		prompt[i] = current_pcb->argbuf[i];
	}
/* terminal the prompt with a NULL char. */
	prompt[i] = 0;
	return 0;
}

/* int32_t vidmap(uint8_t ** screen_start)
 * set screen start to virtual address and map the paging
 * Inputs: uint8_t ** screen_start
 * Outputs: 0 for valid, -1 for invalid 
 */
int32_t vidmap(uint8_t ** screen_start){
	/* Map Virtual Address to Physical Address */
	if(screen_start == NULL) {
        /* Return FAIL if bad pointer is passed in. */
        return -1;
    }

    if((uint32_t)screen_start < 0x8000000 || (uint32_t)screen_start>=0x8400000) {
        /* Return FAIL if screen_start fails outside the user-level range. */
        return -1;
    }

    /* Map from physical address for video memory to virtual address */
    syscall_video_mapping(VIDEO);

    /* Map the screen_start to the virtual address of 132MB.*/
    *screen_start = (uint8_t*)0x8400000;
     return 0;
}

/* int32_t p_avail(void)
 *set the value of process and flags.
 * Inputs:none
 * Outputs: none
 */

int32_t p_avail(uint8_t term_id){
    int i;
    for (i = term_id * MAX_PROC_PT; i < (term_id + 1) * MAX_PROC_PT; i++){
        if (p_flags[i] == 0){
            current_process = i;
            p_flags[i] = 1;
            return 0;
        }
    }
    return -1;
}

/* void save_parent(pcb_t* p)
 *save parent of current pcb
 * Inputs:p
 * Outputs: none
 */
void save_parent(pcb_t* p){
    asm volatile(
        "movl %%ebp, %%eax;"
        "movl %%esp, %%ebx;"
        /* no input */
        :"=a"(p->parent_kbp), "=b"(p->parent_ksp)
    );
    return;
}

/* void save_parent(pcb_t* p)
 *context switch
 * Inputs:p
 * Outputs: none
 */
void swap_user(uint32_t eip_){
    /* 
        Top --> EIP: First instruction of code segment.
                CS: USER_CS (0x0023)
                ESFLAG: 
                ESP: Bottom of user program stack (128MB + 4MB - 4B).
                SS: USER_DS (0x002B)
    */
    asm volatile(
        "movl $0x002B, %%eax;"           /* 16 bit user ss with 16 bit padding. */
        "pushl %%eax;"
        "movl $0x083FFFFC, %%eax;"       /* 32 bit user program esp */
        "pushl %%eax;"
        "pushfl;"                        /* retrieve flags into eax. */
        "popl %%eax;"
        "orl $0x0200, %%eax;"            /* set interrupt enable flag to true. */
        "pushl %%eax;"                    /* IF: 0x0200, refer to table at https://en.wikipedia.org/wiki/FLAGS_register */
        "movl $0x0023, %%eax;"
        "pushl %%eax;"                   /* 16 bit user cs with 16 bit padding. */
        "movl %0, %%eax;"
        "pushl %%eax;"                   /* 32 bit eip. */
        "iret;"
        :                               
        :"r"(eip_)                        /* no output */
        :"eax"                          /* clobbers eax. */
    );

    return;
}

int32_t err_open(const uint8_t* filename){
    //printf("err open\n");
    return -1;
}
/* Read the RTC */
int32_t err_read(int32_t fd, void* buf, int32_t nbytes){
    //printf("err read\n");
    return -1;
}
/* Write to the RTC */
int32_t err_write(int32_t fd, const void* buf, int32_t nbytes){
    //printf("err write\n");
    return -1;
}
/* Close the RTC */
int32_t err_close(int32_t fd){
    //printf("err close\n");
    return -1;
}

int32_t empty(void){
    /* should not be called, ever! */
    return -1;
}



uint32_t dentry_idx=0;

void filesys_init(uint32_t* start_addr){
	bootblk = (bootblock_t*) start_addr;
}

/*
function:  int32_t read_dentry_by_name
Input: const uint8_t* fname, dentry_t* dentry
output: 0 for valid, -1 for invalid

read the file by reading its filename. 
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    int len = strlen((int8_t*)fname);
    //if fname length is not valid, return -1
    if (len>MAX_FNAME || len<1){
        return -1; 
    }
    int i;
    dentry_t* entries = bootblk->dir_entries;
    for (i = 0; i < bootblk->num_entries; i++){
        uint8_t* cur_fname = entries[i].file_name;
        if (strncmp((int8_t*) cur_fname, (int8_t*) fname, FNAME_LENGTH) == 0){
            //match found
            strncpy((int8_t*) (dentry->file_name), (int8_t*) cur_fname, FNAME_LENGTH);
            dentry->file_type = entries[i].file_type;
            dentry->inode = entries[i].inode;
            return 0;
        }

    }
    return -1;
}

/*
function:  int32_t read_dentry_by_index
Input: const uint32_t index, dentry_t* dentry
output: 0 for valid, -1 for invalid

read the file by reading its index. 
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    
    //check if index is within valid range
    if (index < 0 || index >= bootblk->num_entries){
        return -1;
    }
    // check if it is regular file sicnce the other two should be ignored
    // if(bootblk->dir_entries[index].file_type != 2){
    //     return -1; 
    // }
    // modify this if statement if necessary !!!!!!!!!!!!!!!!!!!!!

    strncpy((int8_t*) (dentry->file_name), (int8_t*) (bootblk->dir_entries[index].file_name), FNAME_LENGTH);
    dentry->file_type = bootblk->dir_entries[index].file_type;
    dentry->inode = bootblk->dir_entries[index].inode;

    return 0; 
    
}
/*
function:  int32_t read_data
Input: uint32_t inode, uint32_t offset, uint8_t* buff, uint32_t length
output: 0 for valid, -1 for invalid
giving the inode, the offset from head and the length of bytes we need to read, we put the data into the buff and return 0 if valid
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buff, uint32_t length){
    // check if inode is valid
    if(inode <0 || inode >=bootblk->num_inodes || buff==NULL || offset < 0 || length < 0){
        return -1;
    }
    //find the addr of the inode we want
    inode_t* target_inode = (inode_t*)(BLOCK_SIZE*(inode+1)+(uint32_t)bootblk);
    //do nothing if length==0 or EOF is reached
    if (length==0 || offset>=target_inode->B_length){
        return 0;
    }
    uint32_t data_blk_idx_in_inode, data_blk_idx_in_sysfile, byte_index;
    uint8_t* first_data_blk = (uint8_t*)(BLOCK_SIZE*(1+(bootblk->num_inodes))+(uint32_t)bootblk);
    uint8_t* data_blk_ptr;
    uint32_t i;
    int32_t counter=0;
    for (i=offset; i<offset+length; i++){
        if (i>=target_inode->B_length){
            break;
        }
        data_blk_idx_in_inode = i/BLOCK_SIZE;
        byte_index = i % BLOCK_SIZE;
        if (data_blk_idx_in_inode<0 || data_blk_idx_in_inode>=NUM_DATABLOCK){
            return -1;
        }
        data_blk_idx_in_sysfile = target_inode->data_block[data_blk_idx_in_inode];
        if(data_blk_idx_in_sysfile>=bootblk->num_datablocks || data_blk_idx_in_sysfile<0){
            return -1;
        }
        data_blk_ptr =(uint8_t*)((uint32_t)(first_data_blk)+BLOCK_SIZE*data_blk_idx_in_sysfile+byte_index);
        counter++;
        buff[i-offset] = *data_blk_ptr;
    }
    
    return counter;
}

/*
function: int32_t openfile
Input: filename
Output: 0

open the file
*/
int32_t openfile (const uint8_t* filename){
    return 0;
}

/*
function: int32_t writefile
Input: int32_t fd, void* buf, int32_t nbytes
Output: -1

write the file
*/
int32_t writefile (int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/*
function: int32_t openfile
Input: fd
Output: 0

close the file
*/
int32_t closefile (int32_t fd){
    return 0;
}

/*
function: int32_t readfile
Input: int32_t fd, void* buf, int32_t nbytes
Output: -1 or num_bytes_read

read the file and return the number of bytes read
*/
int32_t readfile (int32_t fd, void* buf, int32_t nbytes){
    if (fd<2 || fd>7){
        return -1;
    }
    fd_t* fds = get_current_fd();
    int num_bytes_read = read_data(fds[fd].inode, fds[fd].filepos, (uint8_t*)buf, nbytes);
    fds[fd].filepos+=num_bytes_read;
    return num_bytes_read;
}
/*
function: int32_t opendir
Input: filename
Output: -1

open the dir
*/
int32_t opendir (const uint8_t* filename){
    return 0;
}
/*
function: int32_t writedir
Input: int32_t fd, void* buf, int32_t nbytes
Output: -1

write the dir
*/
int32_t writedir (int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}
/*
function: int32_t closedir
Input: int32_t fd
Output: 0

close the dir
*/
int32_t closedir (int32_t fd){
    return 0;
}
/*
function: int32_t read dir
Input: int32_t fd, void* buf, int32_t nbytes
Output: 0 or namelen

read the dir, return namelen if valid
*/
int32_t readdir (int32_t fd, void* buf, int32_t nbytes){
    dentry_t dentry;
    if(read_dentry_by_index(dentry_idx, &dentry)==-1){
        //reach the end of directory
        dentry_idx=0;
        return 0;
    }
    uint32_t namelen = strlen((int8_t*)dentry.file_name);
    if (namelen > FNAME_LENGTH){
        namelen = FNAME_LENGTH;
    }
    strncpy((int8_t*)buf, (int8_t*)dentry.file_name, namelen);
    dentry_idx++;
    return namelen;
}
/*
function: uint8_t reboot_term(pcb_t* pcb)
Input: pcb_t* pcb
Output: None
Reboot the terminal
*/

uint8_t reboot_term(pcb_t* pcb){
    uint8_t result = ((pcb -> parent_process) == -1 && (pcb -> process_number) == 0) ||
            ((pcb -> parent_process) == -1 && (pcb -> process_number) == 4) ||
            ((pcb -> parent_process) == -1 && (pcb -> process_number) == 8);
    return result;
}
/*
function: void set_current_process(uint8_t p)
Input: uint8_t p
Output: None
Set the current process to p
*/

void set_current_process(uint8_t p){
    current_process = p;
    return;
}
/*
function: uint8_t fetch_latest_process(uint8_t term_id)
Input: uint8_t term_id
Output:  uint8_t
Getting the latest processed process id 
*/
uint8_t fetch_latest_process(uint8_t term_id){
    int i;
    for (i = (term_id + 1) * MAX_PROC_PT - 1; i >= term_id * MAX_PROC_PT; i--){
        if (p_flags[i] == 1){
            return i;
        }
    }
    return -1;
}
