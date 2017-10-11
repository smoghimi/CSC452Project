/* Creates two children.  Higher priority child does a receive, and should
 * block.  Lower priority child then does a send and should unblock the
 * higher priority child.
 */

#include <stdio.h>
#include <string.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

int XXp1(char *);
int XXp2(char *);
char buf[256];

int mbox_id;

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start2(char *arg)
{
    int kid_status, kidpid;

    USLOSS_Console("start2(): started\n");
    mbox_id = MboxCreate(0, 50);
    USLOSS_Console("\nstart2(): MboxCreate returned id = %d\n", mbox_id);

    kidpid = fork1("XXp1", XXp1, NULL, 2 * USLOSS_MIN_STACK, 4);
    kidpid = fork1("XXp2", XXp2, NULL, 2 * USLOSS_MIN_STACK, 3);

    kidpid = join(&kid_status);
    USLOSS_Console("\nstart2(): joined with kid %d, status = %d\n",
                   kidpid, kid_status);

    kidpid = join(&kid_status);
    USLOSS_Console("\nstart2(): joined with kid %d, status = %d\n",
                   kidpid, kid_status);

    quit(0);
    return 0; /* so gcc will not complain about its absence... */
} /* start2 */


int XXp1(char *arg)
{
    int i, result;
    char buffer[20];

    USLOSS_Console("XXp1(): started\n");

    for (i = 0; i <= 1; i++) {
        USLOSS_Console("XXp1(): sending message #%d to mailbox %d\n",
                       i, mbox_id);
        sprintf(buffer, "hello there, #%d", i);
        result = MboxSend(mbox_id, buffer, strlen(buffer)+1);
        USLOSS_Console("XXp1(): after send of message #%d, result = %d\n",
                       i, result);
    }

    quit(-3);
    return 0;
} /* XXp1 */


int XXp2(char *arg)
{
    char buffer[100];
    int i, result;

    USLOSS_Console("XXp2(): started\n");

    for (i = 0; i <= 1; i++) {
        USLOSS_Console("XXp2(): receiving message #%d from mailbox %d\n",
                       i, mbox_id);
        result = MboxReceive(mbox_id, buffer, 100);
        USLOSS_Console("XXp2(): after receipt of message #%d, result = %d\n",
                       i, result);
        USLOSS_Console("        message = `%s'\n", buffer);
    }

    quit(-4);
    return 0;
} /* XXp2 */
