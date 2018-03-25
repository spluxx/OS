/* 
 * This is the code to span a shell in C 
 * Followed the instructions from "Smashing the Stack for Fun and Profit" 
*/
#include <stdio.h>
#include <string.h>

void main(){
	char *name[2];
	name[0]= "/bin/bash";
	name[1] = NULL;
	execve(name[0],name,NULL);
	exit(0);
}
