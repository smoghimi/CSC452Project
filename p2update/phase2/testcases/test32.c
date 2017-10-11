
/* A test of waitDevice for all four terminals.
 * XXp0 tests terminal 0.
 * XXp1 tests terminal 1.
 * XXp2 tests terminal 2.
 * XXp3 tests terminal 3.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

int XXp1(char *);
int XXp3(char *);
char buf[256];

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start2(char *arg)
{
    int  kid_status, kidpid;
    long control = 0;
    int  i, result;

    USLOSS_Console("start2(): started\n");

    control = USLOSS_TERM_CTRL_RECV_INT(control);

    USLOSS_Console("start2(): calling USLOSS_DeviceOutput to enable receive ");
    USLOSS_Console("interrupts, control = %d\n", control);

    for ( i = 0; i < 4; i++ ) {
        result = USLOSS_DeviceOutput(USLOSS_TERM_DEV, i, (void *)control);
        if ( result != USLOSS_DEV_OK ) {
            USLOSS_Console("start2(): USLOSS_DeviceOutput returned %d for %d ",
                           result, i);
            USLOSS_Console("Halting...\n");
            USLOSS_Halt(1);
        }
    }

    // Create 4 instances of XXp1, but name them XXp0 to XXp3
    // to correspond to input terminals 0 to 3
    kidpid = fork1("XXp0", XXp1, "0", 2 * USLOSS_MIN_STACK, 3);
    kidpid = fork1("XXp1", XXp1, "1", 2 * USLOSS_MIN_STACK, 3);
    kidpid = fork1("XXp2", XXp1, "2", 2 * USLOSS_MIN_STACK, 3);
    kidpid = fork1("XXp3", XXp1, "3", 2 * USLOSS_MIN_STACK, 3);

    kidpid = join(&kid_status);
    USLOSS_Console("start2(): joined with kid %d, status = %d\n\n",
                   kidpid, kid_status);
    kidpid = join(&kid_status);
    USLOSS_Console("start2(): joined with kid %d, status = %d\n\n",
                   kidpid, kid_status);
    kidpid = join(&kid_status);
    USLOSS_Console("start2(): joined with kid %d, status = %d\n\n",
                   kidpid, kid_status);
    kidpid = join(&kid_status);
    USLOSS_Console("start2(): joined with kid %d, status = %d\n\n",
                   kidpid, kid_status);

    quit(0);
    return 0; /* so gcc will not complain about its absence... */
} /* start2 */


int XXp1(char *arg)
{
    int result, status;
    int terminal = atoi(arg);

    USLOSS_Console("XXp%d(): started, calling waitDevice for terminal %d\n",
                   terminal, terminal);

    result = waitDevice(USLOSS_TERM_DEV, terminal, &status);
    USLOSS_Console("XXp%d(): after waitDevice call\n", terminal);

    if ( result == -1 ) {
        USLOSS_Console("XXp%d(): got zap'd result from waitDevice() call. ",
                       terminal);
        USLOSS_Console("Should not have happened!\n");
    }
    else if ( result == 0 )
        USLOSS_Console("XXp%d(): status = %d\n", terminal, status);
    else
        USLOSS_Console("XXp%d(): got %d instead of -1 or 0 from waitDevice\n",
                       terminal, result);

    USLOSS_Console("XXp%d(): receive status for terminal %d = %d\n",
                   terminal, terminal, USLOSS_TERM_STAT_RECV(status));
    USLOSS_Console("XXp%d(): character received = %c\n",
                   terminal, USLOSS_TERM_STAT_CHAR(status));

    quit(-3);
    return 0; /* so gcc will not complain about its absence... */

} /* XXp1 */
