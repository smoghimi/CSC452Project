
/* Easy getting started test.
 * start2 prints a message and quits.
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

  printf("start2(): started\n");
  quit(0);
  return 0; /* so gcc will not complain about its absence... */
}
