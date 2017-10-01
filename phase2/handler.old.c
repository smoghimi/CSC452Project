#include <stdio.h>
#include <phase1.h>
#include <phase2.h>
#include <usloss.h>
#include "message.h"

extern int debugflag2;
int clockTimer = 5;

/* an error method to handle invalid syscalls */
extern void nullsys(systemArgs *args)
{
    USLOSS_Console("nullsys(): Invalid syscall. Halting...\n");
    USLOSS_Halt(1);
} /* nullsys */


extern void clockHandler2(int dev, int unit)
{
  if (DEBUG2 && debugflag2)
    USLOSS_Console("clockHandler2(): called\n");

  timeSlice();
  if (clockTimer%5 == 0){
    int status, a;
    a = USLOSS_DeviceInput(USLOSS_CLOCK_INT, USLOSS_CLOCK_INT, &status);
    a = MboxCondSend(CLOCK_MBOX, &status, 6);
    clockTimer = 5;
  }
  clockTimer--;
} /* clockHandler */


extern void diskHandler(int dev, int unit)
{
  if (DEBUG2 && debugflag2)
    USLOSS_Console("diskHandler(): called\n");


} /* diskHandler */


extern void termHandler(int dev, int unit)
{
  printf("term handler\n");
  if (DEBUG2 && debugflag2)
    USLOSS_Console("termHandler(): called\n");
} /* termHandler */


extern void syscallHandler(int dev, int unit)
{

   if (DEBUG2 && debugflag2)
      USLOSS_Console("syscallHandler(): called\n");


} /* syscallHandler */
