/*
 * Test Invalid disk and Terminal.
 */

#include <stdlib.h>
#include <string.h>
#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <libuser.h>
#include <assert.h>
#include <stdio.h>

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start4(char *arg)
{
   int  status;
   char buffer[80];

   USLOSS_Console("start4(): Attempt to write to a non-existant disk, disk 3\n");

   if (DiskWrite(buffer, 3, 1, 1, 1, &status) >= 0) {
      USLOSS_Console("start4(): Disk : Should not see this!!!\n");
   }

   if (DiskWrite(buffer, 0, 17, 1, 1, &status) >= 0) {
      USLOSS_Console("start4(): Disk : Should not see this!!!\n");
   }

   if (DiskWrite(buffer, 0, 1, 17, 1, &status) >= 0) {
      USLOSS_Console("start4(): Disk : Should not see this!!!\n");
   }

   USLOSS_Console("start4(): done\n");

   Terminate(8);
   return 0;

} /* start4 */
