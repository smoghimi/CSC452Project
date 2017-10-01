#include <stdio.h>
#include <phase1.h>
#include <phase2.h>
#include "message.h"

extern int debugflag2;

/* an error method to handle invalid syscalls */
void nullsys(sysargs *args)
{
    USLOSS_Console("nullsys(): Invalid syscall. Halting...\n");
    USLOSS_Halt(1);
} /* nullsys */


void clockHandler2(int dev, void *arg)
{

   if (DEBUG2 && debugflag2)
      USLOSS_Console("clockHandler2(): called\n");


} /* clockHandler */


void diskHandler(int dev, void *arg)
{

   if (DEBUG2 && debugflag2)
      USLOSS_Console("diskHandler(): called\n");


} /* diskHandler */


void termHandler(int dev, void *arg)
{

   if (DEBUG2 && debugflag2)
      USLOSS_Console("termHandler(): called\n");


} /* termHandler */


void syscallHandler(int dev, void *arg)
{

   if (DEBUG2 && debugflag2)
      USLOSS_Console("syscallHandler(): called\n");


} /* syscallHandler */
