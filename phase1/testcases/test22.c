/* start1 attempts to create two children with invalid priorities. 
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

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 9);
    if (pid1 == -1)
        USLOSS_Console("start1(): could not fork a child--invalid priority\n"); 

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, -10);
    if (pid1 == -1)
        USLOSS_Console("start1(): could not fork a child--invalid priority\n"); 

    return 0; /* so gcc will not complain about its absence... */
}

int XXp1(char *arg)
{
    int i;

    printf("XXp1(): started\n");
    printf("XXp1(): arg = `%s'\n", arg);

    for (i = 0; i < 100; i++)
        ;
    quit(-3);
    return 0;
}
