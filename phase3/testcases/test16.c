/* Basic semaphore release test: */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <libuser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Child1(char *);

int sem1;

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}


int start3(char *arg)
{
    int result;
    int pid;
    int status;

    USLOSS_Console("start3(): started\n");
    result = SemCreate(1, &sem1);
    SemP(sem1);
    USLOSS_Console("start3(): After P in the CS\n");
    Spawn("Child1", Child1, "Child1", USLOSS_MIN_STACK, 2, &pid);
    USLOSS_Console("start3(): fork %d\n", pid);
    Spawn("Child2", Child1, "Child2", USLOSS_MIN_STACK, 2, &pid);
    USLOSS_Console("start3(): fork %d\n", pid);
    result = SemFree(sem1); 
    if(result == 1)
        USLOSS_Console("start3(): After SemFree with processes blocked\n");
    else{
        USLOSS_Console("start3(): test failed ... wrong value returned\n");
        exit(1);
    }
    Wait(&pid, &status);
    USLOSS_Console("start3(): status of quit child = %d\n",status);
    Wait(&pid, &status);
    USLOSS_Console("start3(): status of quit child = %d\n",status);
    USLOSS_Console("start3(): Parent done\n");
    Terminate(8);

    return 0;
} /* start3 */


int Child1(char *arg) 
{

    if(!(strcmp(arg,"Child1")))
        USLOSS_Console("Child1(): starting\n");
    else{
        if(!(strcmp(arg,"Child2")))
            USLOSS_Console("Child2(): starting\n");
        else{
            USLOSS_Console("Child: wrong argument passed ... test failed\n");
            exit(1);
        }
    }
    SemP(sem1);
    USLOSS_Console("%s(): After P in the CS\n", arg);
    SemV(sem1);
    USLOSS_Console("%s(): After V in the CS\n", arg);
    USLOSS_Console("%s(): done\n", arg);
    Terminate(9);

    return 0;
} /* Child1 */

