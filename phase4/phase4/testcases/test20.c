/* TERMTEST
 * Spawn off 8 children. The children either read a line each from each
 * terminal, or write a line each to each terminal.
 */

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
   int i;

   USLOSS_Console("Child%d(): start\n", term);

   for (i = 0; i< 5; i++){
      if (TermRead(buf, MAXLINE, term, &read_length) < 0) {
         USLOSS_Console("ERROR: ReadTeam\n");
         return -1;
      }
      else {
         buf[read_length] = '\0';
         USLOSS_Console("Child%d(): read %s", term, buf);
      }
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
   int  i;

   USLOSS_Console("Child_2%d(): start\n", unit);
   for (i = 0; i< 5; i++) {
      sprintf(buffer,
              "Child %d: A Something interesting to print here... line %d ",
              unit, i);
      switch(unit) {
      case 0:
         strcat(buffer, "zero\n");
         break;
      case 1:
         strcat(buffer, "one\n");
         break;
      case 2:
         strcat(buffer, "second\n");
         break;
      case 3:
         strcat(buffer, "three\n");
         break;
      }
      USLOSS_Console("Child_2%d(): writing to term%d\n", unit,unit);

      result = TermWrite(buffer, strlen(buffer), unit, &size);
      if (result < 0 || size != strlen(buffer))
         USLOSS_Console("\n ***** Child(%d): got bad result or bad size! *****\n\n",
                unit);
   }

   Terminate(1);

   USLOSS_Console("Child(%d): should not see this message!\n", unit);
   return 1;
} /* Child2 */

int start4(char *arg)
{
   int  pid, status, i;
   char buf[12];
   char child_buf[12];

   USLOSS_Console("start4(): Spawn eight children.  \n");
   USLOSS_Console("          4 write 5 lines to a diff terminal.\n");
   USLOSS_Console("          4 read 5 lines to a diff terminal.\n");

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
      //assert(status == 0);
   }

   USLOSS_Console("start4(): done.\n");
   Terminate(1);
   return 0;
} /* start4 */
