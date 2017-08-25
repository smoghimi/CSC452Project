/* Tests what happens when a process tries to zap the Sentinel.
 *
 * start1 creates XXp1 at priority 3
 * start1 blocks on a join
 *
 * XXp1 attempts to zap the Sentinel
 *
 * Sentinel is now the only process that can run. Determines there
 * are 3 processes left. Reports this state, then Halts USLOSS.
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *);
char buf[256];

#define SENTINELPID  1

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
    int zapResult;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    USLOSS_Console("XXp1(): about to zap the Sentinel process\n");
    zapResult = zap(SENTINELPID);
    USLOSS_Console("XXp1(): zapResult = %d\n", zapResult);

    quit(-3);
    return 0;
}
