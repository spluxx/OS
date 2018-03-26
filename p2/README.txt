Compsci 310 Project 2: Hack into a Server
Group: compsci310-spring18-group158 (jb374, gl67, and ih33)

Project Overview:

The goal of this project was to find vulnerabilities in a Ubuntu Linux server (Remote Server: http://310test.cs.duke.edu:9464 Kill Server: http://310test.cs.duke.edu:9465) and to compromise it using shellcode through the stackoverflow attack. Particularly, we used the reverse shellcode to attack the loophole in the server so that we can remotely send commands to the server ater taking over. 


In this project, we were able to compromise a remote server by using stack smashing. We took advantage of the misue of the function strncpy(), which takes 'n' as an input to limit the length of the string. 'n' holds the siginificance of preventing exploitation of stack overflow, where the copied string can be overwritten into the return address. However, instead of limiting the length of the string to 100 with the strncpy() function, the program used "int check_filename_length(byte len)", which incrorrectly casts the string length to 'byte'. This data type has been defined as equibalent to unsigned character data type, which only allows number representation from 0 ~ 255. Using this characteriestic, we can input longer string of filename name that still passes the filename length checker method.

Using gdb we were able to acquire the reletive address of the filename variable respect to the stored return address--140 bytes. 

This allows us to create an attack string composeed of three components: the return address (RA), no operation instruction (NOP's), and shellcode (which contains string represenation of assembly instructions that opens up a shell that is bound to a particular port on a remote server). 

The tutorials we were provided with placed the RA's behind shellcodes and NOP's. However, we were only able to put 140 bytes between the filename and the saved return address. Since the size of the shellcode was already 92 bytes, we only had 48 bytes for the NOPs, which is impractical because we had to search through at least a 1000 bytes and incrementing 48 bytes every minute would've taken too much time. To resolve this problem, we placed 360 btyes of return addresses in front to ensure that the actual return address is changed to our targeted address. After then, we added 360 bytes of NOP's whould lead to our shellcode. 

We initially concerned that this method would overwrite part of memory we are not supposed to touch. However, we noticed that if the return address gets modified succesfully, we don't need to worry about that case. 

After some calculation, our target address would be within a range bound to 0xfffd066 and 0xffffd1e8 so made logical guesses to find the approximate location that will lead to our block of NOP's. 

As for compromising the remote server, the exact address of our NOP's blocks could not be traced. So we had to decrement 300 bytes each trial to find the address of our NOP's blocks. 
The offset of 300 bytes were intentionally chosen so that it is slightly smaller than the length of our NOP's block. Through iteration of logical guesses, we were able to make the remote server to execute the shellcode that allowed us to excess its shell via netcat. 







