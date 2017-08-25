/* Tests blockMe, unblockProc, and zap
 * start1 creates XXp1 at priority 5
 * XXp1 creates:
 *    3 instances of XXp2 at prority 2
 *    XXp3 at priority 3
 *    XXp4 at priority 4
 * Each XXp2 instance calls blockMe()
 * XXp3 zap's pid 5
 * XXp4 zap's pid 7
 * at this point, only XXp1 can run
 * XXp1 calls unblockProc for each instance of XXp2
 * Each instance of XXp2 calls isZapped.
 * Each instance of XXp2 quit's.
 */

#include <stdio.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *);
int XXp2(char *);
int XXp3(char *);
int XXp4(char *);

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int pid1, kid_status;

    USLOSS_Console("start1(): started\n");

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 5);
    pid1 = join(&kid_status);
    USLOSS_Console("start1: XXp1, pid = %d, done; start1 returning...\n", pid1);

    return 0; /* so gcc will not complain about its absence... */
} /* start1 */

int XXp1(char *arg)
{
    int  i, pid[10], result[10];
    char buf[20];

    USLOSS_Console("XXp1(): creating children\n");
    for (i = 0; i < 3; i++) {
        sprintf(buf, "%d", 19 + i);
        pid[i] = fork1("XXp2", XXp2, buf, USLOSS_MIN_STACK, 2);
    }

    USLOSS_Console("XXp1(): creating zapper children\n");
    pid[i] = fork1("XXp3", XXp3, "XXp3", USLOSS_MIN_STACK, 3);
    pid[i] = fork1("XXp4", XXp4, "XXp4", USLOSS_MIN_STACK, 4);

    // dumpProcesses();

    USLOSS_Console("XXp1(): unblocking children\n");
    for (i = 0; i < 3; i++)
        result[i] = unblockProc(pid[i]);

    for (i = 0; i < 3; i++)
        USLOSS_Console("XXp1(): after unblocking %d, result = %d\n",
                       pid[i], result[i]);

    quit(0);
    return 0;
} /* XXp1 */


int XXp2(char *arg)
{
    int result;
    int blockStatus;

    blockStatus = atoi(arg);
    USLOSS_Console("XXp2(): started, pid = %d, calling blockMe\n", getpid());
    result = blockMe(blockStatus);
    USLOSS_Console("XXp2(): pid = %d, after blockMe, result = %d\n",
                   getpid(), result);

    USLOSS_Console("XXp2(): pid = %d, isZapped() = %d\n", getpid(), isZapped());
    quit(0);

    return 0;
} /* XXp2 */

int XXp3(char *arg)
{
    int result;

    USLOSS_Console("XXp3(): started, pid = %d, calling zap on pid 5\n",
                   getpid());
    result = zap(5);
    USLOSS_Console("XXp3(): after call to zap, result of zap = %d\n", result);

    return 0;
} /* XXp3 */

int XXp4(char *arg)
{
    int result;

    USLOSS_Console("XXp4(): started, pid = %d, calling zap on pid 7\n",
                   getpid());
    result = zap(7);
    USLOSS_Console("XXp4(): after call to zap, result of zap = %d\n", result);

    return 0;
} /* XXp4 */
