/* TERMTEST
 * Spawn off 4 children. 
 * Each child reads one line from a different terminal.
 * Child0 reads from term0.in
 * Child1 reads from term1.in
 * etc.
 */

#include <stdio.h>
#include <usloss.h>
#include <usyscall.h>
#include <libuser.h>
#include <assert.h>
#include <phase1.h>
#include <phase2.h>
#include <stdlib.h>

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int Child(char *arg) 
{
    int term = atoi(arg);
    char buf[80] = "";
    int len, read_length;
  
    USLOSS_Console("Child%d(): start\n", term);

    len = 80; /* state the size of our buffer */

    if (TermRead(buf, len, term, &read_length) < 0) {
        USLOSS_Console("Child%d(): ERROR -- TermRead\n", term);
        return -1;
    }
    else {
        buf[read_length] = '\0';
        USLOSS_Console("Child%d(): read %s", term, buf);
        return 0;
    }
  
    USLOSS_Console("Child%d(): done\n", term);

    Terminate(0);
    return 0;
} /* Child */


int start4(char *arg)
{
    int  pid, status, i;
    char buf[12];
    char child_buf[12];

    USLOSS_Console("start4(): Spawn four children.  Each child reads fm a\n");
    USLOSS_Console("          different terminal.  The child reading the\n");
    USLOSS_Console("          shortest line will finish first, etc.\n");

    for (i = 0; i < 4; i++) {
        sprintf(buf, "%d", i);
        sprintf(child_buf, "Child%d", i);
        status = Spawn(child_buf, Child, buf, USLOSS_MIN_STACK, 4, &pid);
        assert(status == 0);
    }

    for (i = 0; i < 4; i++) {
        Wait(&pid, &status);
        assert(status == 0);
    }

    USLOSS_Console("start4(): done.\n");
    Terminate(1);
    return 0;

} /* start4 */
