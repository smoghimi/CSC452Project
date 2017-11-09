#include <stdlib.h>
#include <stdio.h>
#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <libuser.h>
#include <assert.h>

#define ABS(a,b) (a-b > 0 ? a-b : -(a-b))

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int Child(char *arg) 
{
    int begin, end, time;
    int me = atoi(arg);

    USLOSS_Console("Child%d(): Going to sleep for 10 seconds\n", me);
    GetTimeofDay(&begin);
    Sleep(10);
    GetTimeofDay(&end);
    time = end - begin;
    time = ABS(10000000, time);
    if (time > 1000000) {
        USLOSS_Console("Child%d(): Sleep bad: %d %d\n",
                       me, time, ABS(10000000, time));
    }
    else {
        USLOSS_Console("Child%d(): Sleep done at time %d\n", me, end);
    }
    Terminate(1);

    return 0;
} /* Child */


int start4(char *arg)
{
    int pid, status;
  
    USLOSS_Console("start4(): Start 5 children who all sleep for 10 seconds. Upon\n");
    USLOSS_Console("          waking up, each child checks if its sleep time was at\n");
    USLOSS_Console("          least 10 seconds.\n");

    Spawn("Child1", Child, "1", USLOSS_MIN_STACK, 4, &pid);
    Spawn("Child2", Child, "2", USLOSS_MIN_STACK, 4, &pid);
    Spawn("Child3", Child, "3", USLOSS_MIN_STACK, 4, &pid);
    Spawn("Child4", Child, "4", USLOSS_MIN_STACK, 4, &pid);
    Spawn("Child5", Child, "5", USLOSS_MIN_STACK, 4, &pid);
    Wait(&pid, &status);
    Wait(&pid, &status);
    Wait(&pid, &status);
    Wait(&pid, &status);
    Wait(&pid, &status);
    USLOSS_Console("start4(): Test sleep done.\n");
    Terminate(0);
  
    return 0;
}

