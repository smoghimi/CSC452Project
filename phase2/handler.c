#include <stdio.h>
#include <phase1.h>
#include <phase2.h>
#include <usyscall.h>
#include "message.h"

extern int debugflag2;
int clockTimer = 5;

/* an error method to handle invalid syscalls */
void nullsys(systemArgs *args)
{

  USLOSS_Console("nullsys(): Invalid syscall %i. Halting...\n", args->number);
  USLOSS_Halt(1);
} /* nullsys */


void clockHandler2(int dev, void *arg)
{
  if (DEBUG2 && debugflag2)
    USLOSS_Console("clockHandler2(): called\n");

<<<<<<< HEAD
  timeSlice();									//Uses the time slice from phase1
  if (clockTimer%5 == 0){							//Every five iterations of the clock we send a msg
    int status, a;
    a = USLOSS_DeviceInput(USLOSS_CLOCK_INT, USLOSS_CLOCK_INT, &status);	//Because we don't have to worry about multiple boxes
    a = MboxCondSend(CLOCK_MBOX, &status, sizeof(int));				//we use the input at clock_int to get our status and send
    if (a == -10){
      USLOSS_Console("Something went wrong\n");
    }
    clockTimer = 5;								//Clock timer goes from 0 to 5 forever, so there's no overflow
=======
  timeSlice();
  if (clockTimer%5 == 0){
    int status, a;
    a = USLOSS_DeviceInput(USLOSS_CLOCK_INT, USLOSS_CLOCK_INT, &status);
    a = MboxCondSend(CLOCK_MBOX, &status, sizeof(int));
    clockTimer = 5;
>>>>>>> 452-Project-Shaion
  }
  clockTimer--;
} /* clockHandler */


void diskHandler(int dev, void *arg)
{
  if (DEBUG2 && debugflag2)
    USLOSS_Console("diskHandler(): called\n");
  int a, status;
  int blue = *(int*)arg;
  a = USLOSS_DeviceInput(dev, blue, &status);		//Because we are passed the dev and the argument, we de-ref and cast to int in order
  a = MboxCondSend(blue+5, &status, sizeof(int));	//to get a status, and then use the box based on the arg and send the satus address
  if (a == -10){
    USLOSS_Console("Something went wrong\n");
  }
} /* diskHandler */


<<<<<<< HEAD
void termHandler(int dev, int arg)
=======
void termHandler(int dev, void *arg)
>>>>>>> 452-Project-Shaion
{
  if (DEBUG2 && debugflag2)
    printf("termHandler\n");
  int a, status;
<<<<<<< HEAD
  int blue = arg;
  a = USLOSS_DeviceInput(dev, blue, &status);		//Same as the disk handler, but we iterate only past the clock mailbox for out arg
  a = MboxCondSend(blue + 1, &status, sizeof(int));
  if (a == -10){
    USLOSS_Console("Something went wrong\n");
  }
=======
  a = USLOSS_DeviceInput(dev, (int)arg, &status);
  a = MboxCondSend((int)arg, &status, sizeof(int));
>>>>>>> 452-Project-Shaion
} /* termHandler */


void syscallHandler(int dev, void *arg)
{
  systemArgs sysargs = *(systemArgs *)arg;		//De-ref and cast the argument to systemArgs type
  if (sysargs.number >= 50){
    USLOSS_Console("syscallHandler(): sys number %i is wrong.  Halting...\n", sysargs.number);
    USLOSS_Halt(1);
  }
  nullsys(arg);						//If the number of args is less than the max, then we (FOR NOW) simply call nullsys
  if (DEBUG2 && debugflag2)
    USLOSS_Console("syscallHandler(): called\n");
} /* syscallHandler */
