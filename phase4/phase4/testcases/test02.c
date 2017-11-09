#include <stdio.h>
#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>
#include <phase4.h>
#include <libuser.h>
#include <assert.h>
#include <stdlib.h>

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int Child(char *arg)
{
   int tod1,tod2;
   int my_num = atoi(arg);
   int seconds;

   seconds = (10 - my_num) * 3;

   USLOSS_Console("Child%d(): Sleeping for %d seconds\n", my_num, seconds);
   GetTimeofDay(&tod1);
   Sleep(seconds);
   GetTimeofDay(&tod2);
   USLOSS_Console("Child%d(): After sleeping %d seconds, difference ",
                  my_num, seconds);
   USLOSS_Console("in system clock is %d\n", tod2-tod1);

  Terminate(10 + my_num);

  return 0;
}


int start4(char *arg)
{
   int i,cpid,id,result;

   char carg[10];
   char name[] = "Child0";

   for(i = 0; i < 10; i++) {
      sprintf(carg, "%d", i);
      name[5] = carg[0];

      USLOSS_Console("start4(): Spawning Child(%d)\n", i);
      Spawn(name, Child, carg, USLOSS_MIN_STACK, 3, &cpid);
   }


   for(i=0; i < 10; i++) {
      USLOSS_Console("start4(): Waiting on Child\n");
      result = Wait(&cpid, &id);
      USLOSS_Console("start4(): Wait returned %d, pid:%d, status %d\n",
              result,cpid,id);
   }

   Terminate(0);

   return 0;
}
