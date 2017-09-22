
/* testing for big messages to be rejected */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

int XXp1(char *);
char buf[256];

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start2(char *arg)
{
    int mbox_id;
    int result;
    char buffer[80];

    USLOSS_Console("start2(): started\n");
    mbox_id = MboxCreate(10, 5);
    USLOSS_Console("start2(): MboxCreate returned id = %d\n", mbox_id);

    USLOSS_Console("start2(): sending message to mailbox %d\n", mbox_id);
    result = MboxSend(mbox_id, "hello there", 12);
    USLOSS_Console("start2(): after send of message, result = %d\n", result);

    if(result == -1){
        USLOSS_Console("message too big\n");
        quit(0);
        return 0;
    }

    USLOSS_Console("start2(): attempting to receive message from mailbox %d\n",
                   mbox_id);
    result = MboxReceive(mbox_id, buffer, 80);
    USLOSS_Console("start2(): after receive of message, result = %d\n", result);
    USLOSS_Console("          message is `%s'\n", buffer);

    quit(0);
    return 0; /* so gcc will not complain about its absence... */
}

int XXp1(char *arg)
{
    int i;

    USLOSS_Console("XXp1(): started\n");
    USLOSS_Console("XXp1(): arg = `%s'\n", arg);
    for(i = 0; i < 100; i++)
        ;
    quit(-3);
    return 0;
}
