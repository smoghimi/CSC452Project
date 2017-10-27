#include <stdio.h>
#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>
#include <phase4.h>
#include <stdlib.h>

#define INDEX getpid()%MAXPROC

/* ---------- Prototypes ---------- */
static int	ClockDriver(char *);
static int	DiskDriver(char *);

/* ---------- Globals ---------- */
p4proc      ProcTable[MAXPROC];
procQ       SleepQueue;
qPtr        head = &SleepQueue;
int         semRunning;
int         dFlag = 1;


void start3(void)
{
    char	name[128];
    char    buf[10];
    int		i;
    int		clockPID;
    int		pid;
    int		status;
    /*
     * Check kernel mode here.
     */
    int a = USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE;
    if (!a) {
        printf("start3(): Not in kernel mode. Halting...\n");
        USLOSS_Halt(1);
    }

    /*
     * Create clock device driver 
     * I am assuming a semaphore here for coordination.  A mailbox can
     * be used instead -- your choice.
     */
    semRunning = semcreateReal(0);
    clockPID = fork1("Clock driver", ClockDriver, NULL, USLOSS_MIN_STACK, 2);
    if (clockPID < 0) {
	USLOSS_Console("start3(): Can't create clock driver\n");
	USLOSS_Halt(1);
    }
    /*
     * Wait for the clock driver to start. The idea is that ClockDriver
     * will V the semaphore "semRunning" once it is running.
     */

    sempReal(semRunning);

    /*
     * Create the disk device drivers here.  You may need to increase
     * the stack size depending on the complexity of your
     * driver, and perhaps do something with the pid returned.
     */

    for (i = 0; i < USLOSS_DISK_UNITS; i++) {
        sprintf(buf, "%d", i);
        pid = fork1(name, DiskDriver, buf, USLOSS_MIN_STACK, 2);
        if (pid < 0) {
            USLOSS_Console("start3(): Can't create term driver %d\n", i);
            USLOSS_Halt(1);
        }
    }

    // May be other stuff to do here before going on to terminal drivers

    /*
     * Create terminal device drivers.
     */


    /*
     * Create first user-level process and wait for it to finish.
     * These are lower-case because they are not system calls;
     * system calls cannot be invoked from kernel mode.
     * I'm assuming kernel-mode versions of the system calls
     * with lower-case first letters, as shown in provided_prototypes.h
     */
    pid = spawnReal("start4", start4, NULL, 4 * USLOSS_MIN_STACK, 3);
    pid = waitReal(&status);

    /*
     * Zap the device drivers
     */
    zap(clockPID);  // clock driver

    // eventually, at the end:
    quit(0);   
}

/* ClockDriver -----------------------------------------------------------
   Purpose : ClockDriver's purpose is to essentially allow programs to 
                processes to have a delay
 Arguments : Takes a char as input
   Returns : an int.
 * ---------------------------------------------------------------------*/
static int ClockDriver(char *arg)
{
    int result;
    int status;

    // Let the parent know we are running and enable interrupts.
    semvReal(semRunning);
    USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);

    // Infinite loop until we are zap'd
    while(! isZapped()) {
	   result = waitDevice(USLOSS_CLOCK_DEV, 0, &status);
	   if (result != 0) {
	       return 0;
	   }
	/*
	 * Compute the current time and wake up any processes
	 * whose time has come.
	 */

    }
    return 0;
} /* ClockDriver */

/* Sleep -----------------------------------------------------------------
   Purpose : Sleep's purpose is to be the function that a program calls
                when it wants to be delayed for some reason.
 Arguments : Takes an int as the number of seconds that it wants to sleep
   Returns : void
 * ---------------------------------------------------------------------*/
int Sleep(int seconds)
{
    if (dFlag){
        printf("Sleep(): process %i calling sleep.\n", getpid());
        printf("\t we are in mode:%i\n", USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE);
    }

    // mark our status as being asleep in the proctable
    ProcTable[INDEX].status = ASLEEP;

    // Calculate the time from now + seconds
    int time;
    CPUTime(&time);
    int wakeUpTime = time + (seconds * 1000000);

    // add ourself to the Queue
    // if no one is in the Q yet
    if (head->process == 0){
        head->process = getpid();
        head->wakeUpTime = wakeUpTime;
    } // else if there are others in the q we must arrange them by wake-up-time
    else {
        if (wakeUpTime < head->wakeUpTime){
            procQ temp;
            temp.process = getpid();
            temp.wakeUpTime = wakeUpTime;
            temp.next = head;
        }
        else {
            qPtr iter = head;
            while (iter->next != NULL && wakeUpTime > iter->next->wakeUpTime){
                iter = iter->next;
            }
            procQ temp;
            temp.process = getpid();
            temp.wakeUpTime = wakeUpTime;
            temp.next = iter->next;
            iter->next = &temp;
        }
    }

    return 0;
} /* Sleep */

/* DiskDriver ------------------------------------------------------------
   Purpose : Not sure yet
 Arguments : Takes a char as input
   Returns : an int.
 * ---------------------------------------------------------------------*/
static int DiskDriver(char *arg)
{
    return 0;
} /* DiskDriver */
