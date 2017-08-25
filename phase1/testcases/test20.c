/*
 * Check if -1 is returned when there are no more process slots.
 * Attempt to start MAXPROC + 2 processes; i.e., 52 processes
 * Process table has 50 slots. start1 and sentinel occupy two of
 *    the slots.  Thus, 48 new processes will start, with error
 *    messages about the last 4 attempts.
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *), XXp2(char *), XXp3(char *), XXp4(char *);

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int i, pid1;

    USLOSS_Console("TEST: start %d processes\n", MAXPROC);

    for (i = 0; i < MAXPROC+2; i++) {
        pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 2);

        if (pid1 == -1) {
            USLOSS_Console("TEST: i = %d, pid is -1.\n", i);
        }
    }

    for (i = 0; i < MAXPROC + 2; i++) {
        join(&pid1);
    }  

    quit(-1);

    return 0;
}

int XXp1(char *arg)
{
 
    quit(-2);

    return 0;
}
