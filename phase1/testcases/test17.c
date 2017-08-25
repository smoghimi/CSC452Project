/*
 * Check that getpid() functions correctly.
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

#define FLAG1 -2

int XXp1(char *), XXp2(char *), XXp3(char *), XXp4(char *);
int pidlist[5];

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int ret1, ret2, ret3;
    int status;

    pidlist[0] = fork1("XXp1", XXp1, NULL, USLOSS_MIN_STACK, 2);
    pidlist[1] = fork1("XXp1", XXp1, NULL, USLOSS_MIN_STACK, 3);
    pidlist[2] = fork1("XXp1", XXp1, NULL, USLOSS_MIN_STACK, 4);

    ret1 = join(&status);
    USLOSS_Console("start1: joined with child %d\n", ret1);

    ret2 = join(&status);
    USLOSS_Console("start1: joined with child %d\n", ret2);

    ret3 = join(&status);
    USLOSS_Console("start1: joined with child %d\n", ret3);


    USLOSS_Console("TEST:");
    USLOSS_Console("exit getpid test.\n");

    quit(-1);
    return 0;
}

int XXp1(char *arg)
{
    static int i = 0;

    if (getpid() != pidlist[i]){
        USLOSS_Console("TEST:");
        USLOSS_Console("Getpid failed. %d %d\n", getpid(), pidlist[i]);
    }
    else{
        USLOSS_Console("TEST:");
        USLOSS_Console("Getpid passed. %d %d\n", getpid(), pidlist[i]);
    }

    i++;

    quit(FLAG1);
    return 0;
}

