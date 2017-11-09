#include <stdio.h>
#include <libuser.h>
#include <usloss.h>
#include <phase4.h>
#include <string.h>

int Child1(char *arg);
int Child2(char *arg);

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start4(char *arg)
{
    int kidpid, status;

    USLOSS_Console("start4(): Spawn two children. Child1 writes one line to\n");
    USLOSS_Console("          terminal 1. Child2 reads one line from ");
    USLOSS_Console("terminal 1.\n");

    Spawn("Child1", Child1, NULL, 2 * USLOSS_MIN_STACK, 4, &kidpid);
    Spawn("Child2", Child2, NULL, 2 * USLOSS_MIN_STACK, 4, &kidpid);

    Wait(&kidpid, &status);
    Wait(&kidpid, &status);

    USLOSS_Console("start4(): done.\n");
    Terminate(0);

    USLOSS_Console("start4(): should not see this message!\n");
    return 0;

} /* start4 */


int Child1(char *arg)
{
    char buffer[MAXLINE];
    int  result, size;

    sprintf(buffer, "A Something interesting to print here...\n");
    result = TermWrite(buffer, strlen(buffer), 1, &size);

    if ( result < 0 )
        USLOSS_Console("Child1(): got negative result from TermWrite!\n");

    if ( size != strlen(buffer) )
        USLOSS_Console("Child1(): got wrong result, %d, from TermWrite!!\n",
                       result);

    USLOSS_Console("Child1(): Terminating\n");
    Terminate(1);

    USLOSS_Console("Child1(): should not see this message!\n");
    return 1;

} /* Child1 */

int Child2(char *arg)
{
    int  i, size;
    char buffer[MAXLINE];

    // Zero-out buffer before we read into it
    for (i = 0; i < MAXLINE; i++)
        buffer[i] = '\0';

    TermRead(buffer, MAXLINE, 1, &size);
    USLOSS_Console("Child2(): read %d characters from terminal 1\n", size);
    USLOSS_Console("Child2(): read `%s'\n", buffer);

    USLOSS_Console("Child2(): Terminating\n");
    Terminate(1);

    USLOSS_Console("Child2(): should not see this message!\n");
    return 1;

} /* Child2 */

