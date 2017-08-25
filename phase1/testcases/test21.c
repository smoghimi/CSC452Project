/* this test case checks for a process zapping itself 
 * start1 forks XXp1 and is blocked on the join of XXp1 and XXp1 zaps itself
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *);
char buf[256];
int pid1;

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start1(char *arg)
{
    int status, kidpid;
    USLOSS_Console("start1(): started\n");

    pid1 = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 3);
    USLOSS_Console("start1(): after fork of child %d\n", pid1);

    USLOSS_Console("start1(): performing join\n");
    kidpid = join(&status);
    sprintf(buf,"start1(): exit status for child %d is %d\n", kidpid, status); 
    USLOSS_Console("%s", buf);

    quit(0);

    return 0; /* so gcc will not complain about its absence... */
}

int XXp1(char *arg)
{
    int status;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);

    status = zap(pid1);
    USLOSS_Console("XXp1(): after zap'ing itself , status = %d\n", status);

    quit(-3);

    return 0;
}
