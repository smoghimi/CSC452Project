
/* A test of waitDevice for the clock */

#include <stdio.h>
#include <string.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

int XXp1(char *);
int XXp3(char *);
char buf[256];

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start2(char *arg)
{
    int kid_status, kidpid;

    USLOSS_Console("start2(): at beginning, pid = %d\n", getpid());

    kidpid = fork1("XXp1", XXp1, NULL, 2 * USLOSS_MIN_STACK, 3);
    USLOSS_Console("start2(): fork'd child %d\n", kidpid);

    kidpid = join(&kid_status);
    USLOSS_Console("start2(): joined with kid %d, status = %d\n",
                   kidpid, kid_status);

    quit(0);
    return 0; /* so gcc will not complain about its absence... */
} /* start2 */


int XXp1(char *arg)
{
    int result, status;

    USLOSS_Console("XXp1(): started, calling waitDevice for clock\n");

    result = waitDevice(USLOSS_CLOCK_DEV, 0, &status);
    USLOSS_Console("XXp1(): after waitDevice call\n");

    if ( result == -1 ) {
        USLOSS_Console("XXp1(): got zap'd result from waitDevice() call. ");
        USLOSS_Console("Should not have happened!\n");
    }
    else if ( result == 0 ) {
        USLOSS_Console("XXp1(): About to print status from clock Handler:\n");
        USLOSS_Console("XXp1(): status = %d Should be close, but ", status);
        USLOSS_Console("does not have to be an exact match\n");
    }
    else
        USLOSS_Console("XXp1(): got %d instead of -1 or 0 from waitDevice\n",
                       result);

    quit(-3);
    return 0; /* so gcc will not complain about its absence... */
} /* XXp1 */
