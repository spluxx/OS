#You can use either a gcc or g++ compiler
#CC = g++
CC = gcc
EXECUTABLES = basicdmmtest test_basic test_coalesce test_stress1 test_stress2 test_stress3
CFLAGS = -I. -g3 -Wall # -DNDEBUG

all: ${EXECUTABLES}

test: CFLAGS += $(OPTFLAG)
test: ${EXECUTABLES}
	for exec in ${EXECUTABLES}; do \
    		./$$exec ; \
	done

basicdmmtest: basicdmmtest.c dmm.o
	$(CC) $(CFLAGS) -o basicdmmtest basicdmmtest.c dmm.o
test_basic: test_basic.c dmm.o
	$(CC) $(CFLAGS) -o test_basic test_basic.c dmm.o
test_coalesce: test_coalesce.c dmm.o
	$(CC) $(CFLAGS) -o test_coalesce test_coalesce.c dmm.o
test_stress1: test_stress1.c dmm.o
	$(CC) $(CFLAGS) -o test_stress1 test_stress1.c dmm.o
test_stress2: test_stress2.c dmm.o
	$(CC) $(CFLAGS) -o test_stress2 test_stress2.c dmm.o
test_stress3: test_stress3.c dmm.o
	$(CC) $(CFLAGS) -o test_stress3 test_stress3.c dmm.o
clean:
	rm -f *.o ${EXECUTABLES} a.out
	rm -rf *.dSYM

