#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

/* The purpose of this test is to demonstrate that
 * if we zap a process that has already quit but has
 * not joined, we return immediatly from zap with
 * status 0.
 * 
 * Expected output:
 * start1(): started
 * start1(): after fork of child 3
 * start1(): performing first join
 * XXp1(): started
 * XXp1(): arg = `XXp1'
 * XXp2(): started
 * XXp2(): arg = `XXp2'
 * XXp2(): exiting by calling quit(-2)
 * XXp1(): after fork of child 4
 * XXp3(): started
 * XXp3(): arg = `XXp3'
 * XXp3(): calling zap(4)
 * XXp3(): zap(4) returned: 0
 * XXp1(): after fork of child 5
 * XXp1(): performing first join
 * XXp1(): exit status for child 4 is -2
 * XXp1(): performing second join
 * XXp1(): exit status for child 5 is -3
 * start1(): exit status for child 3 is -1
 * All processes completed.
 */

int XXp1(char *), XXp2(char *), XXp3(char *);
char buf[256];

int toZapPid;

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

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 4);
    USLOSS_Console("start1(): after fork of child %d\n", pid1);

    USLOSS_Console("start1(): performing first join\n");
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    return 0;
} /* start */

int XXp1(char *arg)
{
    int status, pid1, kidpid;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    toZapPid = fork1("XXp2", XXp2, "XXp2", USLOSS_MIN_STACK, 3);
    USLOSS_Console("XXp1(): after fork of child %d\n", toZapPid);

    pid1 = fork1("XXp3", XXp3, "XXp3", USLOSS_MIN_STACK, 2);
    USLOSS_Console("XXp1(): after fork of child %d\n", pid1);

    USLOSS_Console("XXp1(): performing first join\n");
    kidpid = join(&status);
    sprintf(buf,"XXp1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);
    USLOSS_Console("XXp1(): performing second join\n");
    kidpid = join(&status);
    sprintf(buf,"XXp1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);
    quit(-1);
    return 0;
} /* XXp1 */

int XXp2(char *arg)
{
    USLOSS_Console("XXp2(): started\n");
    USLOSS_Console("XXp2(): arg = `%s'\n", arg);

    USLOSS_Console("XXp2(): exiting by calling quit(-2)\n");
    quit(-2);

    return 0;
} /* XXp2 */

int XXp3(char *arg)
{
    int result;

    USLOSS_Console("XXp3(): started\n");
    USLOSS_Console("XXp3(): arg = `%s'\n", arg);

    USLOSS_Console("XXp3(): calling zap(%d)\n", toZapPid);
    result = zap(toZapPid);
    USLOSS_Console("XXp3(): zap(%d) returned: %d\n", toZapPid, result);

    quit(-3);

    return 0;
} /* XXp3 */
