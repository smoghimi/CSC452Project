/* recursive terminate test */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <libuser.h>
#include <stdio.h>

int Child1 (char *);
int Child2 (char *);
int Child2a(char *);
int Child2b(char *);
int Child2c(char *);

int sem1;

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}


int start3(char *arg)
{
    int pid;
    int status;

    USLOSS_Console("start3(): started\n");
    Spawn("Child1", Child1, "Child1", USLOSS_MIN_STACK, 4, &pid);
    USLOSS_Console("start3(): spawned process %d\n", pid);
    Wait(&pid, &status);
    USLOSS_Console("\nstart3(): child %d returned status of %d\n", pid, status);
    USLOSS_Console("start3(): done\n");
    Terminate(8);
    return 0;
} /* start3 */


int Child1(char *arg) 
{
    int pid;
    int status;

    GetPID(&pid);
    USLOSS_Console("%s(): starting, pid = %d\n", arg, pid);
    Spawn("Child2", Child2, "Child2", USLOSS_MIN_STACK, 2, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);
    Wait(&pid, &status);
    USLOSS_Console("\n%s(): child %d returned status of %d\n",
                   arg, pid, status);
    USLOSS_Console("%s(): done\n", arg);
    Terminate(9);

    return 0;
} /* Child1 */

int Child2(char *arg) 
{
    int pid, kidpid, status, result;

    GetPID(&pid);
    USLOSS_Console("\n%s(): starting, pid = %d\n", arg, pid);
    Spawn("Child2a", Child2a, "Child2a", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);
    Spawn("Child2b", Child2b, "Child2b", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);
    Spawn("Child2c", Child2c, "Child2c", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);
    result = Wait(&kidpid, &status);
    USLOSS_Console("%s(): Wait result for child %d has status %d\n",
                   arg, kidpid, status);
    result = Wait(&kidpid, &status);
    USLOSS_Console("%s(): Wait result for child %d has status %d\n",
                   arg, kidpid, status);
    result = Wait(&kidpid, &status);
    USLOSS_Console("%s(): Wait result for child %d has status %d\n",
                   arg, kidpid, status);
    Terminate(10);

    return result;  // return result to avoid compiler warning
} /* Child2 */

int Child2a(char *arg) 
{
    USLOSS_Console("\n%s(): starting the code for Child2a\n", arg);
    Terminate(11);

    return 0;
} /* Child2a */

int Child2b(char *arg) 
{
    USLOSS_Console("\n%s(): starting the code for Child2b\n", arg);
    Terminate(11);

    return 0;
} /* Child2b */

int Child2c(char *arg) 
{
    USLOSS_Console("\n%s(): starting the code for Child2c\n", arg);
    Terminate(11);

    return 0;
} /* Child2c */
