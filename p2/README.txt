Compsci 310 Project 2: Hack into a Server
Group: compsci310-spring18-group158 (jb374, gl67, and ih33)

Project Overview:

The goal of this project was to find vulnerabilities in a Ubuntu Linux server (Remote Server: http://310test.cs.duke.edu:9464 Kill Server: http://310test.cs.duke.edu:9465) and to compromise it using shellcode through the stackoverflow attack. Particularly, we used the reverse shellcode to attack the loophole in the server so that we can remotely send commands to the server ater taking over. 


Stage 1. Find the vulnerability in webserver.c

The server's vulnerability was that the int function "check_filename_length(byte len)" that was used to check the buffer bound. The length of the string was typecasted to byte, instead of int. This is the critical flaw because even though the "char filename[100];" seems to set the size of the name to under 100 chars but this flaw allows the input of strings longer than the demanded size. The result of this flaw is the Buffer Overflow, which means the characters longer than the requried bound would overwrite the return address. 


Stage 2. Use gdb (GNU Debugger) to identify where the relevant variables are stored in memory


