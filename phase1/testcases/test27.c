#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

/*
 * The purpose of this test is to test when we join and our children have
 * already quit.  One will be done where we have been zapped before calling
 * join, one will be done where we have not been zapped before calling join.
 *
 * Expected output:
 *
 * start1(): started
 * start1(): after fork of child 3
 * start1(): after fork of child 4
 * start1(): after fork of child 5
 * start1(): performing first join
 * XXp1(): started
 * XXp1(): arg = `XXp1'
 * XXp4(): started
 * XXp4(): arg = `XXp4FromXXp1'
 * XXp2(): started
 * XXp2(): arg = `XXp2'
 * XXp2(): calling zap(5)
 * XXp3(): started
 * XXp3(): arg = `XXp3'
 * XXp4(): started
 * XXp4(): arg = `XXp4FromXXp3a'
 * XXp1(): after fork of child 6
 * XXp1(): performing first join
 * XXp1(): exit status for child 6 is -4
 * start1(): exit status for child 3 is -1
 * start1(): performing second join
 * XXp3(): after fork of child 7
 * XXp4(): started
 * XXp4(): arg = `XXp4FromXXp3b'
 * XXp3(): after fork of child 8
 * XXp3(): performing first join
 * XXp3(): exit status for child -1 is -4
 * XXp3(): performing second join
 * XXp3(): exit status for child -1 is -4
 * start1(): exit status for child 5 is -3
 * start1(): performing third join
 * XXp2(): return value of zap(5) is 0
 * start1(): exit status for child 4 is -2
 * All processes completed.
 */

int  XXp1(char *), XXp2(char *), XXp3(char *), XXp4(char *);
char buf[256];
int  pid3;

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int status, pid1, pid2, kidpid;

    USLOSS_Console("start1(): started\n");
    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 5);
    USLOSS_Console("start1(): after fork of child %d\n", pid1);

    pid2 = fork1("XXp2", XXp2, "XXp2", USLOSS_MIN_STACK, 5);
    USLOSS_Console("start1(): after fork of child %d\n", pid2);

    pid3 = fork1("XXp3", XXp3, "XXp3", USLOSS_MIN_STACK, 5);
    USLOSS_Console("start1(): after fork of child %d\n", pid3);

    USLOSS_Console("start1(): performing first join\n");
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    USLOSS_Console("start1(): performing second join\n");
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    USLOSS_Console("start1(): performing third join\n");
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    return 0;
} /* start1 */

int XXp1(char *arg)
{
    int pid1, kidpid, status;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);
    pid1 = fork1("XXp4", XXp4, "XXp4FromXXp1", USLOSS_MIN_STACK, 2);
    USLOSS_Console("XXp1(): after fork of child %d\n", pid1);

    USLOSS_Console("XXp1(): performing first join\n");
    kidpid = join(&status);
    sprintf(buf,"XXp1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    quit(-1);

    return 0;
} /* XXp1 */

int XXp2(char *arg)
{
    int zap_result;

    USLOSS_Console("XXp2(): started\n");
    USLOSS_Console("XXp2(): arg = `%s'\n", arg);

    USLOSS_Console("XXp2(): calling zap(%d)\n", pid3);
    zap_result = zap(pid3);
    USLOSS_Console("XXp2(): return value of zap(%d) is %d\n", pid3, zap_result);

    quit(-2);

    return 0;
} /* XXp2 */

int XXp3(char *arg)
{
    int pid1, pid2, kidpid, status;

    USLOSS_Console("XXp3(): started\n");
    USLOSS_Console("XXp3(): arg = `%s'\n", arg);

    pid1 = fork1("XXp4", XXp4, "XXp4FromXXp3a", USLOSS_MIN_STACK, 2);
    USLOSS_Console("XXp3(): after fork of child %d\n", pid1);

    pid2 = fork1("XXp4", XXp4, "XXp4FromXXp3b", USLOSS_MIN_STACK, 3);
    USLOSS_Console("XXp3(): after fork of child %d\n", pid2);

    USLOSS_Console("XXp3(): performing first join\n");
    kidpid = join(&status);
    sprintf(buf,"XXp3(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    USLOSS_Console("XXp3(): performing second join\n");
    kidpid = join(&status);
    sprintf(buf,"XXp3(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    quit(-3);

    return 0;
} /* XXp3 */

int XXp4(char *arg)
{
    USLOSS_Console("XXp4(): started\n");
    USLOSS_Console("XXp4(): arg = `%s'\n", arg);

    quit(-4);

    return 0;
} /* XXp4 */
