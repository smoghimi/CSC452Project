#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

/* The purpose of this test is to demonstrate that
 * an attempt to zap an non-existant process results in
 * a halt (even if the procTable slot where that process
 * would be if it existed is occupied).
 *
 * Expected output:
 * start1(): started
 * start1(): after fork of child 3
 * start1(): after fork of child 4
 * start1(): performing first join
 * XXp1(): started
 * XXp1(): arg = `XXp1'
 * XXp1(): zapping a non existant processes pid, should cause abort, calling zap(204)
 * zap(): process being zapped does not exist.  Halting...
 */

int XXp1(char *), XXp2(char *);
char buf[256];

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

    pid2 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 5);
    USLOSS_Console("start1(): after fork of child %d\n", pid2);

    USLOSS_Console("start1(): performing first join\n");
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    USLOSS_Console("start1(): performing second join\n");
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    return 0;
} /* start1 */

int XXp1(char *arg)
{
    int zap_result, to_zap;

    to_zap = 204;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    USLOSS_Console("XXp1(): zapping a non existant processes pid, should ");
    USLOSS_Console("cause abort, calling zap(%d)\n", to_zap);
    zap_result = zap(to_zap);
    USLOSS_Console("XXp1(): zap_result = %d\n", zap_result);

    quit(-1);

    return 0;
} /* XXp1 */
