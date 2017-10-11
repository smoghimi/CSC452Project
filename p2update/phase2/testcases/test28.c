/* checking for release: 3 instances of XXp2 receive messages from a "zero-slot"
 * mailbox, which causes them to block. XXp3 then releases the mailbox.
 * XXp3 is lower priority than the 3 blocked processes.
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
    int kid_status, kidpid;
    char buffer[20];
    int result;

    USLOSS_Console("start2(): started\n");
    mbox_id  = MboxCreate(0, 50);
    USLOSS_Console("\nstart2(): MboxCreate returned id = %d\n", mbox_id);

    kidpid   = fork1("XXp2a", XXp2, "XXp2a", 2 * USLOSS_MIN_STACK, 2);
    kidpid   = fork1("XXp2b", XXp2, "XXp2b", 2 * USLOSS_MIN_STACK, 2);
    kidpid   = fork1("XXp2c", XXp2, "XXp2c", 2 * USLOSS_MIN_STACK, 2);
    kidpid   = fork1("XXp3",  XXp3, NULL,    2 * USLOSS_MIN_STACK, 4);

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

    result = MboxCondReceive(mbox_id, buffer, sizeof(buffer));

    if (result == -1)
        USLOSS_Console("failed to recv from released mailbox ... success\n");
    else
        USLOSS_Console("test failed result = %d\n",result);

    quit(0);
    return 0; /* so gcc will not complain about its absence... */

} /* start2 */


int XXp2(char *arg)
{
    int result;
    char buffer[20];

    USLOSS_Console("%s(): receiving message from mailbox %d, msg_size = %d\n",
                   arg, mbox_id, sizeof(buffer));
    result = MboxReceive(mbox_id, buffer, sizeof(buffer));
    USLOSS_Console("%s(): after receive of message, result = %d\n",
                   arg, result);

    if (result == -3)
        USLOSS_Console("%s(): zap'd by MboxReceive() call\n", arg);

    quit(-3);
    return 0;

} /* XXp2 */


int XXp3(char *arg)
{
    int result;

    USLOSS_Console("\nXXp3(): started, releasing mailbox %d\n", mbox_id);

    result = MboxRelease(mbox_id);

    USLOSS_Console("XXp3(): MboxRelease returned %d\n", result);

    quit(-4);
    return 0;
} /* XXp3 */
