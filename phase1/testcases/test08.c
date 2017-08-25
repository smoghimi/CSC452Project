/* This test is to check whether fork tests for the mode being == kernel */

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
    int result;

    USLOSS_Console("start1(): started\n");

    result = USLOSS_PsrSet( USLOSS_PsrGet() & ~USLOSS_PSR_CURRENT_MODE );
    if ( result != USLOSS_DEV_OK ) {
        USLOSS_Console("start1: USLOSS_PsrSet returned %d\n", result);
        USLOSS_Console("Halting...\n");
        USLOSS_Halt(1);
    }
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

    USLOSS_Console("XXp1(): started -- should not see this message!!!!!!!!\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);
    for(i = 0; i < 100; i++)
        ;
    quit(-3);
    return 0;
}
