#You can use either a gcc or g++ compiler
CC = g++
#CC = gcc
EXECUTABLES = test5 test1 test2 test3 test4
TESTS = exec_test5 exec_test1 exec_test2 exec_test3 exec_test4
CFLAGS = -ldl -g3 -Wall # -DDEBUG

all: ${EXECUTABLES} ${TESTS}

test1: test1.cc
	$(CC) $(CFLAGS) -o tests/test1 thread.cc test1.cc libinterrupt.a
	$(CC) $(CFLAGS) -o tests/test1_v thread.o test1.cc libinterrupt.a
test2: test2.cc
	$(CC) $(CFLAGS) -o tests/test2 thread.cc test2.cc libinterrupt.a
	$(CC) $(CFLAGS) -o tests/test2_v thread.o test2.cc libinterrupt.a
test3: test3.cc
	$(CC) $(CFLAGS) -o tests/test3 thread.cc test3.cc libinterrupt.a
	$(CC) $(CFLAGS) -o tests/test3_v thread.o test3.cc libinterrupt.a
test4: test4.cc
	$(CC) $(CFLAGS) -o tests/test4 thread.cc test4.cc libinterrupt.a
	$(CC) $(CFLAGS) -o tests/test4_v thread.o test4.cc libinterrupt.a
test5: test5.cc 
	$(CC) $(CFLAGS) -o tests/test5 thread.cc test5.cc libinterrupt.a
	$(CC) $(CFLAGS) -o tests/test5_v thread.o test5.cc libinterrupt.a

exec_test1: 
	tests/test1 	>> tests/test1.out	
	tests/test1_v 	>> tests/test1_v.out	
exec_test2: 
	tests/test2 	>> tests/test2.out	
	tests/test2_v 	>> tests/test2_v.out	
exec_test3: 
	tests/test3 	>> tests/test3.out	
	tests/test3_v 	>> tests/test3_v.out	
exec_test4: 
	tests/test4 	>> tests/test4.out	
	tests/test4_v 	>> tests/test4_v.out	
exec_test5: 
	tests/test5	>> tests/test5.out
	tests/test5_v 	>> tests/test5_v.out

clean:
		rm -rf tests/*test*	
		rm -rf *.dSYM

