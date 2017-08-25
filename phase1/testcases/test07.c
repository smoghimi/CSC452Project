/* start1 creates XXp1.
 * start1 then quit's before XXp1 has a chance to run
 * USLOSS should complain about start1 quitting while having
 *    active children.
 */
#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

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
    int pid1;

    USLOSS_Console("start1(): started\n");

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 3);
    USLOSS_Console("start1(): after fork of child %d\n", pid1);
    quit(0);
    return 0; /* so gcc will not complain about its absence... */
}

int XXp1(char *arg)
{
    int i;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);
    for (i = 0; i < 100; i++)
        ;
    quit(-3);
    return 0;
}
