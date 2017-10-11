
/* start2 attempts to create MAXMBOX + 3 mailboxes.
 * the last 3 should return -1
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start2(char *arg)
{
  int mbox_id;
  int i;

  printf("start2(): started, trying to create too many mailboxes...\n");
  for (i = 0; i < MAXMBOX + 3; i++) {
    mbox_id = MboxCreate(10, 50);
    if (mbox_id < 0)
      printf("start2(): MailBoxCreate returned id less than zero, id = %d\n",
              mbox_id);
  }

  quit(0);
  return 0; /* so gcc will not complain about its absence... */
}
