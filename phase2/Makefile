PREFIX = ${HOME}

TARGET = libphase2.a
ASSIGNMENT = 452phase2
CC = gcc
AR = ar

COBJS = phase2.o
CSRCS = ${COBJS:.o=.c}

PHASE1LIB = patrickphase1
#PHASE1LIB = patrickphase1debug
#PHASE1LIB = phase1
#PHASE1LIB = phase1debug

HDRS = message.h

INCLUDE = ${PREFIX}/include

CFLAGS = -Wall -g -std=gnu99 -I${INCLUDE} -I.

UNAME := $(shell uname -s)

ifeq ($(UNAME), Darwin)
        CFLAGS += -D_XOPEN_SOURCE
endif

LDFLAGS += -L. -L${PREFIX}/lib 

TESTDIR = testcases
TESTS= test00 test01 test02 test03 test04 test05 test06 test07 test08 \
       test09 test10 test11 test12 test13 test14 test15 test16 test17 \
       test18 test19 test20 test21 test22 test23 test24	test25 test26 \
       test27 test28 test29 test30 test31 test32 test33 test34 test35 \
       test36 test37 test38 test39 test40 test41 test42 test43 test44

LIBS = -l$(PHASE1LIB) -lphase2 -lusloss3.6

$(TARGET):	$(COBJS)
		$(AR) -r $@ $(COBJS) 

$(TESTS):       $(TARGET) p1.o
	$(CC) $(CFLAGS) -c $(TESTDIR)/$@.c
	$(CC) $(LDFLAGS) -o $@ $(LIBS) $@.o $(LIBS) p1.o
# $(LIBS) appears twice on previous line so startup can "find" start1
# on Linux.  The first occurrence is not necessary on OS X

clean:
	rm -f $(COBJS) $(TARGET) core term*.out test*.o test??.txt $(TESTS) \
		p1.o

phase2.o:	message.h

submit: $(CSRCS) $(HDRS) Makefile
	tar cvzf phase2.tgz *.c *.h Makefile
