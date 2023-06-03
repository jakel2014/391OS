#include "tests.h"
#include "x86_desc.h"
#include "paging.h"
#include "idt.h"
#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "keyboard.h"
#include "file_system.h"
#include "file_system_driver.h"
#include "terminal_driver.h"

#define PASS 1
#define FAIL 0

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


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 31 IDT entries are not NULL
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
	for (i = 0; i <= 30; ++i){ // iteratring across first 31 IDT entries
		if (i != 15 && (22 > i && i > 27)) { // skip over reserved entries
			if ((idt[i].offset_15_00 == NULL) && 
				(idt[i].offset_31_16 == NULL)){
				assertion_failure();
				result = FAIL;
			}
		}
	}

	return result;
}

// add more tests here

/* IDT Test - divide_by_zero
 * 
 * Checks divide_by_zero exception
 * Inputs: None
 * Outputs: divide_by_zero exception/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: idt.c
 */
int idt_divide_test(){
	TEST_HEADER;
	int i;
	int zero = 0;
	i = 1/zero;
	return FAIL;
}

/* IDT Test - page_fault
 * 
 * Checks page_fault exception
 * Inputs: None
 * Outputs: page_fault exception/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: idt.c
 */
int idt_page_fault_test() {
	TEST_HEADER;
	int deref_NULL;
	int result = FAIL;
	int* INVALID_PTR = (int*)(0x01); // physical address 0x01 (should not contain anything)
	deref_NULL = (int)*(INVALID_PTR);

	return result;
}

/* IDT Test - system call
 * 
 * Checks System Call
 * Inputs: None
 * Outputs: System Call Handler is Invoked
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: idt.c
 */
int idt_system_call_test(){
	TEST_HEADER;
	int result = FAIL;
	asm volatile ("int $0x80"); // System Call at 0x80 in IDT table

	return result;
}


/* Paging Test - video test
 * 
 * Checks valid address in video memory
 * Inputs: None
 * Outputs: PASS/page_fault exception
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h/paging.c/paging_func.S
 */
int video_valid_paging_test(){
	int deref_VID;
	int result = PASS;
	int* VID_PTR = (int*)(0xB8000); // physical address of video memory
	deref_VID = (int)*(VID_PTR);

	return result;
}

/* Paging Test - video test
 * 
 * Checks invalid address above video memory
 * Inputs: None
 * Outputs: page_fault exception/FAIL
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h/paging.c/paging_func.S
 */
int above_video_invalid_paging_test(){
	int deref_INVALID_VID;
	int result = FAIL;
	int* INVALID_VID_PTR = (int*)(0xB9001); // physical address of 1 above video memory
	deref_INVALID_VID = (int)*(INVALID_VID_PTR);

	return result;
}

/* Paging Test - video test
 * 
 * Checks invalid address below video memory
 * Inputs: None
 * Outputs: page_fault exception/FAIL
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h/paging.c/paging_func.S
 */
int below_video_invalid_paging_test(){
	int deref_INVALID_VID;
	int result = FAIL;
	int* INVALID_VID_PTR = (int*)(0xB7FFF); // physical address of 1 below video memory
	deref_INVALID_VID = (int)*(INVALID_VID_PTR);

	return result;
}

/* Paging Test - kernel test
 * 
 * Checks valid address in kernel memory
 * Inputs: None
 * Outputs: PASS/page_fault exception
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h/paging.c/paging_func.S
 */
int kernel_valid_paging_test(){
	int deref_KER;
	int result = PASS;
	int* KER_PTR = (int*)(0x400000); // physical address of kernel memory
	deref_KER = (int)*(KER_PTR);

	return result;
}

/* Paging Test - kernel test
 * 
 * Checks invalid address below kernel memory
 * Inputs: None
 * Outputs: page_fault exception/FAIL
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h/paging.c/paging_func.S
 */
int below_kernel_invalid_paging_test(){
	int deref_NULL;
	int result = FAIL;
	int* INVALID_PTR = (int*)(0x3FFFFF); // physical address one below kernel memory
	deref_NULL = (int)*(INVALID_PTR);

	return result;
}

/* Paging Test - kernel test
 * 
 * Checks invalid address above kernel memory
 * Inputs: None
 * Outputs: page_fault exception/FAIL
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h/paging.c/paging_func.S
 */
int above_kernel_invalid_paging_test(){
	int deref_NULL;
	int result = FAIL;
	int* INVALID_PTR = (int*)(0x800001); // physical address one above kernel memory
	deref_NULL = (int)*(INVALID_PTR);

	return result;
}

/* Paging Test - null test
 * 
 * Checks invalid address at 0x00
 * Inputs: None
 * Outputs: page_fault exception/FAIL
 * Side Effects: None
 * Coverage: Load Paging, Paging definition
 * Files: paging.h/paging.c/paging_func.S
 */
int null_invalid_paging_test(){
	int deref_NULL;
	int result = FAIL;
	int* NULL_PTR = (int*)(0x00); // physical address at 0
	deref_NULL = (int)*(NULL_PTR);

	return result;
}

/* Checkpoint 2 tests */

// ----------	File System Tests	----------

/* file_open_test
 * 
 * Checks if file can be opened
 * Inputs: None
 * Outputs: PASS/FAIL for file opening
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_open_test(){
// 	int i = file_open((uint8_t*)("frame1.txt"));

// 	if (i == -1) {return FAIL;}
// 	return PASS;
// }

/* file_read_frame_test
 * 
 * Checks if file can be opened and if its contents can be read
 * Inputs: None
 * Outputs: PASS/FAIL for file opening/buf being filled with contents of file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_read_frame_test(){
// 	int i = file_open((uint8_t*)("frame0.txt"));
// 	if (i == -1) {return FAIL;}

// 	printf("%d\n", file_dentry.inode_number);
	
// 	uint32_t offset = 0;
// 	i = file_read(0, 200, file_dentry.inode_number, offset); // 200 is to test length being greater than file length
// 	if (i == 0) {return FAIL;}

// 	int j;
// 	for(j = 0; j < offset; j++) {
// 		putc(' ');
// 	}
// 	for (j = 0; j < i; j++) {
// 		putc(file_buf[j]);
// 	}
// 	return PASS;
	
// }

/* file_read_fish_test
 * 
 * Checks if fish file can be succesfully read from start of file
 * Inputs: None
 * Outputs: PASS/FAIL for file opening/buf being filled with contents of file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_read_fish_test(){
// 	int i = file_open((uint8_t*)("fish"));
// 	if (i == -1) {return FAIL;}

// 	printf("%d\n", file_dentry.inode_number);
	
// 	i = file_read(0, 0x00000120, file_dentry.inode_number, 0); // 0x00000120 is the length
// 	if (i == 0) {return FAIL;}

// 	int j;
	
// 	printf("\n");
// 	for (j = 0; j < i; j++) {
// 		if (j % 16 == 0 && j != 0) {printf("\n");} // checking for new line by modding by 16

// 		if (file_buf[j] < 32 || 127 <= file_buf[j]) { // between 32 and 127 are garbage binary values
// 			putc('.');
// 		} else {
// 			putc(file_buf[j]);
// 		}
// 	}
// 	printf("\n");
// 	printf("\n");


// 	return PASS;
	
// }

/* file_read_fish_hidden_test
 * 
 * Checks if fish file can be succesfully from an offset
 * Inputs: None
 * Outputs: PASS/FAIL for file opening/buf being filled with contents of file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_read_fish_hidden_test(){
// 	int i = file_open((uint8_t*)("fish"));
// 	if (i == -1) {return FAIL;}

// 	printf("%d\n", file_dentry.inode_number);
	
// 	i = file_read(0, 21, file_dentry.inode_number, 0x00001000); // 21 is the length, 0x00001000 is the offset
// 	if (i == 0) {return FAIL;}

// 	int j;
	
// 	printf("\n");
// 	for (j = 0; j < i; j++) {
// 		if (j % 16 == 0 && j != 0) {printf("\n");}  //checking for new line by modding by 16
// 		if (file_buf[j] < 32 || 127 <= file_buf[j]) { // between 32 and 127 are garbage binary values
// 			putc('.');
// 		} else {
// 			putc(file_buf[j]);
// 		}
// 	}
// 	printf("\n");
// 	printf("\n");

// 	return PASS;
// }

/* file_read_ls_test
 * 
 * Checks if ls file can be succesfully read at start/end of file
 * Inputs: None
 * Outputs: PASS/FAIL for file opening/buf being filled with contents of file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_read_ls_test(){
// 	int i = file_open((uint8_t*)("ls"));
// 	if (i == -1) {return FAIL;}

// 	printf("%d\n", file_dentry.inode_number);
	
// 	i = file_read(0, 16, file_dentry.inode_number, 0); // 16 is length
// 	if (i == 0) {return FAIL;}

// 	int j;
	
// 	printf("Beginning:\n");
// 	for (j = 0; j < i; j++) {
// 		if (j % 16 == 0 && j != 0) {printf("\n");} //checking for new line by modding by 16
// 		if (file_buf[j] < 32 || 127 <= file_buf[j]) { // between 32 and 127 are garbage binary values
// 			putc('.');
// 		} else {
// 			putc(file_buf[j]);
// 		}
// 	}
// 	printf("\n");
// 	printf("\n");

// 	i = file_read(0, 37, file_dentry.inode_number, 0x000014c0); // reading length 37 and starting at 0x000014c0
// 	if (i == 0) {return FAIL;}
	
// 	printf("End:\n");
// 	for (j = 0; j < i; j++) {
// 		if (j % 16 == 0 && j != 0) {printf("\n");} //checking for new line by modding by 16
// 		if (file_buf[j] < 32 || 127 <= file_buf[j]) { // between 32 and 127 are garbage binary values
// 			putc('.');
// 		} else {
// 			putc(file_buf[j]);
// 		}
// 	}
// 	printf("\n");
// 	printf("\n");
// 	return PASS;
// }

/* file_read_grep_test
 * 
 * Checks if grep file can be succesfully read at start/end of file
 * Inputs: None
 * Outputs: PASS/FAIL for file opening/buf being filled with contents of file
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_read_grep_test(){
// 	int i = file_open((uint8_t*)("grep"));
// 	if (i == -1) {return FAIL;}

// 	printf("%d\n", file_dentry.inode_number);
	
// 	i = file_read(0, 16, file_dentry.inode_number, 0); // reading length of 16
// 	if (i == -1) {return FAIL;}

// 	int j;
	
// 	printf("Beginning:\n");
// 	for (j = 0; j < i; j++) {
// 		if (j % 16 == 0 && j != 0) {printf("\n");} //checking for new line by modding by 16
// 		if (file_buf[j] < 32 || 127 <= file_buf[j]) { // between 32 and 127 are garbage binary values
// 			putc('.');
// 		} else {
// 			putc(file_buf[j]);
// 		}
// 	}
// 	printf("\n");
// 	printf("\n");

// 	i = file_read(0, 37, file_dentry.inode_number, 0x000017e0); // reading length 37 and starting at 0x000014c0
// 	if (i == -1) {return FAIL;}
	
// 	printf("End:\n");
// 	for (j = 0; j < i; j++) {
// 		if (j % 16 == 0 && j != 0) {printf("\n");} //checking for new line by modding by 16
// 		if (file_buf[j] < 32 || 127 <= file_buf[j]) { // between 32 and 127 are garbage binary values
// 			putc('.');
// 		} else {
// 			putc(file_buf[j]);
// 		}
// 	}
// 	printf("\n");
// 	printf("\n");
// 	return PASS;
// }

/* file_read_big_filename_test
 * 
 * Checks if open fails if name is too long
 * Inputs: None
 * Outputs: PASS/FAIL for file open failing with too long of a name
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_read_big_filename_test(){
// 	int i = file_open((uint8_t*)("verylargetextwithverylongname.txt"));
// 	if (i == -1) {return PASS;}

// 	printf("%d\n", file_dentry.inode_number);
	
// 	i = file_read(0, 37, file_dentry.inode_number, 0x000014c0);
// 	if (i == 0) {return FAIL;}

// 	int j;
	
// 	printf("\n");
// 	for (j = 0; j < i; j++) {
// 		if (j % 16 == 0 && j != 0) {printf("\n");} //checking for new line by modding by 16
// 		if (file_buf[j] < 32 || 127 <= file_buf[j]) { // between 32 and 127 are garbage binary values
// 			putc('.');
// 		} else {
// 			putc(file_buf[j]);
// 		}
// 	}
// 	printf("\n");
// 	printf("\n");

// 	return FAIL;
// }

/* file_read_offset_bounds_test
 * 
 * Checks if file open fails if offset is too large
 * Inputs: None
 * Outputs: PASS/FAIL for file open failing if offset is too large
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_read_offset_bounds_test(){
// 	int i = file_open((uint8_t*)("frame0.txt"));
// 	if (i == -1) {return FAIL;}

// 	printf("%d\n", file_dentry.inode_number);
	
// 	uint32_t offset = 10;
// 	i = file_read(0, 200, file_dentry.inode_number, 188); // reading length 200 from offset 188
// 	if (i == 0) {return FAIL;}

// 	int j;
// 	for(j = 0; j < offset; j++) {
// 		putc(' ');
// 	}
// 	for (j = 0; j < i; j++) {
// 		putc(file_buf[j]);
// 	}
// 	return PASS;
	
// }

/* directory_read_test
 * 
 * Checks if directory names can be successfully displayed
 * Inputs: None
 * Outputs: PASS/FAIL if directory file names are successfully named
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int directory_read_test(){
// 	int i;
// 	int count;
// 	for(i = 0; i < DIR_ENTRIES; i++){
// 		int j;
// 		count = directory_read(0, i);
// 		if(count != 0){
// 			printf("File Name: ");
// 			for(j = 0; j < count; j++){
// 				putc(directory_buf[j]);
// 			}
// 			putc('\n');
// 		}
		
// 	}

//     return PASS;

// }

/* file_close_test
 * 
 * Checks if file can be successfully closed
 * Inputs: None
 * Outputs: PASS/FAIL if file can be successfully closed
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_close_test(){
// 	file_close(0);

// 	return PASS;
// }

/* directory_close_test
 * 
 * Checks if directory entry can be successfully closed
 * Inputs: None
 * Outputs: PASS/FAIL if directory entry can be successfully closed
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int directory_close_test(){
// 	directory_close(0);

// 	return PASS;
// }

/* file_write_test
 * 
 * Makes sure file_write returns -1
 * Inputs: None
 * Outputs: PASS/FAIL if file_write returns -1
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int file_write_test(){
// 	int res = file_write(0, 0, 0);
// 	if(res == -1)
// 		return PASS;
// 	return FAIL;
// }

/* directory_write_test
 * 
 * Makes sure directory_write returns -1
 * Inputs: None
 * Outputs: PASS/FAIL if directory_write returns -1
 * Side Effects: None
 * Coverage: File System
 * Files: file_system.c
 */
// int directory_write_test(){
// 	int res = directory_write(0, 0, 0);
// 	if(res == -1)
// 		return PASS;
// 	return FAIL;
// }


// ----------	Terminal/Keyboard Tests	----------
/* terminal_driver_test
 * 
 * tests the terminal read and terminal write
 * Inputs: None
 * Outputs: PASS/FAIL if function exists after writing 'quit'
 * Side Effects: None
 * Coverage: Terminal
 * Files: terminal_driver.c
 */
int terminal_driver_test(){
	TEST_HEADER;
	int read, write;
	terminal_init();
	char buf[127];
	memset(buf, 0, 128); // 128 is length of buffer
	int read_count, write_count;
	//int open = terminal_open(0);
	// write = terminal_read(0, buf, 128);

	while (1) {				// When not equal
		// memset(buf, 0, 128);
		read = terminal_read(0, buf, 128); // 128 is length of buffer
		read_count += read;
		if (strncmp((int8_t*)keyboard_buffer, "quit", 4) == 0) {break;} // check 4 bytes
		write = terminal_write(0, buf, read);
		write_count += write_count;
	}
	printf("write = %d, read = %d", write_count, read_count);
	return PASS;
}



// ----------	RTC Tests	----------
/* rtc_frequency_cycle_test
 * 
 * tests the rtc
 * Inputs: None
 * Outputs: PASS/FAIL if rtc successfully changes frequency
 * Side Effects: None
 * Coverage: RTC
 * Files: rtc.c
 */
int rtc_frequency_cycle_test(){
	TEST_HEADER;
	clear();
	uint32_t i = 2;
	uint32_t j = 0;
	int32_t fd;
	int32_t checkpass = 0;

	fd = rtc_open(NULL);

	for(i = 2; i <= 1024; i *= 2){ // 1024 is max freq to be tested
		printf("Testing %d Hz\n", i);
		checkpass += rtc_write(0, &i, sizeof(int32_t));
		//rtc_change_freq(i);
		
		for(j = 0; j <= i; j++){
			checkpass += rtc_read(fd, NULL, sizeof(int32_t));
			printf("8");
		}
		printf("\n");
		clear();
	}
	
	if(checkpass != 0){
		return FAIL;
	}
	return PASS;

}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	clear();
	/* Checkpoint 1 */
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("idt_test - page_fault", idt_page_fault_test());
	//TEST_OUTPUT("idt_test - divide_by_zero", idt_divide_test());
	//TEST_OUTPUT("idt_test - system call", idt_system_call_test());
	//TEST_OUTPUT("video_valid_paging_test", video_valid_paging_test());
	//TEST_OUTPUT("above_video_invalid_paging_test", above_video_invalid_paging_test());
	//TEST_OUTPUT("below_video_invalid_paging_test", below_video_invalid_paging_test());
	//TEST_OUTPUT("kernel_valid_paging_test", kernel_valid_paging_test());
	//TEST_OUTPUT("below_kernel_invalid_paging_test", below_kernel_invalid_paging_test());
	//TEST_OUTPUT("above_kernel_invalid_paging_test", above_kernel_invalid_paging_test());
	//TEST_OUTPUT("null_invalid_paging_test", null_invalid_paging_test());

	/* Checkpoint 2 */
	//----------	File System		-------
	//TEST_OUTPUT("file_open_test", file_open_test());
	//TEST_OUTPUT("file_read_frame_test", file_read_frame_test());
	//TEST_OUTPUT("file_read_fish_test", file_read_fish_test()); // NOT EXACT
	//TEST_OUTPUT("file_read_fish_hidden_test", file_read_fish_hidden_test());
	//TEST_OUTPUT("file_read_ls_test", file_read_ls_test()); // Shows beginning and end msg
	//TEST_OUTPUT("file_read_grep_test", file_read_grep_test()); // Shows beginning and end msg
	//TEST_OUTPUT("file_read_big_filename_test", file_read_big_filename_test()); // File name too big. Shouldn't open
	//TEST_OUTPUT("file_read_offset_bounds_test", file_read_offset_bounds_test());
	//TEST_OUTPUT("directory_read_test", directory_read_test());
	//TEST_OUTPUT("file_close_test", file_close_test());
	//TEST_OUTPUT("directory_close_test", directory_close_test());
	//TEST_OUTPUT("file_write_test", file_write_test());
	//TEST_OUTPUT("directory_write_test", directory_write_test());


	//----------	RTC		-------
	//TEST_OUTPUT("rtc_frequency_cycle_test", rtc_frequency_cycle_test());

	//TEST_OUTPUT("terminal_driver_test", terminal_driver_test());
}


