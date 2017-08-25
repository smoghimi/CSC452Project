#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

/*
 * The purpose of this test is to demonstrate that
 * an attempt by start1 to zap itself causes an abort
 * 
 * Expected output:
 * start1(): started
 * start1(): after fork of child 3
 * start1(): zapping myself, should cause abort, calling zap(2)
 * zap(): process 2 tried to zap itself.  Halting...
 */

char buf[256];
int pid1;

int XXp1(char *);

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int status, kidpid;

    USLOSS_Console("start1(): started\n");

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 3);

    USLOSS_Console("start1(): after fork of child %d\n", pid1);

    USLOSS_Console("start1(): zapping myself, should cause abort, ");
    USLOSS_Console("calling zap(%d)\n", getpid());

    zap(getpid());

    USLOSS_Console("start1(): performing first join\n");

    kidpid = join(&status);

    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    return 0;
} /* start1 */

int XXp1(char *arg)
{
    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    quit(-2);

    return 0;
} /* XXp1 */
