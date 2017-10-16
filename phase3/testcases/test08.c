/*
 * Sem Free + Max Sem Create test.
 */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>
#include <libuser.h>
#include <stdio.h>

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start3(char *arg)
{
   int semaphore[MAXSEMS+1];
   int sem_result;
   int i;

   USLOSS_Console("start3(): started.  Calling SemCreate\n");
   for (i = 0; i < MAXSEMS; i++) {
      sem_result = SemCreate(0, &semaphore[i]);
      if (sem_result == -1)
         USLOSS_Console("start3(): i = %3d, sem_result = %2d\n", i, sem_result);
   }

   sem_result = SemCreate(0, &semaphore[MAXSEMS]);

   if (sem_result != -1)
      USLOSS_Console("start3(): ERROR: sem_result should have been -1, but was not\n");

   USLOSS_Console("start3(): freeing one semaphore\n");
   sem_result = SemFree(semaphore[105]);

   if (sem_result != 0)
      USLOSS_Console("start3(): ERROR: SemFree should have returned -1, but did not\n");

   sem_result = SemCreate(0, &semaphore[MAXSEMS]);

   if (sem_result == 0)
      USLOSS_Console("start3(): Correct result from last call to SemCreate()\n");
   else {
      USLOSS_Console("start3(): ERROR: last call to SemCreate should have ");
      USLOSS_Console("returned 0, but did not\n");
   }
   
   Terminate(8);

   return 0;
} /* start3 */
