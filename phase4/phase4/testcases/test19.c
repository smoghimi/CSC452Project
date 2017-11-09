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
   char buf[MAXLINE] = "";
   int read_length;

   USLOSS_Console("Child%d(): start\n", term);

   if (TermRead(buf, MAXLINE, term, &read_length) < 0) {
      USLOSS_Console("ERROR: ReadTeam\n");
      return -1;
   }
   else {
      buf[read_length] = '\0';
      USLOSS_Console("Child%d(): read %s", term, buf);
      return 0;
   }

   USLOSS_Console("Child%d(): done\n", term);

   Terminate(0);
   return 0;
} /* Child 1 */

int Child2(char *arg)
{
   char buffer[MAXLINE];
   int  result, size;
   int  unit = atoi(arg);

   sprintf(buffer, "Child %d: A Something interesting to print here...\n",
           unit);

   //result = TermWrite(buffer, MAXLINE, unit, &size);
   result = TermWrite(buffer, strlen(buffer), unit, &size);
   if (result < 0 || size != strlen(buffer)) {
      USLOSS_Console("\n ***** Child(%d): got bad result or bad size! *****\n\n ",
             unit);
      USLOSS_Console("result = %d size = %d and bufferlength = %d\n",
             result, size, strlen(buffer));
   }

   Terminate(0);

   USLOSS_Console("Child(%d): should not see this message!\n", unit);
   return 1;
} /* Child2 */

int start4(char *arg)
{
   int  pid, status, i;
   char buf[12];
   char child_buf[12];

   USLOSS_Console("start4(): Spawn four children.  Each child reads fm a different\n");
   USLOSS_Console("          terminal.  The child reading the shortest line will\n");
   USLOSS_Console("          finish first, etc.\n");
   USLOSS_Console("start4(): Spawn four children.  Each child writes to a different\n");
   USLOSS_Console("          terminal.\n");

   for (i = 0; i < 4; i++) {
      sprintf(buf, "%d", i);
      sprintf(child_buf, "Child%d", i);
      status = Spawn(child_buf, Child1, buf, USLOSS_MIN_STACK,2, &pid);
      sprintf(child_buf, "Child%d", i+4);
      assert(status == 0);
      status = Spawn(child_buf, Child2, buf, USLOSS_MIN_STACK,2, &pid);
      assert(status == 0);
   }

   for (i = 0; i < 8; i++) {
      Wait(&pid, &status);
      assert(status == 0);
   }

   USLOSS_Console("start4(): done.\n");
   Terminate(1);
   return 0;
} /* start4 */
