#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <libuser.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static char XXbuf[4][512];
int ubiq(int);

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int k1(char *arg)
{
    ubiq(5);
    Terminate(2);
    return 0;
} /* k1 */

int k2(char *arg)
{
    ubiq(3);
    Terminate(3);
    return 0;
} /* k2 */

int k3(char *arg)
{
    ubiq(9);
    Terminate(4);
    return 0;
} /* k3 */

int k4(char *arg)
{
    ubiq(0);
    Terminate(5);
    return 0;
} /* k4 */

int k5(char *arg)
{
    ubiq(7);
    Terminate(6);
    return 0;
} /* k5 */

int k6(char *arg)
{
    ubiq(2);
    Terminate(7);
    return 0;
} /* k6 */

int k7(char *arg)
{
    ubiq(1);
    Terminate(8);
    return 0;
} /* k7 */

int k8(char *arg)
{
    ubiq(6);
    Terminate(9);
    return 0;
} /* k8 */

int cksum=0;

int ubiq(int t)
{
    int status = -1;
    char buf[50];
    int z = t % 4;

    USLOSS_Console("going to write track %d\n", t);
    if (DiskWrite(XXbuf[z], 1, t, 4, 1, &status) < 0) {
        USLOSS_Console("ERROR: DiskPut\n");
    }

    if (status != 0) {
        sprintf(buf,"disk_put returned error   %d\n",t);
        USLOSS_Console(buf);
    }
    else {
        USLOSS_Console("after writing track %d\n", t);
        cksum+=t;
    }
    return 0;
} /* ubiq */

int start4(char *arg)
{
    int status, pid;

    USLOSS_Console("start4(): disk scheduling test, 8 children\n");
    USLOSS_Console("          algorithm test           \n");
    USLOSS_Console("          different tracks\n");

    strcpy(XXbuf[0],"One flew East\n");
    strcpy(XXbuf[1],"One flew West\n");
    strcpy(XXbuf[2],"One flew over the coo-coo's nest\n");
    strcpy(XXbuf[3],"--did it work?\n");

    Spawn("k1", k1, NULL, USLOSS_MIN_STACK, 1, &pid);
    Spawn("k2", k2, NULL, USLOSS_MIN_STACK, 1, &pid);
    Spawn("k3", k3, NULL, USLOSS_MIN_STACK, 1, &pid);
    Spawn("k4", k4, NULL, USLOSS_MIN_STACK, 1, &pid);
    Spawn("k5", k5, NULL, USLOSS_MIN_STACK, 1, &pid);
    Spawn("k6", k6, NULL, USLOSS_MIN_STACK, 1, &pid);
    Spawn("k7", k7, NULL, USLOSS_MIN_STACK, 1, &pid);
    Spawn("k8", k8, NULL, USLOSS_MIN_STACK, 1, &pid);

    Wait(&pid, &status);
    USLOSS_Console("process %d quit with status %d\n", pid, status);
    Wait(&pid, &status);
    USLOSS_Console("process %d quit with status %d\n", pid, status);
    Wait(&pid, &status);
    USLOSS_Console("process %d quit with status %d\n", pid, status);
    Wait(&pid, &status);
    USLOSS_Console("process %d quit with status %d\n", pid, status);
    Wait(&pid, &status);
    USLOSS_Console("process %d quit with status %d\n", pid, status);
    Wait(&pid, &status);
    USLOSS_Console("process %d quit with status %d\n", pid, status);
    Wait(&pid, &status);
    USLOSS_Console("process %d quit with status %d\n", pid, status);
    Wait(&pid, &status);
    USLOSS_Console("process %d quit with status %d\n", pid, status);

    USLOSS_Console("start4(): done %d\n",cksum);
    Terminate(2);
    return 0;
} /* start4 */
