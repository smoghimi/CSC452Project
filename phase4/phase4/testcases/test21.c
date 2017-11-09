#include <stdio.h>
#include <usloss.h>
#include <usyscall.h>
#include <libuser.h>
#include <assert.h>
#include <phase1.h>
#include <phase2.h>
#include <stdlib.h>
#include <string.h>

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int Child1(char *arg)
{
   int term = atoi(arg);
   char buf[MAXLINE + 1] = "";
   char buffer[MAXLINE + 1];
   int result, read_length;
   int i, size;

   USLOSS_Console("Child%d(): start\n", term);

   for (i = 0; i< 10; i++) {
      if (TermRead(buf, MAXLINE, term, &read_length) < 0) {
         USLOSS_Console("ERROR: ReadTeam\n");
         return -1;
      }
      else {
         buf[read_length] = '\0';
         sprintf(buffer, "%s", buf);
         USLOSS_Console("Child%d(): buffer read from term%d ==== %s",term, term, buf);
         result = TermWrite(buffer, strlen(buffer), term, &size);
         if (result < 0 || size != strlen(buffer)) {
             USLOSS_Console("\n ***** Child(%d): got bad result = %d ", term, result);
             USLOSS_Console("or bad size = %d! *****\n\n ", size);
         }
      }
   }

   USLOSS_Console("Child%d(): done\n", term);

   Terminate(0);
   return 0;
} /* Child 1 */

int start4(char *arg)
{
   int  pid, status;
   char buf[12];
   char child_buf[12];

   USLOSS_Console("start4(): Spawn one child.\n");

   sprintf(buf, "%d", 0);
   sprintf(child_buf, "Child%d", 0);
   status = Spawn(child_buf, Child1, buf, USLOSS_MIN_STACK,2, &pid);
   assert(status == 0);

   Wait(&pid, &status);
   assert(status == 0);

   USLOSS_Console("start4(): done.\n");
   Terminate(1);
   return 0;
} /* start4 */
