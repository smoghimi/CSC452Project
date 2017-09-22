/* test releasing a mailbox with a number of blocked receivers (all of
 * various lower or equal priorities than the releaser).
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

int XXp1(char *);
int XXp2(char *);
int XXp3(char *);
int XXp4(char *);

char buf[256];
int mbox_id;
char buffer[11];

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start2(char *arg)
{
    int result;
    int kidpid;
    int status;

    USLOSS_Console("start2(): started\n");
    mbox_id = MboxCreate(1, 13);
    USLOSS_Console("start2(): MboxCreate returned id = %d\n", mbox_id);

    kidpid = fork1("XXp1", XXp1, NULL, 2 * USLOSS_MIN_STACK, 4);
    kidpid = fork1("XXp2", XXp2, NULL, 2 * USLOSS_MIN_STACK, 3);
    kidpid = fork1("XXp3", XXp3, NULL, 2 * USLOSS_MIN_STACK, 2);
    kidpid = fork1("XXp4", XXp4, NULL, 2 * USLOSS_MIN_STACK, 4);

    USLOSS_Console("start2(): sending message to mailbox %d\n", mbox_id);
    result = MboxReceive(mbox_id, buffer, 12);

    join(&status);
    join(&status);
    join(&status);
    join(&status);
    USLOSS_Console("start2(): after send of message, result = %d\n", result);

    result = kidpid; // to avoid warning about kidpid set but not used

    quit(0);
    return 0; /* so gcc will not complain about its absence... */

} /* start2 */


int XXp1(char *arg)
{

    int result;

    USLOSS_Console("XXp1(): receiving message to mailbox %d should block\n",
                   mbox_id);
    result = MboxReceive(mbox_id, buffer, 12);
    USLOSS_Console("XXp1(): after send of message, result = %d\n", result);

    quit(-3);
    return 0;
} /* XXp1 */


int XXp2(char *arg)
{

    int result;

    USLOSS_Console("XXp2(): receiving message from mailbox %d should block\n",
                   mbox_id);
    result = MboxReceive(mbox_id, buffer, 12);
    USLOSS_Console("XXp2(): after send of message, result = %d\n", result);

    quit(-3);
    return 0;

} /* XXp2 */


int XXp3(char *arg)
{

    int result;

    USLOSS_Console("XXp3(): receiving message from mailbox %d, should block\n",
                   mbox_id);
    result = MboxReceive(mbox_id, buffer, 12);
    USLOSS_Console("XXp3(): after send of message, result = %d\n", result);

    quit(-3);
    return 0;

} /* XXp3 */


int XXp4(char *arg)
{

  int result;

  USLOSS_Console("XXp4(): Releasing Mailbox %d\n", mbox_id);
  result = MboxRelease(mbox_id);
  USLOSS_Console("XXp4(): after release of mailbox, result = %d\n", result);

  quit(-3);
  return 0;

} /* XXp4 */
