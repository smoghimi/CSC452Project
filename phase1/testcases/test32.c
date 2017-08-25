#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

/* The purpose of this test is to demonstrate that
 * an attempt to zap an nonexistant process results in
 * a halt.
 *
 * Expected output:
 * start1(): started
 * start1(): after fork of child 3
 * start1(): performing first join
 * XXp1(): started pid=3
 * XXp1(): arg = `XXp1'
 * XXp1(): calling zap(4), which is a non-existant process
 *    should cause an abort
 * zap(): process being zapped does not exist.  Halting...
*/

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

    USLOSS_Console("start1(): performing first join\n");
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status);
    USLOSS_Console("%s", buf);

    return 0;
} /* start1 */

int XXp1(char *arg)
{
    int zapResult, toZap;

    toZap = 4;

    USLOSS_Console("XXp1(): started pid=%d\n", getpid());
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    USLOSS_Console("XXp1(): calling zap(%d), which is a non-existant process\n", toZap);
    USLOSS_Console("   should cause an abort\n");
    zapResult = zap(toZap);
    USLOSS_Console("XXp1(): zapResult = %d\n", zapResult);

    quit(-1);

    return 0;
} /* XXp1 */
