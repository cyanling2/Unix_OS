#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
//#include "filesys.h"
 #include "syscall.h"
#define PASS 1
#define FAIL 0
#define READ_BUF_SIZE 1024

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

int cp1_rtc_flag;
int cp1_keyboard_flag;
int cp1_rtc_test_flag(){
	return cp1_rtc_flag;
}
int cp1_keyboard_test_flag(){
	return cp1_keyboard_flag;
}
/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
//changed to 10
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here


/* div0 Test
 * test divide by 0, should not pass
 * Inputs: None
 * Outputs: PASS/EXCEPTION
 */
int div0_test(){
	TEST_HEADER;
	int a = 0;
	int b = 1/a;
	b = 1;
	return PASS;
}

/* deref Test
 * NULL test, should fail, page fault exception.
 * Inputs: None
 * Outputs: PASS/EXCEPTION
 */
int deref_test(){
	TEST_HEADER;
	/* page fault. */
	int* c = NULL;
	int d = *c;
	d = d;
	return PASS;
}

/* paging-deref Test 1
 * derefence memorythat does not exist test, should fail, page fault exception.
 * Inputs: None
 * Outputs: PASS/EXCEPTION
 */
int paging_deref_test1(){
	TEST_HEADER;

	/* derefence memory that should not exist. */
	char val = *((char *) 0xB7FFF);
	val = 0;
	return PASS;
}

/* paging-deref Test 2
 * test B8000,first page test, should pass
 * Inputs: None
 * Outputs: PASS/EXCEPTION
 */
/* this test should pass since B8000 is marked present. */
int paging_deref_test2()
{
	TEST_HEADER;

	/* derefence memory that should exist. */
	char val = *((char *) 0xB8000);
	val = 0;
	return PASS;
}

/* paging-deref Test 3
 * second page test, should pass
 * Inputs: None
 * Outputs: PASS/EXCEPTION
 */
int paging_deref_test3(){
	TEST_HEADER;

	/* dereference memory that should exist, (in the second page and present.) */
	char val = *((char * ) 0x480000);
	val = 0;
	return PASS;
}

/* paging-deref Test 4
 * first directory test, should fail
 * Inputs: None
 * Outputs: PASS/EXCEPTION
 */
int paging_deref_test4(){
	TEST_HEADER;

	/* dereference memory that should NOT exist, (in the first directory and present.) */
	char val = *((char * ) 0x380000);
	val = 0;
	return PASS;
}

/* paging-deref Test 5
 * last entry test, should pass.
 * Inputs: None
 * Outputs: PASS/EXCEPTION
 */
int paging_deref_test5(){
	TEST_HEADER;
	/* last entry in the video memory table, should pass.*/
	char val = *((char *) 0xB8FFF);
	val = 0;
	return PASS;
}

/* paging-deref Test 6
 * irst entry in the table RIGHT AFTER video memory table, should not pass
 * Inputs: None
 * Outputs: PASS/EXCEPTION
 */
int paging_deref_test6(){
	TEST_HEADER;

	/* first entry in the table RIGHT AFTER video memory table, should not pass.*/
	char val = *((char *) 0xB9000);
	val = 0;
	return PASS;
}

/* paging-deref Test 6
 * Unpaged memory address, should not pass
 * Inputs: None
 * Outputs: PASS/EXCEPTION
 */
int paging_deref_test7(){
	TEST_HEADER;

	/* Unpaged memory address, should not pass.*/
	char val = *((char *) 0x1200000);
	val = 0;
	return PASS;
}
/* Checkpoint 2 tests */

/* rtc test
 * test the functionality of rtc. call rtc_open first to set the freq to 2HZ, and then, by incrementing buf, 
   we write different freq in it and we can see the speed of "1" displayedd on the terminal is changing.
 * Inputs: None
 * Outputs: PASS
 */
int rtc_test()
{
	TEST_HEADER;
	int count = 0;
	int* buf;//initiate the stating frequency
	*buf = 2;
	rtc_open(0);//open the rtc, freq = 2hz
	while(count<240){
		if(count % 30 == 0){
			*buf *= 2;
			rtc_write(0, buf, 4); //we input different frequency here
		}
		if(rtc_read(0,0,0) == 0){//when interrupted
			count++;
			printf("1", count);
		}
	}
	rtc_close(0);
	return PASS;
}


/* term test
 * test the functionality of terminal. call term_open first, and we set the buf to 0 first. Then, we call term_read and term_write.
   we can type whatever we want on the screen, and after we hit enter, we can see the text we typed would display on the screen again.
 * Inputs: None
 * Outputs: PASS
 */
int term_test(){
	TEST_HEADER;
	term_open(0);// open the terminal
	uint8_t buf[128];
	int i;
	while (1){
		for (i = 0; i<128; i++){// initiate the buf to 0 first
			buf[i] = 0;
		}
		term_read(0, buf, 128);// call read. it will return when enter is hit
		if (buf[0]=='l' && buf[1]=='s' && buf[2]=='\n'){
			//execute ls cmd
			printf("list directory...\n");
			uint8_t buff[READ_BUF_SIZE];
			int32_t cnt;
			while (0 != (cnt = readdir(0, buff, 0))){
				if (cnt==-1){
					return FAIL;
				}
				term_write(2, buff, cnt);
				printf("\n");

			}
		}
		else{
			printf("typed input is: ");
			term_write(2, buf, 128);//write whatever in the buf to the screen
		}
		
	}
	term_close(0);
	return PASS;
}

/* read_dentry_by_name_test
 * test the functionality of reading valid file by name. if read_dentry_by_name return 0, we know that we read the name of file, return pass
 * Inputs: uint8_t* file_name
 * Outputs: PASS
 */
int read_dentry_by_name_test(uint8_t* file_name){
	TEST_HEADER;
	dentry_t dentry;
	if (read_dentry_by_name(file_name, &dentry) == 0){
		return PASS;
	}
	else{
		return FAIL;
	}

}

/* read_dentry_by_index_test
 * test the functionality of reading valid file by index. if read_dentry_by_index return 0, we know that we read the index of file, return pass
 * Inputs: uint32_t index
 * Outputs: PASS
 */
int read_dentry_by_index_test(uint32_t index){
	TEST_HEADER;
	dentry_t dentry;
	if (read_dentry_by_index(index, &dentry) == 0){
		return PASS;
	}
	else{
		return FAIL;
	}

}

// /* readfile_test
//  * test the functionality of reading valid file. If we can read the file in, return pass
//  * Inputs: uint8_t* file_name, int32_t nbyte
//  * Outputs: PASS
//  */
// int readfile_test(uint8_t* file_name, int32_t nbyte){
// 	TEST_HEADER;
// 	// term_open(0);
// 	dentry_t dentry;
// 	if (read_dentry_by_name(file_name, &dentry)==-1){
// 		//file name is not found
// 		printf("file not found");
// 		return FAIL;
// 	}
// 	// OPtable_t file_operation_table = {
//     // 	.read = readfile,
//     // 	.write = writefile,
//     // 	.open = openfile,
//     // 	.close = closefile
// 	// };
// 	fda[2].inode = dentry.inode;
// 	fda[2].filepos = 0;
// 	// fda[2].OPtable = &file_operation_table;
// 	fda[2].flags = 1;
// 	uint8_t buf[READ_BUF_SIZE];
// 	int32_t cnt;
// 	while (0 != (cnt = readfile(2, buf, nbyte))){
// 		if (cnt==-1){
// 			//FAIL
// 			return FAIL;
// 		}
// 		term_write(2, buf, cnt);
// 	}
// 	return PASS;
// }

/* readdir_test
 * test the functionality of reading valid directory. If we can read the directory in, return pass
 * Inputs: NULL
 * Outputs: PASS
 */
int readdir_test(){
	TEST_HEADER;
	term_open(0);
	uint8_t buf[READ_BUF_SIZE];
	int32_t cnt;
	while (0 != (cnt = readdir(0, buf, 0))){
		if (cnt==-1){
			return FAIL;
		}
		term_write(2, buf, cnt);
		printf("\n");

	}
	term_close(0);
	return PASS;
}

/* Checkpoint 3 tests */


/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	 //TEST_OUTPUT("idt_test", idt_test());
	 //TEST_OUTPUT("div0_test", div0_test());
	 //TEST_OUTPUT("deref_test", deref_test());
	 //TEST_OUTPUT("paging_deref_test1", paging_deref_test1());
	 //TEST_OUTPUT("paging_deref_test2", paging_deref_test2());
	 //TEST_OUTPUT("paging_deref_test3", paging_deref_test3());
	 //TEST_OUTPUT("paging_deref_test4", paging_deref_test4());
	 //TEST_OUTPUT("paging_deref_test5", paging_deref_test5());
	 //TEST_OUTPUT("paging_deref_test6", paging_deref_test6());
	 //TEST_OUTPUT("paging_deref_test7", paging_deref_test7());
	 // launch your tests here
     //TEST_OUTPUT("rtc_test", rtc_test());
	 //TEST_OUTPUT("term_test", term_test());
	 //TEST_OUTPUT("read_dentry_by_name_test: frame2.txt", read_dentry_by_name_test((uint8_t*)"frame2.txt"));
	 //TEST_OUTPUT("read_dentry_by_index_test: 0", read_dentry_by_index_test(0));
	 //TEST_OUTPUT("readfile_test: fish", readfile_test((uint8_t*)"fish", READ_BUF_SIZE));
	 //TEST_OUTPUT("readfile_test: verylong", readfile_test((uint8_t*)"verylargetextwithverylongname.tx", READ_BUF_SIZE));
	 //TEST_OUTPUT("readdir_test", readdir_test());
	 //TEST_OUTPUT("readfile_test: fish", readfile_test((uint8_t*)"fish", READ_BUF_SIZE));
	 //TEST_OUTPUT("my readfile_test: grep", readfile_test((uint8_t*)"grep", READ_BUF_SIZE));
	 //TEST_OUTPUT("my readfile_test: ls", readfile_test((uint8_t*)"ls", READ_BUF_SIZE));

}


