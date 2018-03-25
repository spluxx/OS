Compsci 310 Project 2: Hack into a Server
Group: compsci310-spring18-group158 (jb374, gl67, and ih33)

Project Overview:

The goal of this project was to find vulnerabilities in a Ubuntu Linux server (Remote Server: http://310test.cs.duke.edu:9464 Kill Server: http://310test.cs.duke.edu:9465) and to compromise it using shellcode through the stackoverflow attack. 



The vulnerability of the server was that the int function "check_filename_length(byte len)":

int check_filename_length(byte len) {
  if (len < 100) {
    return 1;
  }
  return 0;
}

The problem is the input takes the byte of the string length instead of as an int. 
