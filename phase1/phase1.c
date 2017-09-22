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

void addToReadyList(procPtr);
void quitProcTableEntry(int);
void setupParent(procPtr);
void unBlockZappers(int);
void enableInterrupts();
void dispatcher(void);
void clockHandler();
void timeSlice();
void launch();

int blockMe(int newStatus);
int unblockProc(int pid);
int readCurStartTime();
int checkDeadlock();
int zap(int pid);
int isZapped();
int readTime();
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
    USLOSS_IntVec[USLOSS_ILLEGAL_INT] = clockHandler;

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
    USLOSS_Console("fork1(): called while in user mode, by process ");
    if(Current != NULL){
      USLOSS_Console("%i. Halting...\n", Current->pid);
    }
    USLOSS_Halt(1);
    return -1;
  }

    // Return if stack size is too small
    if (stacksize < USLOSS_MIN_STACK) {
      return -2;
      USLOSS_Halt(1);
    }

    // Return if priority is out of range
    if (priority > 6 || priority < 0) {
      return -1;
      USLOSS_Halt(1);
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
      return -1;
      USLOSS_Halt(1);
    } 
    else {
      while (ProcTable[nextPid % MAXPROC].status != S_EMPTY) {
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
    addToReadyList(&ProcTable[procSlot]);

    ProcTable[procSlot].status = S_READY;
    ProcTable[procSlot].procSlot = procSlot;
    // for future phase(s)
    p1_fork(ProcTable[procSlot].pid);

    if(Current != NULL)
      setupParent(&ProcTable[procSlot]);
    // Call dispatcher with the current process.
    if(priority != 6) {
      dispatcher();  
    }

    // More stuff to do here...

    return ProcTable[procSlot].pid;  // -1 is not correct! Here to prevent warning.
} /* fork1 */

void setupParent(procPtr child)
{
  if(DEBUG && debugflag){
    USLOSS_Console("setupParent(): setting up parent for %s\n", child->name);
  }
  if (Current != NULL){
    if (Current->childProcPtr != NULL){
      procPtr temp = Current->childProcPtr;
      while (temp->nextSiblingPtr != NULL) {
        temp = temp->nextSiblingPtr;
      }
      temp->nextSiblingPtr = child;
    } else {
      ProcTable[Current->procSlot].childProcPtr = child;
    }
    child->parentPtr = Current;
  }
}

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
        USLOSS_Console("launch(): starting %s\n", Current->name);

    enableInterrupts();

    // Call the function passed to fork1, and capture its return value
    result = Current->startFunc(Current->startArg);

    if (DEBUG && debugflag)
        USLOSS_Console("Process %d returned to launch\n", Current->pid);

    quit(result);

} /* launch */

int getpid()
{
  return Current->pid;
}


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
  if (DEBUG && debugflag){
    USLOSS_Console("join(): Starting join on process %s\n", Current->name);
  }

  if (Current->childProcPtr == NULL) { // If it has no children.
    //USLOSS_Console("join(): Current process has no children. Halting...\n");
    return -2;
    USLOSS_Halt(1);
  } 

  // If it has at least one child.
  procPtr temp = Current->childProcPtr;
  procPtr beforeTemp = NULL;
  while (temp->status >= 0 && temp->nextSiblingPtr != NULL){
    if(temp->pid != Current->childProcPtr->pid){
      beforeTemp = temp;      
    }
    temp = temp->nextSiblingPtr;      
  }
  // If this condition is true then no children have quit.
  if (temp->nextSiblingPtr == NULL && temp->status >= 0){
    Current->status = S_JOIN_BLOCKED;
    dispatcher();
  }
  int returnVal;

  temp = Current->childProcPtr;
  beforeTemp = NULL;
  while (temp->status >= 0 && temp->nextSiblingPtr != NULL){
    beforeTemp = temp;
    temp = temp->nextSiblingPtr;      
  }

  temp->status = 0;

  if (beforeTemp == NULL){
    Current->childProcPtr = temp->nextSiblingPtr;
  } else {
    beforeTemp->nextSiblingPtr = temp->nextSiblingPtr;
  }

  *status = temp->quitStatus;
  Current->status = S_RUNNING;

  returnVal = temp->pid;
  if(isZapped()){
    returnVal = -1;
  }
  quitProcTableEntry(temp->procSlot);
  return returnVal;
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
  if (DEBUG && debugflag){
    USLOSS_Console("quit(): quitting %s\n", Current->name);
  }

  // check to see if it has any unquit child processes
  if (Current->childProcPtr != NULL && Current->childProcPtr->status > 0){
    USLOSS_Console("quit(): process %i, '%s', has active children. Halting...\n", Current->pid, Current->name);
    USLOSS_Halt(1);
  }

  int modeResult;
  modeResult = mode();
  if (modeResult != 1){
    USLOSS_Console("quit(): called while in user mode, by process %i. Halting...\n", Current->pid);
    USLOSS_Halt(1);
  }

  if (Current->status >= 0){
    processTableCounter--;
    if (Current->parentPtr == NULL){
      Current->status = 0;
      quitProcTableEntry(Current->procSlot);
    } else {
      unBlockZappers(Current->procSlot);
      Current->status = -1;
    }
    Current->quitStatus = status;
    p1_quit(Current->pid);
    dispatcher();
  }
} /* quit */

void quitProcTableEntry(int procSlot)
{
  // Check to see if it was zapped. If it was, unblock all zap'ers.
  unBlockZappers(procSlot);

  ProcTable[procSlot].priority = 0;
  free(ProcTable[procSlot].stack);
  ProcTable[procSlot].stackSize = 0;
  ProcTable[procSlot].procSlot = -1;
  ProcTable[procSlot].nextProcPtr = NULL;
  ProcTable[procSlot].childProcPtr = NULL;
  ProcTable[procSlot].nextSiblingPtr = NULL;
  ProcTable[procSlot].parentPtr = NULL;
  ProcTable[procSlot].zapPtr = NULL;
}

void unBlockZappers(int procSlot)
{
  if (DEBUG && debugflag){
    USLOSS_Console("unBlockZappers():\n");
  }
  if (ProcTable[procSlot].zapPtr != NULL){
    procPtr temp, next;
    temp = ProcTable[procSlot].zapPtr;
    addToReadyList(temp);
    while (temp->nextZapPtr != NULL){
      next = temp->nextZapPtr;
      temp->nextZapPtr = NULL;
      temp = next;
      addToReadyList(next);
    }
  }
}

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
  int a;
  if (DEBUG && debugflag) {
    USLOSS_Console("dispatcher(): Dispatching process\n");
  }

  // Check if current has a parent that is join blocked
  if (Current != NULL && Current->parentPtr != NULL && Current->parentPtr->status == S_JOIN_BLOCKED && Current->status < 0){
    procPtr old = Current;
    Current = Current->parentPtr;
    a = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, USLOSS_CLOCK_INT, &Current->startTime);
    USLOSS_ContextSwitch(&old->state, &Current->state);
  }

  procPtr nextProcess = NULL;
  for (int i = 0; i < READY_LISTS; i++) {
    if (ReadyLists[i].size > 0) {
      if (Current == NULL || i+1 < Current->priority || Current->status != S_RUNNING){
        nextProcess = ReadyLists[i].head;
        ReadyLists[i].head = nextProcess->nextProcPtr;
        ReadyLists[i].size--;
        if (ReadyLists[i].size == 0){
          ReadyLists[i].head = NULL;
        }
        if (nextProcess != NULL && nextProcess->status == S_READY){
          break;
        }
      } 
    }
  }

  if (nextProcess != NULL){
    nextProcess->status = S_RUNNING;
    if (Current == NULL){
      p1_switch(-1, nextProcess->pid);      
    } else {
      p1_switch(Current->pid, nextProcess->pid);
    }  

    enableInterrupts();

    if (Current == NULL){
      Current = nextProcess;
      a = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, USLOSS_CLOCK_INT, &nextProcess->startTime);
      USLOSS_ContextSwitch(NULL, &nextProcess->state);
    } 
    else {
      if (Current->status == 2) {
        addToReadyList(Current);
      }
      procPtr old = Current;
      Current = nextProcess;
      a = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, USLOSS_CLOCK_INT, &nextProcess->startTime);
      USLOSS_ContextSwitch(&old->state, &nextProcess->state);
    }
  }
} /* dispatcher */

/* ------------------------------------------------------------------------
   Name - clockHandler
   Purpose - Will be some sort of clock handler but in the meantime does
              nothing.
   Parameters - none
   Returns - nothing
   ----------------------------------------------------------------------- */    
void clockHandler()
{
  if (Current != NULL){
    timeSlice();
  }
}

void timeSlice()
{
  int currentTime, a;
  a = USLOSS_DeviceInput(USLOSS_CLOCK_DEV, USLOSS_CLOCK_INT, &currentTime);
  if (Current->startTime - currentTime >= 80000){
    dispatcher();
  }
}

/* ------------------------------------------------------------------------
   Name - addToReadyList
   Purpose - Adds a procPtr to the ready list.
   Parameters - procPtr
   Returns - nothing
   ----------------------------------------------------------------------- */
void addToReadyList(procPtr toBeAdded)
{
  if (DEBUG && debugflag) {
    USLOSS_Console("addToReadyList(): Adding process %s with pid:%i and priority:%i\n", toBeAdded->name, toBeAdded->pid, toBeAdded->priority);
  }
  toBeAdded->status = S_READY;
  int adjustedPriority = toBeAdded->priority-1;
  if (ReadyLists[adjustedPriority].size == 0) {
      ReadyLists[adjustedPriority].head = toBeAdded;
      ReadyLists[adjustedPriority].size++;
  }
  else {
    procPtr temp = ReadyLists[adjustedPriority].head;
    int i = 0;
    while(temp->nextProcPtr != NULL && temp->nextProcPtr->status == S_READY && i < ReadyLists[adjustedPriority].size){
      temp = temp->nextProcPtr;
      i++;
    }
    temp->nextProcPtr = toBeAdded;
    ReadyLists[adjustedPriority].size++;
  }
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
    if (DEBUG && debugflag){
        USLOSS_Console("sentinel(): called\n");
    }
    while (1)
    {
        checkDeadlock();
        USLOSS_WaitInt();
    }
} /* sentinel */

void dumpProcesses()
{
  USLOSS_Console("%s\t%s\t%s\t%s\t\t%s\t%s\t%s\n", "PID", "Parent", "Priority", "Status", "# Kids", "CPUtime", "Name");
  for (int i = 0; i < MAXPROC; i++){
    if (ProcTable[i].status != 0){
      USLOSS_Console(" %i\t", ProcTable[i].pid);
      if(ProcTable[i].parentPtr != NULL){
        USLOSS_Console("  %i\t", ProcTable[i].parentPtr->pid);
      } else {
        USLOSS_Console("  -2\t");
      }
      USLOSS_Console("   %i\t\t", ProcTable[i].priority);
      if(ProcTable[i].status == 1){
        USLOSS_Console("READY\t");
      } else if (ProcTable[i].status == 2){
        USLOSS_Console("RUNNING\t");
      } else if (ProcTable[i].status == 3){
        USLOSS_Console("JOIN_BLOCKED");
      } else if (ProcTable[i].status == 4){
        USLOSS_Console("ZAPPING\t");
      } else if (ProcTable[i].status >= 10){
        USLOSS_Console("BLOCKME\t");
      } else {
        USLOSS_Console("%i\t", ProcTable[i].status);
      }
      USLOSS_Console("\t  0\t");
      USLOSS_Console("   %d\t", ProcTable[i].startTime);
      USLOSS_Console("%s\n", ProcTable[i].name);
    }
    else {
      USLOSS_Console(" %i\t", -1);
      USLOSS_Console("  %i\t", -1);
      USLOSS_Console("   %i\t", -1);
      USLOSS_Console("\tEMPTY\t");
      USLOSS_Console("\t  0\t");
      USLOSS_Console("   -1\t");
      USLOSS_Console("\n");
    }
  }
}

int blockMe(int newStatus)
{
  if (Current == NULL){
    USLOSS_Console("FATAL ERROR: CURRENT == NULL at blockMe()\n");
    USLOSS_Halt(1);
  }
  if (DEBUG && debugflag){
    USLOSS_Console("blockMe(): blocking process %s with pid %i\n", Current->name, Current->pid);
  }

  if (newStatus <= 10){
    USLOSS_Console("blockMe(): newStatus must be > 10. Halting...\n");
    USLOSS_Halt(1);
  }

  Current->status = newStatus;
  dispatcher();
  if(isZapped()){
    return -1;
  }

  return 0;
}

int unblockProc(int pid)
{
  if (DEBUG && debugflag){
    USLOSS_Console("unBlockProc(): unblocking process %i\n", pid);
  }

  if (ProcTable[pid].status <= 10){
    return -2;
  } else {
    addToReadyList(&ProcTable[pid]);
  }

  dispatcher();

  if (isZapped()){
    return -1;
  } 
  return 0;
}

/* check to determine if deadlock has occurred... */
int checkDeadlock()
{
  if(processTableCounter != 1){
    for (int i = 0; i < MAXPROC; i++){
      procPtr temp;
      temp = &ProcTable[i];
      if(temp->priority != 6 && temp->priority > 2){
        USLOSS_Console("checkDeadlock(): numProc = %i. Only Sentinel should be left. Halting...\n", processTableCounter);
        USLOSS_Halt(1);
        return 1;
      }
    }
  }
  
  USLOSS_Console("All processes completed.\n");
  USLOSS_Halt(0);
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

/*
* This operation marks a process pid as being zapped. Subsequent calls to
* isZapped by that process will return 1
* Does not return until the zapped proccess has quit
*/
int zap(int pid)
{
	if(DEBUG && debugflag)
		USLOSS_Console("zap(): Zapping process %i\n", pid);
	if(Current != NULL)
	{
		if(Current->pid == pid)
		{
			USLOSS_Console("zap(): process %i tried to zap itself.  Halting...\n", pid);
			USLOSS_Halt(1);
			return -1;
		}
    if(ProcTable[pid].status == 0){
      USLOSS_Console("zap(): process being zapped does not exist.  Halting...\n");
      USLOSS_Halt(1);
      return -1;
    }
    if(ProcTable[pid].status < 0){
      if(isZapped()){
        return -1;
      }
      return 0;
    }
    for (int i = 0; i < MAXPROC; i++){
      if (ProcTable[i].pid == pid){
        ProcTable[i].zapped = 1;
        Current->status = S_ZAPPING;
        if (ProcTable[i].zapPtr == NULL){
          ProcTable[i].zapPtr = Current;
        } else {
          procPtr temp;
          temp = ProcTable[i].zapPtr;
          while (temp->nextZapPtr != NULL){
            temp = temp->nextZapPtr;
          }
          temp->nextZapPtr = Current;
        }
        dispatcher();
        if(isZapped()){
          return -1;
        }
      } 
    }
	}
	return 0;
}

/*
* Simply checks if the currently running proccess has been zapped
*/
int isZapped()
{
  if (DEBUG && debugflag){
    USLOSS_Console("isZapped(): starting");
  }
  if (Current != NULL){
    return Current->zapped;
  }
  return -1;
}
