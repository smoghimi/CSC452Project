/*
 * check_zapped_by_manyprocs
 * NOTE: The output for this is non-deterministic.

 * Check if all process which have issued a zap on a process are awakened
 * when the target process finally quits.
 */

#include <stdio.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *), XXp2(char *), XXp3(char *), XXp4(char *);
char buf[256];

#define N 10

int victim;

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int i, status, pid2;
    char buf[25];

    USLOSS_Console("start1(): started\n");

    victim = fork1("XXp3", XXp3,"XXp3",USLOSS_MIN_STACK,5);

    for (i = 0; i < N; i++) {
        sprintf(buf, "%d", i);
        pid2 = fork1("XXp2", XXp2, buf, USLOSS_MIN_STACK, 4);
        USLOSS_Console("fork1 has started %d\n", pid2);
    }

    join(&status);
    for (i = 0;i < N;i++) {
        join(&status);
    }

    USLOSS_Console("start1(): calling quit\n");
    quit(-1);

    return 0;
} /* start1 */

int count = 0;

int XXp2(char *arg)
{
    int i = atoi(arg);

    count++;
    USLOSS_Console("XXp2(): %d zapping XXp3\n", i);
    zap(victim);
    USLOSS_Console("XXp2(): %d after zap\n", i);

    quit(-3);

    return 0;
}

int XXp3(char *arg)
{
    USLOSS_Console("XXp3(): started\n");

    while(count < N) {
        ;
    }
    USLOSS_Console("XXp3(): count = %d\n",count);
    quit(-4);

    return 0;
}

