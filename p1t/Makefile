#You can use either a gcc or g++ compiler
CC = g++
NTESTS = 7
TESTS = build_tests exec_tests
CFLAGS = -ldl -g3 -Wall # -DDEBUG

all: ${TESTS}

build_tests:
	n=1 ; while [[ $$n -le $(NTESTS) ]] ; do\
	  $(CC) $(CFLAGS) -o tests/test$$n"_v" thread.o test$$n.cc libinterrupt.a;\
	  $(CC) $(CFLAGS) -o tests/test$$n thread.cc test$$n.cc libinterrupt.a; \
	  echo "done building test"$$n; \
	  ((n = n + 1)); \
	done; 											

exec_tests:
	n=1 ; while [[ $$n -le $(NTESTS) ]] ; do\
	  tests/test$$n >> tests/test$$n.out;\
	  tests/test$$n"_v" >> tests/test$$n"_v".out;\
	  echo "done executing test"$$n; \
	  ((n = n + 1));\
	done;						

clean:
		rm -rf tests/*test*	
		rm -rf *.dSYM

