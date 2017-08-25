#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *), XXp2(char *);
char buf[256];

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int status, pid1, kidpid;

    USLOSS_Console("start1(): started\n");
    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 3);
    USLOSS_Console("start1(): after fork of child %d\n", pid1);
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status); 
    USLOSS_Console("%s", buf);
    return 0;
}

int XXp1(char *arg)
{
    int pid1, kidpid, status;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);
    pid1 = fork1("XXp2", XXp2, "XXp2", USLOSS_MIN_STACK, 2);
    USLOSS_Console("XXp1(): after fork of child %d\n", pid1);
    kidpid = join(&status);
    sprintf(buf,"XXp1(): exit status for child %d is %d\n", kidpid, status); 
    USLOSS_Console("%s", buf);
    quit(-3);
    return 0;
}

int XXp2(char *arg)
{
    USLOSS_Console("XXp2(): started\n");
    USLOSS_Console("XXp2(): arg = `%s'\n", arg);
    quit(5);
    return 0;
}

