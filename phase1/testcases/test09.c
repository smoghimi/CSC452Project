/* This test case checks whether quit checks for mode being == kernel */ 

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *);
char buf[256];

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int status, pid1, kidpid;
    USLOSS_Console("start1(): started\n");

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 3);
    USLOSS_Console("start1(): after fork of child %d\n", pid1);
    USLOSS_Console("start1(): performing join\n");
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status); 
    USLOSS_Console("%s", buf);
    quit(0);
    return 0; /* so gcc will not complain about its absence... */
}

int XXp1(char *arg)
{
    int i;
    int result;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    for (i = 0; i < 100; i++)
        ;

    result = USLOSS_PsrSet( USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE );
    if ( result != USLOSS_DEV_OK ) {
        USLOSS_Console("start1: USLOSS_PsrSet returned %d\n", result);
        USLOSS_Console("Halting...\n");
        USLOSS_Halt(1);
    }

    quit(-3);
    return 0;
}
