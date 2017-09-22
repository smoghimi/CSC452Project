
/* Creates two children.  Higher priority child does a receive, and should
 * block.  Lower priority child then does a send and should unblock the
 * higher priority child.
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>
#include <string.h>  /* so we can use strlen() */

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

  printf("start2(): started\n");
  mbox_id = MboxCreate(10, 50);
  printf("start2(): MboxCreate returned id = %d\n", mbox_id);

  kidpid = fork1("XXp1", XXp1, NULL, 2 * USLOSS_MIN_STACK, 4);
  kidpid = fork1("XXp2", XXp2, NULL, 2 * USLOSS_MIN_STACK, 3);

  kidpid = join(&kid_status);
  printf("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

  kidpid = join(&kid_status);
  printf("start2(): joined with kid %d, status = %d\n", kidpid, kid_status);

  quit(0);
  return 0; /* so gcc will not complain about its absence... */
} /* start2 */


int XXp1(char *arg)
{
  int result;

  printf("XXp1(): started\n");
  printf("XXp1(): sending message to mailbox %d\n", mbox_id);
  result = MboxSend(mbox_id, "hello there", strlen("hello there")+1);
  printf("XXp1(): after send of message, result = %d\n", result);

  quit(-3);
  return 0;
} /* XXp1 */


int XXp2(char *arg)
{
  char buffer[100];
  int result;

  printf("XXp2(): started\n");
  printf("XXp2(): receiving message from mailbox %d\n", mbox_id);
  result = MboxReceive(mbox_id, buffer, 100);
  printf("XXp2(): after receipt of message, result = %d\n", result);
  printf("        message = `%s'\n", buffer);

  quit(-4);
  return 0;
} /* XXp2 */
