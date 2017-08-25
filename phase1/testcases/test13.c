/*
 * This test checks to see if a process puts processes blocked on join
 * and blocked on zap back to the ready list:
 *
 *                                         fork
 *           _____ XXp1 (priority = 3)  ----------- XXp3 (priority = 5)
 *          /                                     |
 *  start1                               zap      |
 *          \____ XXp2 (priority = 4) ------------- 
 *
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *), XXp2(char *), XXp3(char *);
char buf[256];
int pid_e;

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int status, pid1, pid2, kid_pid;

    USLOSS_Console("start1(): started\n");

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 3);
    USLOSS_Console("start1(): after fork of child %d\n", pid1);

    pid2 = fork1("XXp2", XXp2, "XXp2", USLOSS_MIN_STACK, 4);
    USLOSS_Console("start1(): after fork of child %d\n", pid2);

    USLOSS_Console("start1(): performing join\n");
    kid_pid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kid_pid, status); 
    USLOSS_Console("%s", buf);

    USLOSS_Console("start1(): performing join\n");
    kid_pid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kid_pid, status); 
    USLOSS_Console("%s", buf);

    return 0;
}

int XXp1(char *arg)
{
    int status, kid_pid;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    USLOSS_Console("XXp1(): executing fork of first child\n");
    pid_e = fork1("XXp3", XXp3, "XXp3", USLOSS_MIN_STACK, 5);
    USLOSS_Console("XXp1(): fork1 of first child returned pid = %d\n", pid_e);

    USLOSS_Console("XXp1(): joining with first child\n" );
    kid_pid = join(&status);
    USLOSS_Console("XXp1(): join returned kid_pid = %d, status = %d\n",
            kid_pid, status);

    quit(-3);
    return 0;
}

int XXp2(char *arg)
{
    int status;

    USLOSS_Console("XXp2(): started\n");

    USLOSS_Console("XXp2(): zap'ing child with pid_e \n");
    status = zap(pid_e);
    USLOSS_Console("XXp2(): after zap'ing child with pid_e, status = %d\n",
                   status);

    quit(5);
    return 0;
}

int XXp3(char *arg)
{
    USLOSS_Console("XXp3(): started\n");
    dumpProcesses();
    quit(5);
    return 0;
}


