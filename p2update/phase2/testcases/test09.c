
/* Creates a 5-slot mailbox. Creates XXp1 that sends five messages to the
 * mailbox, then terminates. Creates XXp2a,b,c each of which sends a
 * message to the mailbox and gets blocked since the box is full.
 * Creates XXp3, which releases the mailbox, freeing the 5 slots and
 * unblocking XXp2a,b,c.  Each of XXp2a,b,c should return -3 from their
 * MboxReceive calls.  Note that XXp2a,b,c are lower priority than
 * XXp3, which is releasing the mailbox.
 */


#include <stdio.h>
#include <string.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

int XXp1(char *);
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

   printf("start2(): started\n");
   mbox_id  = MboxCreate(5, 50);
   printf("start2(): MboxCreate returned id = %d\n", mbox_id);

   kidpid = fork1("XXp1",  XXp1, NULL,    2 * USLOSS_MIN_STACK, 3);
   kidpid = join(&kid_status);
   printf("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);
   kidpid   = fork1("XXp2a", XXp2, "XXp2a", 2 * USLOSS_MIN_STACK, 3);
   kidpid   = fork1("XXp2b", XXp2, "XXp2b", 2 * USLOSS_MIN_STACK, 3);
   kidpid   = fork1("XXp2c", XXp2, "XXp2c", 2 * USLOSS_MIN_STACK, 3);
   pausepid = fork1("XXp4",  XXp4, "XXp4",  2 * USLOSS_MIN_STACK, 3);
   kidpid = join(&kid_status);
   if (kidpid != pausepid)
      printf("\n***Test Failed*** -- join with pausepid failed!\n\n");

   kidpid   = fork1("XXp3",  XXp3, NULL,    2 * USLOSS_MIN_STACK, 2);

   kidpid = join(&kid_status);
   printf("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

   kidpid = join(&kid_status);
   printf("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

   kidpid = join(&kid_status);
   printf("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

   kidpid = join(&kid_status);
   printf("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

   quit(0);
   return 0; /* so gcc will not complain about its absence... */
} /* start2 */


int XXp1(char *arg)
{
   int i, result;
   char buffer[20];

   printf("XXp1(): started\n");

   for (i = 0; i < 5; i++) {
      printf("XXp1(): sending message #%d to mailbox %d\n", i, mbox_id);
      sprintf(buffer, "hello there, #%d", i);
      result = MboxSend(mbox_id, buffer, strlen(buffer)+1);
      printf("XXp1(): after send of message #%d, result = %d\n", i, result);
   }

   quit(-3);
   return 0; /* so gcc will not complain about its absence... */
} /* XXp1 */


int XXp2(char *arg)
{
   int result;
   char buffer[20];

   sprintf(buffer, "hello from %s", arg);
   printf("%s(): sending message '%s' to mailbox %d, msg_size = %lu\n",
          arg, buffer, mbox_id, strlen(buffer)+1);
   result = MboxSend(mbox_id, buffer, strlen(buffer)+1);
   printf("%s(): after send of message '%s', result = %d\n",
          arg, buffer, result);

   if (result == -3)
      printf("%s(): zap'd by MboxSend() call\n", arg);

   quit(-3);
   return 0;

} /* XXp2 */


int XXp3(char *arg)
{
   int result;

   printf("XXp3(): started\n");

   result = MboxRelease(mbox_id);

   printf("XXp3(): MboxRelease returned %d\n", result);

   quit(-4);
   return 0;
} /* XXp3 */


int XXp4(char *arg)
{

   printf("XXp4(): started and quitting\n");
   quit(-4);

   return 0;
}
