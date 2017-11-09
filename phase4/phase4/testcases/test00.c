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
    int me = atoi(arg);

    USLOSS_Console("Child%d(): started, calling Terminate\n", me);
    Terminate(1);

    return 0;
} /* Child */


int start4(char *arg)
{
    int begin, end, time;
  
    USLOSS_Console("start4(): started\n");
    USLOSS_Console("          going to sleep for 5 seconds.\n");

    GetTimeofDay(&begin);

    USLOSS_Console("start4(): Sleep starts at %6d\n", begin);

    Sleep(5);

    GetTimeofDay(&end);

    time = end - begin;
    time = ABS(5000000, time);
    if (time > 1000000) {
        USLOSS_Console("start4(): Sleep bad: %d %d\n",
                       time, ABS(10000000, time));
    }
    else {
        USLOSS_Console("start4(): Sleep done at %8d\n", end);
    }

    USLOSS_Console("start4(): done.\n");
    Terminate(0);
  
    return 0;
}

