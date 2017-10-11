#include <stdio.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>


/* test overflowing the mailbox, releasing some, then overflowing again */

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}


int start2(char *arg)
{
    int mbox_id;
    int i, result;


    USLOSS_Console("start2(): started, trying to create too many ");
    USLOSS_Console("mailboxes...\n");

    // Create MAXMBOX - 5 mailboxes -- all should succeed
    for (i = 1; i <= MAXMBOX - 5; i++) {
        mbox_id = MboxCreate(10, 50);
        if (mbox_id < 0) {
            USLOSS_Console("start2(): MailBoxCreate returned id less than ");
            USLOSS_Console("zero, id = %d\n", mbox_id);
        }
    }


    // Release 6 of the mailboxes (boxes 10 to 15)
    for (i = 0; i < 6; i++) { 
        USLOSS_Console("start2(): calling MboxRelease(%d)\n", i);

        result = MboxRelease(10+i);

        USLOSS_Console("start2(): calling MboxRelease(%d) returned %d\n",
                       i, result);
    }


    //dumpMboxes();

    // Create 8 mailboxes, which be two mailboxes too many
    for (i = 0; i < 8; i++) {
        mbox_id = MboxCreate(10, 50);
        if (mbox_id < 0) {
            USLOSS_Console("start2(): MailBoxCreate for i = %d returned ", i);
            USLOSS_Console("id less than zero, id = %d\n", mbox_id);
        }
        else {
            USLOSS_Console("start2(): MailBoxCreate for i = %d returned ", i);
            USLOSS_Console("id = %d\n", mbox_id);
        }
    }


    quit(0);
    return 0; /* so gcc will not complain about its absence... */
}
