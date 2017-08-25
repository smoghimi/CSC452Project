#include <stdio.h>
#include <usloss.h>
#include <phase1.h>
#include <string.h>

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
    char buf[20];

    USLOSS_Console("start1(): started\n");

    for (j = 0; j < 2; j++) {
        for (i = 2; i < 5; i++) {
            sprintf(buf, "XXp%d", i);
            USLOSS_Console("start1(): buf = `%s'\n", buf);
            kidpid = fork1("XXp1", XXp1, buf, USLOSS_MIN_STACK, 3);
            USLOSS_Console("start1(): after fork of child %d\n", kidpid);
        }

        for (i = 2; i < 5; i++) {
            kidpid = join (&status);
            USLOSS_Console("start1(): after join of child %d, status = %d\n",
                           kidpid, status);
        }

    }
    return 0;
}

int XXp1(char *arg)
{
    int i;

    USLOSS_Console("XXp1(): %s, started, pid = %d\n", arg, getpid());
    if ( strcmp(arg, "XXp3") == 0 ) {
        for (i = 0; i < 10000000; i++)
            if ( i == 7500000)
                dumpProcesses();
    }
    else {
       for (i = 0; i < 10000000; i++)
           ;
    }

    USLOSS_Console("XXp1(): exitting, pid = %d\n", getpid());
    quit(-getpid());
    return 0;
}

