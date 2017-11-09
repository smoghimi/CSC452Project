/* KILLER
 * Spawn 5 children to sleep
 * Spawn 2 children ... 1 to read 1 line from each terminal and one to write one line to each terminal
 * Spawn 4 children ....
 * 1 to write to disk0 1 to read from disk0
 * 1 to write to disk1 1 to read from disk1
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

int ChildS(char *arg)
{
   int tod1,tod2;
   int my_num = atoi(arg);
   int seconds;

   seconds = (5 - my_num);

   USLOSS_Console("ChildS(%d): Sleeping for %d seconds\n", my_num, seconds);
   GetTimeofDay(&tod1);
   Sleep(seconds);
   GetTimeofDay(&tod2);
   USLOSS_Console("ChildS(%d): After sleeping %d seconds, diff in sys_clock is %d\n",
          my_num, seconds, tod2-tod1);

   Terminate(10 + my_num);

   return 0;
} /* ChildS */

int ChildTR(char *arg)
{
   char buf[80] = "";
   int read_length;
   int i;

   USLOSS_Console("ChildTR(): start\n");

   for (i=0; i<4; i++) {
      if (TermRead(buf, 80, i, &read_length) < 0) {
         USLOSS_Console("ChildTR(): ERROR: ReadTerm\n");
         return -1;
      }
      else {
         USLOSS_Console("ChildTR(): terminal %d, read_length = %d\n", i, read_length);
         buf[read_length] = '\0';
         USLOSS_Console("ChildTR(): read from term%d: read %s", i, buf);
      }
   }

   USLOSS_Console("ChildTR(): done\n");

   Terminate(0);
   return 0;
} /* ChildTR */

int ChildTW(char *arg)
{
   char buffer[MAXLINE];
   int  result, size;
   int  i;

   for (i=0; i<4; i++) {
      sprintf(buffer,
              "ChildTW(): A Something interesting to print to term %d ...\n",
              i);
      result = TermWrite(buffer, strlen(buffer), i, &size);
      if (result < 0 || size != strlen(buffer)) {
          USLOSS_Console("\n ***** ChildTW(%d): got bad result or bad size! *****\n\n",
                 i);
          USLOSS_Console("result = %d, size = %d, strlen(buffer) = %d\n",
                 result, size, strlen(buffer));
      }
   }
   Terminate(1);

   USLOSS_Console("ChildTW(): should not see this message!\n");
   return 1;
} /* ChildTW */

int ChildDW0(char *arg)
{
   int status;
   char disk_buf_A[512];

   USLOSS_Console("\nChildDW0(): writing to disk 0, track 5, sector 0\n");
   sprintf(disk_buf_A, "ChildDW0(): A wonderful message to put on the disk...");
   DiskWrite(disk_buf_A, 0, 5, 0, 1, &status);
   USLOSS_Console("ChildDW0(): DiskWrite0 returned status = %d\n", status);
   return 0;
} /* ChildDW0 */

int ChildDW1(char *arg)
{
   int status;
   char disk_buf_A[512];

   USLOSS_Console("\nChildDW1(): writing to disk 1, track 5, sector 0\n");
   sprintf(disk_buf_A, "ChildDW1(): A wonderful message to put on the disk...");
   DiskWrite(disk_buf_A, 1, 5, 0, 1, &status);
   USLOSS_Console("ChildDW1(): DiskWrite1 returned status = %d\n", status);
   return 0;
} /* ChildDW1 */

int ChildDR0(char *arg)
{
   int status;
   char disk_buf_B[512];

   USLOSS_Console("\nChildR0(): reading from disk 0, track 5, sector 0\n");
   DiskRead(disk_buf_B, 0, 5, 0, 1, &status);
   USLOSS_Console("ChildR0(): DiskRead returned status = %d\n", status);
   USLOSS_Console("ChildR0(): disk_buf_B contains:\n%s\n", disk_buf_B);
   return 0;
} /* ChildDR0 */

int ChildDR1(char *arg)
{
   int status;
   char disk_buf_B[512];

   USLOSS_Console("\nChildR1(): reading from disk 1, track 5, sector 0\n");
   DiskRead(disk_buf_B, 1, 5, 0, 1, &status);
   USLOSS_Console("ChildR1(): DiskRead returned status = %d\n", status);
   USLOSS_Console("ChildR1(): disk_buf_B contains:\n%s\n", disk_buf_B);
   return 0;
} /* ChildDR1 */

int start4(char *arg)
{
   int  pid, status, i;
   char buf[12];
   char name[] = "ChildS";

   USLOSS_Console("start4(): Spawning 5 children to sleep\n");
   for (i = 0; i < 5; i++) {
      sprintf(buf, "%d", i);
      name[5] = buf[0];
      status = Spawn(name, ChildS, buf, USLOSS_MIN_STACK,2, &pid);
   }

   USLOSS_Console("start4(): Spawning 2 children to termfuncs\n");
   status = Spawn("ChildTR", ChildTR, NULL, USLOSS_MIN_STACK,2, &pid);
   status = Spawn("ChildTW", ChildTW, NULL, USLOSS_MIN_STACK,2, &pid);

   USLOSS_Console("start4(): Spawning 4 children to diskfuncs\n");
   status = Spawn("ChildDW0", ChildDW0, NULL, USLOSS_MIN_STACK,2, &pid);
   status = Spawn("ChildDW1", ChildDW1, NULL, USLOSS_MIN_STACK,2, &pid);
   status = Spawn("ChildDR0", ChildDR0, NULL, USLOSS_MIN_STACK,4, &pid);
   status = Spawn("ChildDR1", ChildDR1, NULL, USLOSS_MIN_STACK,4, &pid);

   for (i = 0; i < 11; i++) {
      Wait(&pid, &status);
   }

   USLOSS_Console("start4(): done.\n");
   Terminate(1);
   return 0;
} /* start4 */
