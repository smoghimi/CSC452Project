/* Counting Semaphore test */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <libuser.h>
#include <stdio.h>

int Child1(char *);
int Child2(char *);

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

    result = SemCreate(3, &sem1);
    USLOSS_Console("start3(): SemCreate returned %d\n", result);

    SemP(sem1);
    USLOSS_Console("start3(): After P in the CS\n");

    Spawn("Child1", Child1, "Child1", USLOSS_MIN_STACK, 2, &pid);
    USLOSS_Console("\nstart3(): spawn %d\n", pid);

    Spawn("Child2", Child2, "Child2", USLOSS_MIN_STACK, 3, &pid);
    USLOSS_Console("start3(): spawn %d\n", pid);

    SemV(sem1);
    USLOSS_Console("\nstart3(): After V -- may appear before: Child1(): After P attempt #3\n");

    Wait(&pid, &status);
    USLOSS_Console("\nstart3(): status of quit child = %d\n",status);

    Wait(&pid, &status);

    USLOSS_Console("start3(): status of quit child = %d\n",status);
    USLOSS_Console("start3(): Parent done\n");

    Terminate(8);

    return 0;
} /* start3 */


int Child1(char *arg) 
{
    int i;

    USLOSS_Console("\n%s(): starting\n", arg);
    for (i = 0; i < 5; i++) {
        SemP(sem1);
        if (i == 3)
            USLOSS_Console("%s(): After P attempt #3 -- may appear before: start3(): After V\n", arg);
        else
            USLOSS_Console("%s(): After P attempt #%d\n", arg, i);
    }
    USLOSS_Console("%s(): done\n", arg);
    Terminate(9);

    return 0;
} /* Child1 */


int Child2(char *arg) 
{
    int i;

    USLOSS_Console("\n%s(): starting\n", arg);
    for (i = 0; i < 5; i++) {
        SemV(sem1);
        USLOSS_Console("%s(): After V attempt #%d\n", arg, i);
    }
    USLOSS_Console("%s(): done\n", arg);
    Terminate(10);
    return 0;
} /* Child2 */
