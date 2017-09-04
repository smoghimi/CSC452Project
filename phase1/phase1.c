/* ------------------------------------------------------------------------
   phase1.c

   University of Arizona
   Computer Science 452
   Fall 2015

   ------------------------------------------------------------------------ */

#include "phase1.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <usloss.h>
#include "kernel.h"

/* ------------------------- Prototypes ----------------------------------- */
int sentinel (char *);
extern int start1 (char *);
void dispatcher(void);
void launch();
void clockHandler();
void enableInterrupts();
int checkDeadlock();
int mode();


/* -------------------------- Globals ------------------------------------- */

// Patrick's debugging global variable...
int debugflag = 1;

// the process table
procStruct ProcTable[MAXPROC];

// How many processes are currently in the process table
int processTableCounter = 0;

// Index of process table to set next process in. -1 if process table is full
int processIndex = 0;

// Process lists
static readyList ReadyLists[READY_LISTS];

// current process ID
procPtr Current;

// the next pid to be assigned
unsigned int nextPid = SENTINELPID;


/* -------------------------- Functions ----------------------------------- */
/* ------------------------------------------------------------------------
   Name - startup
   Purpose - Initializes process lists and clock interrupt vector.
             Start up sentinel process and the test process.
   Parameters - argc and argv passed in by USLOSS
   Returns - nothing
   Side Effects - lots, starts the whole thing
   ----------------------------------------------------------------------- */
void startup(int argc, char *argv[])
{
    int result; /* value returned by call to fork1() */
    Current = NULL;
    /* initialize the process table */
    if (DEBUG && debugflag)
        USLOSS_Console("startup(): initializing process table, ProcTable[]\n");
    // for (int i = 0; i < MAXPROC; i++)
    // {
    //   procStruct emptyProcess;
    //   ProcTable[i] = emptyProcess;
    // }

    // Initialize the Ready list, etc.
    if (DEBUG && debugflag)
        USLOSS_Console("startup(): initializing the Ready list\n");

    // Initialize the clock interrupt handler
    USLOSS_IntVec[USLOSS_CLOCK_INT] = clockHandler;

    // startup a sentinel process
    if (DEBUG && debugflag)
        USLOSS_Console("startup(): calling fork1() for sentinel\n");
    result = fork1("sentinel", sentinel, NULL, USLOSS_MIN_STACK,
                    SENTINELPRIORITY);
    if (result < 0) {
        if (DEBUG && debugflag) {
            USLOSS_Console("startup(): fork1 of sentinel returned error, ");
            USLOSS_Console("halting...\n");
        }
        USLOSS_Halt(1);
    }
  
    // start the test process
    if (DEBUG && debugflag)
        USLOSS_Console("startup(): calling fork1() for start1\n");
    result = fork1("start1", start1, NULL, 2 * USLOSS_MIN_STACK, 1);
    if (result < 0) {
        USLOSS_Console("startup(): fork1 for start1 returned an error, ");
        USLOSS_Console("halting...\n");
        USLOSS_Halt(1);
    }

    USLOSS_Console("startup(): Should not see this message! ");
    USLOSS_Console("Returned from fork1 call that created start1\n");

    return;
} /* startup */

/* ------------------------------------------------------------------------
   Name - finish
   Purpose - Required by USLOSS
   Parameters - none
   Returns - nothing
   Side Effects - none
   ----------------------------------------------------------------------- */
void finish(int argc, char *argv[])
{
    if (DEBUG && debugflag)
        USLOSS_Console("in finish...\n");
} /* finish */

/* ------------------------------------------------------------------------
   Name - fork1
   Purpose - Gets a new process from the process table and initializes
             information of the process.  Updates information in the
             parent process to reflect this child process creation.
   Parameters - the process procedure address, the size of the stack and
                the priority to be assigned to the child process.
   Returns - the process id of the created child or -1 if no child could
             be created or if priority is not between max and min priority.
   Side Effects - ReadyList is changed, ProcTable is changed, Current
                  process information changed
   ------------------------------------------------------------------------ */
int fork1(char *name, int (*startFunc)(char *), char *arg,
          int stacksize, int priority)
{
    int procSlot = -1;

    if (DEBUG && debugflag)
      USLOSS_Console("fork1(): creating process %s\n", name);

    // test if in kernel mode; halt if in user mode
    int modeResult;
    modeResult = mode();
    if (modeResult != 1){
      USLOSS_Console("fork1(): Must be in kernel mode to access fork1. Halting...\n");
      USLOSS_Halt(1);
      return -1;
    }

    // Return if stack size is too small
    if (stacksize < USLOSS_MIN_STACK) {
      USLOSS_Console("fork1(): Process stack size is too small. Halting...\n");
      USLOSS_Halt(1);
      return -2;
    }

    // Return if priority is out of range
    if (priority > 6 || priority < 0) {
      USLOSS_Console("fork1(): Process priority out of range. Halting...\n");
      USLOSS_Halt(1);
      return -1;
    }

    // Return if Startfunc or name is null
    if (startFunc == NULL) {
      USLOSS_Console("fork1(): Process startFunc is null. Halting...\n");
      USLOSS_Halt(1);
      return -1;
    }
    if (name == NULL) {
      USLOSS_Console("fork1(): Process name is null. Halting...\n");
      USLOSS_Halt(1);
      return -1;
    }

    // Is there room in the process table? What is the next PID?
    if (processTableCounter >= MAXPROC) {
      USLOSS_Console("fork1(): Process table is full. Halting...\n");
      USLOSS_Halt(1);
      return -1;
    } 
    else {
      while (ProcTable[nextPid % MAXPROC].priority != 0) {
        nextPid++;
      }
      procSlot = nextPid%MAXPROC;
      processTableCounter++;
    }

    // fill-in entry in process table */
    if ( strlen(name) >= (MAXNAME - 1) ) {
        USLOSS_Console("fork1(): Process name is too long.  Halting...\n");
        USLOSS_Halt(1);
        return -1;
    }
    strcpy(ProcTable[procSlot].name, name);
    ProcTable[procSlot].startFunc = startFunc;
    if ( arg == NULL )
        ProcTable[procSlot].startArg[0] = '\0';
    else if ( strlen(arg) >= (MAXARG - 1) ) {
        USLOSS_Console("fork1(): argument too long.  Halting...\n");
        USLOSS_Halt(1);
    }
    else
        strcpy(ProcTable[procSlot].startArg, arg);
    ProcTable[procSlot].pid = nextPid++;
    ProcTable[procSlot].priority = priority;
    ProcTable[procSlot].stackSize = stacksize;
    ProcTable[procSlot].stack = (char *)malloc(stacksize);

    // Initialize context for this process, but use launch function pointer for
    // the initial value of the process's program counter (PC)
    USLOSS_ContextInit(&(ProcTable[procSlot].state),
                       ProcTable[procSlot].stack,
                       ProcTable[procSlot].stackSize,
                       NULL,
                       launch);

    // Add the process to the correct ready list
    // since ReadyLists goes from 0-5 we need to subtract 1 from the priority
    int adjustedPriority;
    adjustedPriority = priority-1;
    if (ReadyLists[adjustedPriority].size == 0) {
      ReadyLists[adjustedPriority].head = &ProcTable[procSlot];
      ReadyLists[adjustedPriority].size++;
    }
    else {
      procPtr temp = ReadyLists[adjustedPriority].head;
      for (int i = 0; i < ReadyLists[adjustedPriority].size; i++){
        temp = temp->nextProcPtr;
      }
      temp->nextProcPtr = &ProcTable[procSlot];
      ReadyLists[adjustedPriority].size++;
    }

    ProcTable[procSlot].status = READY;
    // for future phase(s)
    p1_fork(ProcTable[procSlot].pid);

    // Call dispatcher with the current process.
    if(priority != 6) {
      dispatcher();  
    }

    // More stuff to do here...

    return nextPid;  // -1 is not correct! Here to prevent warning.
} /* fork1 */

/* ------------------------------------------------------------------------
   Name - mode
   Purpose - Checks if the mode is in kernel or user
   Parameters - none
   Returns - 1 if kernel, 0 if user
   Side Effects - none
   ------------------------------------------------------------------------ */
int mode() 
{
  //if (DEBUG && debugflag)
  //  USLOSS_Console("mode(): Determining processor mode.\n");
  unsigned int mode;
  mode = USLOSS_PsrGet();
  if ((mode & USLOSS_PSR_CURRENT_MODE) == 0)
    return 0;
  else
    return 1;
}

/* ------------------------------------------------------------------------
   Name - launch
   Purpose - Dummy function to enable interrupts and launch a given process
             upon startup.
   Parameters - none
   Returns - nothing
   Side Effects - enable interrupts
   ------------------------------------------------------------------------ */
void launch()
{
    int result;

    if (DEBUG && debugflag)
        USLOSS_Console("launch(): started\n");

    enableInterrupts();

    // Call the function passed to fork1, and capture its return value
    result = Current->startFunc(Current->startArg);

    if (DEBUG && debugflag)
        USLOSS_Console("Process %d returned to launch\n", Current->pid);

    quit(result);

} /* launch */


/* ------------------------------------------------------------------------
   Name - join
   Purpose - Wait for a child process (if one has been forked) to quit.  If 
             one has already quit, don't wait.
   Parameters - a pointer to an int where the termination code of the 
                quitting process is to be stored.
   Returns - the process id of the quitting child joined on.
             -1 if the process was zapped in the join
             -2 if the process has no children
   Side Effects - If no child process has quit before join is called, the 
                  parent is removed from the ready list and blocked.
   ------------------------------------------------------------------------ */
int join(int *status)
{

    return -1;  // -1 is not correct! Here to prevent warning.
} /* join */


/* ------------------------------------------------------------------------
   Name - quit
   Purpose - Stops the child process and notifies the parent of the death by
             putting child quit info on the parents child completion code
             list.
   Parameters - the code to return to the grieving parent
   Returns - nothing
   Side Effects - changes the parent of pid child completion status list.
   ------------------------------------------------------------------------ */
void quit(int status)
{
    p1_quit(Current->pid);
} /* quit */


/* ------------------------------------------------------------------------
   Name - dispatcher
   Purpose - dispatches ready processes.  The process with the highest
             priority (the first on the ready list) is scheduled to
             run.  The old process is swapped out and the new process
             swapped in.
   Parameters - none
   Returns - nothing
   Side Effects - the context of the machine is changed
   ----------------------------------------------------------------------- */
void dispatcher(void)
{
    if (DEBUG && debugflag) {
      USLOSS_Console("dispatcher(): Dispatching process\n");
    }

    procPtr nextProcess = NULL;
    for (int i = 0; i < READY_LISTS; i++) {
      if (ReadyLists[i].size > 0) {
        nextProcess = ReadyLists[i].head;
        ReadyLists[i].head = nextProcess->nextProcPtr;
        ReadyLists[i].size--;
        break;
      }
    }

    if (Current == NULL){
      p1_switch(-1, nextProcess->pid);      
    }

    enableInterrupts();

    if (Current == NULL){
      Current = nextProcess;
      USLOSS_ContextSwitch(NULL, &nextProcess->state);
    } else {
      USLOSS_ContextSwitch(&Current->state, &nextProcess->state);
      Current = nextProcess;    }

} /* dispatcher */

/* ------------------------------------------------------------------------
   Name - clockHandler
   Purpose - Will be some sort of clock handler but in the meantime does
              nothing.
   Parameters - none
   Returns - nothing
   ----------------------------------------------------------------------- */    
void clockHandler(){
  // does nothing!
}

/* ------------------------------------------------------------------------
   Name - sentinel
   Purpose - The purpose of the sentinel routine is two-fold.  One
             responsibility is to keep the system going when all other
             processes are blocked.  The other is to detect and report
             simple deadlock states.
   Parameters - none
   Returns - nothing
   Side Effects -  if system is in deadlock, print appropriate error
                   and halt.
   ----------------------------------------------------------------------- */

int sentinel (char *dummy)
{
    if (DEBUG && debugflag)
        USLOSS_Console("sentinel(): called\n");
    while (1)
    {
        checkDeadlock();
        USLOSS_WaitInt();
    }
} /* sentinel */

/* check to determine if deadlock has occurred... */
int checkDeadlock()
{
  for (int i = 0; i < MAXPROC; i++) {
    if(ProcTable[i].status == READY) {
      return 1;
    }
  }
  return 0;
} /* checkDeadlock */

void enableInterrupts()
{
  if (mode()){
    // Enable interrupts
    unsigned int mode;
    mode = USLOSS_PsrGet();
    mode = mode | (USLOSS_PSR_CURRENT_INT);
    mode = USLOSS_PsrSet(mode);
  }
}

/*
 * Disables the interrupts.
 */
void disableInterrupts()
{
    // turn the interrupts OFF iff we are in kernel mode
    // if not in kernel mode, print an error message and
    // halt USLOSS

} /* disableInterrupts */
