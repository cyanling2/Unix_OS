#include "paging.h"

/* void keyboard_init()
 *initialize paging
 * Inputs: None
 * Outputs: None
 */

void paging_init(){
    /* for each entry in the page directory, 
       we grant them R/W status but mark them with not present.*/
    int index;
    
    for (index = 0; index < NUM_DIR; index++){
        page_dir[index] = INIT_DIR;
    }
   
    /* for each entry in the table directory,
       each page is spaced 4KB apart, 
       we grant them R/W status but mark them with not present. */
    for (index = 0; index < NUM_TBL; index++){
        page_tbl[index] = (index * FPAGE_SIZE) + INIT_TBL;
    }

    /* place the page table addr into the first directory entry, and mark as present. */
    page_dir[0] = ((unsigned int) page_tbl) | RW_PRESENT;

    /* place the 4MB kernel start address into the second entry, mark present and mark size. */
    page_dir[1] = KERNEL_START | KSIZE_MARKER | RW_PRESENT;

    /* index video memory at the first 20 most significant bits of video memory address.*/
    page_tbl[0xB8] = page_tbl[0xB8] | RW_PRESENT;

    /* set CR registers. */
    /* code mirrored from https://wiki.osdev.org/Paging */
    asm volatile(
                "movl %0, %%eax;"           /* load the page directory address into CR3 register */
                "movl %%eax, %%cr3;"        
                "movl %%cr4, %%eax;"
                "orl  $0x00000010, %%eax;"  /* enables PSE. */
                "movl %%eax, %%cr4;"
                "movl %%cr0, %%eax;"
                "orl  $0x80000001, %%eax;"  /* enable paging and protection bits in cr0 register */
                "movl %%eax, %%cr0;"
                :                          
                :"r"(page_dir)              /* read page_dir as input. */
                :"%eax"                     /* clobbered register */
    );

    printf("writing registers done!\n");
    return;
}

/* void pcb_mapping(uint32_t phys, uint32_t virt)
 *map the virtual address to the physical address for correct mapping for pcb
 * Inputs:phys, virt
 * Outputs: None
 */
void pcb_mapping(uint32_t phys, uint32_t virt){
    /* check index that the virt address is at. */
    int32_t index =  virt / PAGEDIR_SIZE;

    /* mark present, rw access, size as 4MB and userspace */
    page_dir[index] = phys | (RW_PRESENT | KSIZE_MARKER | USER_MARKER);

    /* flush TLB to activate the mapping */
    flush();
}

/* void syscall_video_mapping (uint32_t physical_address)
 * Inputs: physical address
 * Return Value: none
 * Function: Map from virtual address to physical address for our video memory */
void syscall_video_mapping (uint32_t phys)
{   

    /* Make the page_dir points the page table points to our video memory */
    page_dir[VIDMAP_IDX] = ((unsigned int)page_video_tab) | RW_PRESENT | USER_MARKER;
    page_video_tab[0] = phys | RW_PRESENT | USER_MARKER;

    /* Flush the tlb */
    flush();
}

void term_video_mapping (uint32_t phys, uint8_t idx)
{   
    /* Make the page_dir points the page table points to our video memory */
    page_dir[0] = ((unsigned int)page_tbl) | RW_PRESENT;
    page_tbl[0xB9 + idx] = phys | RW_PRESENT;

    /* Flush the tlb */
    flush();
}

/* void flush(void)
 *making sure that we won't get page fault after map pcb
 * Inputs:none
 * Outputs: None
 */
void flush(void)
{
    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"    
        :                       /* no output */
        :                       /* no input  */
        :"eax"                  /* clobbered: %%eax */
    );
}




