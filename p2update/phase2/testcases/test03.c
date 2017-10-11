
/* start2 creates a mailbox, sends a message to the mailbox,
 * then receives the message.
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

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

  printf("start2(): started\n");
  mbox_id = MboxCreate(10, 50);
  printf("start2(): MboxCreate returned id = %d\n", mbox_id);

  printf("start2(): sending message to mailbox %d\n", mbox_id);
  result = MboxSend(mbox_id, "hello there", 12);
  printf("start2(): after send of message, result = %d\n", result);

  printf("start2(): attempting to receive message from mailbox %d\n", mbox_id);
  result = MboxReceive(mbox_id, buffer, 80);
  printf("start2(): after receive of message, result = %d\n", result);
  printf("          message is `%s'\n", buffer);

  quit(0);
  return 0; /* so gcc will not complain about its absence... */
}
