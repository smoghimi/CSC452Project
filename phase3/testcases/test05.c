
/*
 * Max Sem Create test.
 */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>
#include <libuser.h>
#include <stdio.h>

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start3(char *arg)
{
    int semaphore;
    int sem_result;
    int i;

    USLOSS_Console("start3(): started.  Calling SemCreate\n");

    for (i = 0; i < MAXSEMS + 2; i++) {
        sem_result = SemCreate(0, &semaphore);
        USLOSS_Console("i = %3d, sem_result = %2d\n", i, sem_result);
    }

    Terminate(8);

    return 0;
} /* start3 */

