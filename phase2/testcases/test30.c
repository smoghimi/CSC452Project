/* test releasing a mailbox with a number of blocked senders (all of
 * various higher or equal priorities than the releaser),

 * and then trying to receive
 * and send to the now unused mailbox.
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

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start2(char *arg)
{
  int status;
  int result;
  int kidpid;

  printf("start2(): started\n");
  mbox_id = MboxCreate(1, 13);
  printf("start2(): MboxCreate returned id = %d\n", mbox_id);

  kidpid = fork1("XXp1", XXp1, NULL, 2 * USLOSS_MIN_STACK, 4);
  kidpid = fork1("XXp2", XXp2, NULL, 2 * USLOSS_MIN_STACK, 3);
  kidpid = fork1("XXp3", XXp3, NULL, 2 * USLOSS_MIN_STACK, 2);
  kidpid = fork1("XXp4", XXp4, NULL, 2 * USLOSS_MIN_STACK, 4);

  printf("start2(): sending message to mailbox %d\n", mbox_id);
  result = MboxSend(mbox_id, "hello there", 12);
  printf("start2(): after send of message, result = %d\n\n", result);

  join(&status);
  join(&status);
  join(&status);
  join(&status);

  result = kidpid;  // to avoid warning about kidpid set but not used

  quit(0);
  return 0; /* so gcc will not complain about its absence... */

} /* start2 */


int XXp1(char *arg)
{

   int result;

   printf("XXp1(): sending message to mailbox %d\n", mbox_id);
   result = MboxSend(mbox_id, "hello there", 12);
   printf("XXp1(): after send of message, result = %d\n", result);

   quit(-3);
   return 0;
} /* XXp1 */


int XXp2(char *arg)
{

   int result;

   printf("XXp2(): sending message to mailbox %d\n", mbox_id);
   result = MboxSend(mbox_id, "hello there", 12);
   printf("XXp2(): after send of message, result = %d\n", result);

   quit(-3);
   return 0;
} /* XXp2 */


int XXp3(char *arg)
{

   int result;

   printf("XXp3(): sending message to mailbox %d\n", mbox_id);
   result = MboxSend(mbox_id, "hello there", 12);
   printf("XXp3(): after send of message, result = %d\n", result);

   quit(-3);
   return 0;

} /* XXp3 */


int XXp4(char *arg)
{

   int result;

   printf("\nXXp4(): Releasing MailBox %d\n", mbox_id);
   result = MboxRelease(mbox_id);
   printf("\nXXp4(): after release of mailbox, result = %d\n", result);

   quit(-3);
   return 0;

} /* XXp4 */
