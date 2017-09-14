
PREFIX = ${HOME}

TARGET = libphase1.a
ASSIGNMENT = 452phase1
CC = gcc
AR = ar

COBJS = phase1.o p1.o
CSRCS = ${COBJS:.o=.c}

HDRS = kernel.h phase1.h

INCLUDE = ${PREFIX}/include

CFLAGS = -Wall -g -I${INCLUDE} -I. -std=gnu99

UNAME := $(shell uname -s)

ifeq ($(UNAME), Darwin)
	CFLAGS += -D_XOPEN_SOURCE      # use for Mac, NOT for Linux!!
endif

LDFLAGS = -L. -L${PREFIX}/lib

TESTDIR = testcases
TESTS = test00 test01 test02 test03 test04 test05 test06 test07 test08 test09 test10 test11 test12 test13 test14 test15 test16 test17 test18 test19 test20 test21 test22 test23 test24 test25 test26 test27 test28 test29 test30 test31 test32 test33 test34 test35 test36
LIBS = -lphase1 -lusloss3.6

$(TARGET):	$(COBJS)
		$(AR) -r $@ $(COBJS) 

$(TESTS):	$(TARGET) p1.o
	$(CC) $(CFLAGS) -c $(TESTDIR)/$@.c
	$(CC) $(LDFLAGS) -o $@ $@.o $(LIBS) p1.o

clean:
	rm -f $(COBJS) $(TARGET) p1.o test??.o test?? test??.txt core term*.out

phase1.o:	kernel.h

submit:	$(CSRCS) $(HDRS) Makefile
	tar cvzf phase1.tgz $(CSRCS) $(HDRS) Makefile
