
/* checking for release: 3 instances of XXp2 send messages to a zero-slot
 * mailbox, which causes them to block. XXp4 then releases the mailbox.
 * All processes are at the same priority.
 */

#include <stdio.h>
#include <string.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

int XXp2(char *);
int XXp3(char *);
int XXp4(char *);
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
    int kid_status, kidpid, pausepid;

    USLOSS_Console("start2(): started\n");
    mbox_id  = MboxCreate(0, 50);
    USLOSS_Console("\nstart2(): MboxCreate returned id = %d\n", mbox_id);

    kidpid   = fork1("XXp2a", XXp2, "XXp2a", 2 * USLOSS_MIN_STACK, 3);
    kidpid   = fork1("XXp2b", XXp2, "XXp2b", 2 * USLOSS_MIN_STACK, 3);
    kidpid   = fork1("XXp2c", XXp2, "XXp2c", 2 * USLOSS_MIN_STACK, 3);
    pausepid = fork1("XXp4",  XXp4, "XXp4",  2 * USLOSS_MIN_STACK, 3);
    kidpid = join(&kid_status);
    if (kidpid != pausepid)
        USLOSS_Console("\n***Test Failed*** -- join with pausepid failed!\n\n");

    kidpid   = fork1("XXp3",  XXp3, NULL,    2 * USLOSS_MIN_STACK, 2);

    kidpid = join(&kid_status);
    USLOSS_Console("\nstart2(): joined with kid %d, status = %d\n",
                   kidpid, kid_status);

    kidpid = join(&kid_status);
    USLOSS_Console("\nstart2(): joined with kid %d, status = %d\n",
                   kidpid, kid_status);

    kidpid = join(&kid_status);
    USLOSS_Console("\nstart2(): joined with kid %d, status = %d\n",
                   kidpid, kid_status);

    kidpid = join(&kid_status);
    USLOSS_Console("\nstart2(): joined with kid %d, status = %d\n",
                   kidpid, kid_status);

    quit(0);
    return 0; /* so gcc will not complain about its absence... */
} /* start2 */


int XXp2(char *arg)
{
    int result;
    char buffer[20];

    sprintf(buffer, "hello from %s", arg);
    USLOSS_Console("%s(): sending message '%s' to mailbox %d, msg_size = %lu\n",
                   arg, buffer, mbox_id, strlen(buffer)+1);
    result = MboxSend(mbox_id, buffer, strlen(buffer)+1);
    USLOSS_Console("%s(): after send of message '%s', result = %d\n",
                   arg, buffer, result);

    if (result == -3)
        USLOSS_Console("%s(): zap'd by MboxSend() call\n", arg);

    quit(-3);
    return 0;

} /* XXp2 */


int XXp3(char *arg)
{
    int result;

    USLOSS_Console("XXp3(): started\n");
 
    result = MboxRelease(mbox_id);

    USLOSS_Console("XXp3(): MboxRelease returned %d\n", result);

    quit(-4);
    return 0;
} /* XXp3 */


int XXp4(char *arg)
{

    USLOSS_Console("XXp4(): started and quitting\n");
    quit(-4);

    return 0;
}
