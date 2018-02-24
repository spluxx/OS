#You can use either a gcc or g++ compiler
CC = g++
#CC = gcc
EXECUTABLES = basic_test lock_test
CFLAGS = -ldl -g3 -Wall 

all: ${EXECUTABLES}

deli: deli.cc 
	$(CC) $(CFLAGS) -o tests/deli thread.cc deli.cc libinterrupt.a
	$(CC) $(CFLAGS) -o tests/deli_v thread.o deli.cc libinterrupt.a
basic_test: basic_test.cc
	$(CC) $(CFLAGS) -o tests/basic_test thread.cc basic_test.cc libinterrupt.a
	$(CC) $(CFLAGS) -o tests/basic_test_v thread.o basic_test.cc libinterrupt.a
lock_test: lock_test.cc
	$(CC) $(CFLAGS) -o tests/lock_test thread.cc lock_test.cc libinterrupt.a
	$(CC) $(CFLAGS) -o tests/lock_test_v thread.o lock_test.cc libinterrupt.a

clean:
		rm -f tests/*
			rm -rf *.dSYM
