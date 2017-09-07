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
    int status, kidpid, i, j;

    USLOSS_Console("start1(): started\n");

    for (j = 0; j < 2; j++) {
        for (i = 2; i < MAXPROC; i++) {
            kidpid = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 3);
            USLOSS_Console("start1(): after fork of child %d\n", kidpid);
        }

        dumpProcesses();

        for (i = 2; i < MAXPROC; i++) {
            kidpid = join (&status);
            USLOSS_Console("start1(): after join of child %d, status = %d\n",
                           kidpid, status);
        }
        dumpProcesses();
    }
    return 0;
}

int XXp1(char *arg)
{
    USLOSS_Console("XXp1(): started, pid = %d\n", getpid());
    quit(-getpid());
    return 0;
}

