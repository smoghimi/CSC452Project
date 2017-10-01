#include <stdio.h>
#include <phase1.h>
#include <phase2.h>
#include "message.h"

extern int debugflag2;
int clockTimer = 5;

/* an error method to handle invalid syscalls */
void nullsys(systemArgs *args)
{
    USLOSS_Console("nullsys(): Invalid syscall. Halting...\n");
    USLOSS_Halt(1);
} /* nullsys */


void clockHandler2(int dev, void *arg)
{
  if (DEBUG2 && debugflag2)
    USLOSS_Console("clockHandler2(): called\n");

  timeSlice();
  if (clockTimer%5 == 0){
    int status, a;
    a = USLOSS_DeviceInput(USLOSS_CLOCK_INT, USLOSS_CLOCK_INT, &status);
    a = MboxCondSend(CLOCK_MBOX, &status, sizeof(int));
    clockTimer = 5;
  }
  clockTimer--;
} /* clockHandler */


void diskHandler(int dev, void *arg)
{

   if (DEBUG2 && debugflag2)
      USLOSS_Console("diskHandler(): called\n");


} /* diskHandler */


void termHandler(int dev, void *arg)
{
  if (DEBUG2 && debugflag2)
    printf("termHandler\n");
  int a, status;
  a = USLOSS_DeviceInput(dev, (int)arg, &status);
  a = MboxCondSend((int)arg, &status, sizeof(int));
} /* termHandler */


void syscallHandler(int dev, void *arg)
{

   if (DEBUG2 && debugflag2)
      USLOSS_Console("syscallHandler(): called\n");


} /* syscallHandler */
