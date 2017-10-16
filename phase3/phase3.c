#include <usloss.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>

#define INDEX getpid()%MAXPROC

/* ------------------------- Function Prototypes ------------------------- */
/* ---------- Externs ---------- */
extern int start3(char*);

/* ---------- Void Prototypes ---------- */
void terminate(systemArgs * args);
void semcreate(systemArgs * args);
void getpid2(systemArgs * args);
void semFree(systemArgs * args);
void spawn(systemArgs * args);
void wait2(systemArgs * args);
void semFreeReal(int handle);
void semP(systemArgs * args);
void semV(systemArgs * args);
void semPReal(int handle);
void semVReal(int handle);
void setToKernelMode();
void terminateReal();
void setToUserMode();

/* ---------- Int Prototypes ---------- */
int spawnReal(char* name, int(*func)(char *), char*arg, int stacksize, int priority);
int waitReal(int*);
int spawnLaunch();

/* ---------- Other Prototypes ---------- */
int semcreateReal(int value);

/* ------------------------- Globals ------------------------- */
p3proc ProcTable[MAXPROC];
int SemTable[MAXSEMS];
int MboxTable[MAXSEMS];
int BlockedSems[MAXSEMS];
int debugFlag = 0;
int currentPID = 4;
int currentSems = 0;
int semCount = 0;

int start2(char *arg)
{
    int pid;
    int status;

    for (int i = 0; i < MAXSEMS; i++){
        SemTable[i] = -1;
    }
    /*
     * Check kernel mode here.
     */

    /*
     * Data structure initialization as needed...
     */
    systemCallVec[SYS_SPAWN]        = spawn;
    systemCallVec[SYS_WAIT]         = wait2;
    systemCallVec[SYS_TERMINATE]    = terminate;
    systemCallVec[SYS_SEMCREATE]    = semcreate;
    systemCallVec[SYS_SEMP]         = semP;
    systemCallVec[SYS_SEMV]         = semV;
    systemCallVec[SYS_GETPID]       = getpid2;
    systemCallVec[SYS_SEMFREE]      = semFree;

    /*
     * Create first user-level process and wait for it to finish.
     * These are lower-case because they are not system calls;
     * system calls cannot be invoked from kernel mode.
     * Assumes kernel-mode versions of the system calls
     * with lower-case names.  I.e., Spawn is the user-mode function
     * called by the test cases; spawn is the kernel-mode function that
     * is called by the syscallHandler; spawnReal is the function that
     * contains the implementation and is called by spawn.
     *
     * Spawn() is in libuser.c.  It invokes USLOSS_Syscall()
     * The system call handler calls a function named spawn() -- note lower
     * case -- that extracts the arguments from the sysargs pointer, and
     * checks them for possible errors.  This function then calls spawnReal().
     *
     * Here, we only call spawnReal(), since we are already in kernel mode.
     *
     * spawnReal() will create the process by using a call to fork1 to
     * create a process executing the code in spawnLaunch().  spawnReal()
     * and spawnLaunch() then coordinate the completion of the phase 3
     * process table entries needed for the new process.  spawnReal() will
     * return to the original caller of Spawn, while spawnLaunch() will
     * begin executing the function passed to Spawn. spawnLaunch() will
     * need to switch to user-mode before allowing user code to execute.
     * spawnReal() will return to spawn(), which will put the return
     * values back into the sysargs pointer, switch to user-mode, and 
     * return to the user code that called Spawn.
     */
    pid = spawnReal("start3", start3, NULL, USLOSS_MIN_STACK, 3);
    ProcTable[pid].startFunc = start3;
    ProcTable[pid].arg = NULL;

    /* Call the waitReal version of your wait code here.
     * You call waitReal (rather than Wait) because start2 is running
     * in kernel (not user) mode.
     */
    pid = waitReal(&status);

    return 0;
} /* start2 */

/* spawn------------------------------------------------------------------
   Name - spawn
   Purpose - to spawn a new process!
   Parameters - system args which contain everything we need to call fork.
   Returns - void
   -------------------------------------------------------------------- */
void spawn(systemArgs * args)
{
    if (debugFlag){
        printf("spawn():\n");
    }
    if (args->arg1 == NULL) {
        USLOSS_Console("Null function pointer. Halting...(Shaion you will need to change this)\n");
        USLOSS_Halt(1);
    }
    if ((int)args->arg3 < USLOSS_MIN_STACK) {
        USLOSS_Console("Stacksize is too small. Halting...(Shaion you will need to change this)\n");
        USLOSS_Halt(1);
    }
    if (args->arg4 < 0 || (int)args->arg4 >= 6) {
        USLOSS_Console("Priority is out of bounds. Halting...(Shaion you will need to change this)\n");
        USLOSS_Halt(1);
    }
    int pid = spawnReal(args->arg5, args->arg1, args->arg2, (int)args->arg3, (int)args->arg4);

    if (ProcTable[INDEX].children == 1) {
        ProcTable[INDEX].child = &ProcTable[pid%MAXPROC];
    }
    else {
        procPtr temp = ProcTable[INDEX].child;
        while(temp->nextSibling != NULL){
            temp = temp->nextSibling;
        }
    }
    args->arg1 = (void *) pid;
    setToUserMode();
} /* spawn */

/* spawnReal--------------------------------------------------------------
   Name - spawnReal
   Purpose - Wait for a child process (if one has been forked) to quit.  If 
             one has already quit, don't wait.
   Parameters - name: the name of the process
                startfunc: the function that spawnLaunch will call
                arg: the argument for the process
                stacksize: the size of the stack
                priority: the priority of the process
   Returns - the process id that the the process being created returns.
                In other words this returns what fork1 returns.
   -------------------------------------------------------------------- */
int spawnReal(char * name, int(*startFunc)(char *), char * arg, int stacksize, int priority)
{
    if (debugFlag){
        printf("spawnReal():\n");
        printf("%s - %s - %i - %i\n", name, arg, stacksize, priority);
    }
    ProcTable[INDEX].children++;
    ProcTable[currentPID%MAXPROC].startFunc = startFunc;
    ProcTable[currentPID%MAXPROC].arg = arg;
    ProcTable[currentPID%MAXPROC].pid = currentPID;
    currentPID++;
    int a = fork1(name, spawnLaunch, arg, stacksize, priority);
    return a;
} /* spawnReal */

/* spawnLaunch------------------------------------------------------------
   Name - spawnLaunch
   Purpose - launches the function stored in the current processes PCB
   Parameters - none
   Returns - Whatever the function that we are launching returned.
   -------------------------------------------------------------------- */
int spawnLaunch()
{
    unsigned int a = USLOSS_PsrGet();
    a ^= USLOSS_PSR_CURRENT_MODE;
    a = USLOSS_PsrSet(a);
    int index = getpid() % MAXPROC;
    int returnValue;
    returnValue = ProcTable[index].startFunc(ProcTable[index].arg);
    setToKernelMode();
    quit(0);
    return returnValue;
} /* spawnLaunch */

/* wait-------------------------------------------------------------------
   Name - wait
   Purpose - calls waitReal 
   Parameters - sysArgs that we will fill with the return result of 
                waitReal
   Returns - none
   -------------------------------------------------------------------- */
void wait2(systemArgs * args)
{
    int status;
    args->arg1 = (void *) waitReal(&status);
    if(args->arg1 > 0){
        args->arg4 = 0;
    } 
    else {
        args->arg4 = (void *) -1;
    }
    args->arg2 = (void *) status;
    setToUserMode();
} /* wait */

/* waitReal---------------------------------------------------------------
   Name - waitReal
   Purpose - calls join
   Parameters - a status int that will be filled with the result of our
                join call
   Returns - whatever join returns
   ------------------------------------------------------------------------ */
int waitReal(int* status)
{
    int result;
    result = join(status);
    return result;
} /* waitReal */

/* terminate--------------------------------------------------------------
   Name - terminate
   Purpose - calls terminateReal and quits after it returns 
   Parameters - sysArgs that contain the quit status for us to use
   Returns - none
   -------------------------------------------------------------------- */
void terminate(systemArgs * args)
{
    terminateReal();
    setToUserMode();
    quit((int)args->arg1);
} /* terminate */

/* terminateReal----------------------------------------------------------
   Name - terminateReal
   Purpose - zaps all of the children of current process if it has any. 
                If it did have children then it keeps calling waitReal
                until it returns <= 0.
   Parameters - none
   Returns - void 
   -------------------------------------------------------------------- */
void terminateReal()
{
    int status;
    int result;
    procPtr temp = ProcTable[getpid() % MAXPROC].child;
    while (temp != NULL && temp->nextSibling != NULL){
        zap(temp->pid);
    }
    result = waitReal(&status);
    while(result > 0){
        result = waitReal(&status);
    }
} /* terminateReal */

/* semcreate--------------------------------------------------------------
   Name - semcreate
   Purpose - to check the input from our syscall and then call screateReal
   Parameters - systemArgs
   Returns - void 
   -------------------------------------------------------------------- */
void semcreate(systemArgs * args)
{
    if (debugFlag){
        printf("semcreate():\n");
    } 
    if (args->arg1 < 0){
        args->arg4 = -1;
        setToUserMode();
        return;
    }
    if (semCount >= MAXSEMS){
        args->arg4 = (void *) -1;
        setToUserMode();
        return;
    }

    args->arg1 = (void *)semcreateReal((int)args->arg1);
    if ((int)args->arg1 == -3){
        args->arg4 = (void *) -1;
    } else {
        semCount++;
        args->arg4 = 0;
    }
    setToUserMode();
} /* semcreate */

/* semcreateReal----------------------------------------------------------
   Name - semcreateReal
   Purpose - takes in an int value and if there is a handle available it
                will assign that value to that handle.
   Parameters - value
   Returns - an int handle 
   -------------------------------------------------------------------- */
int semcreateReal(int value)
{
    int counter = MAXSEMS+1;
    while (SemTable[currentSems%MAXSEMS] != -1 && counter--){
        currentSems++;
    }
    if(counter == 0){
        return -3;
    }
    SemTable[currentSems%MAXSEMS] = value;
    MboxTable[currentSems%MAXSEMS] = MboxCreate(0, 0);
    return currentSems;
} /* semcreateReal */

/* semP-------------------------------------------------------------------
   Name - semP
   Purpose - to check if the given handle is valid and if it is call
                semPReal
   Parameters - systemArgs
   Returns - void 
   -------------------------------------------------------------------- */
void semP(systemArgs * args)
{
    if (debugFlag){
        printf("semP():\n");
    }

    int handle = (int)args->arg1;
    if (SemTable[handle] == -1){
        args->arg4 = (void *) -1;
    }

    semPReal(handle);
    args->arg4 = 0;
    setToUserMode();
} /* semP */

/* semPReal---------------------------------------------------------------
   Name - semPReal
   Purpose - if the semaphore is positive then it decrements it. Else it 
                will block until it is positive and then decrement it.
   Parameters - semaphore handle
   Returns - void 
   -------------------------------------------------------------------- */
void semPReal(int handle)
{
    if (SemTable[handle] > 0){
        SemTable[handle]--;
    }
    else {
        // Block ourselves here
        while(SemTable[handle] <= 0){
            BlockedSems[handle]++;
            MboxReceive(MboxTable[handle], NULL, 0);
            BlockedSems[handle]--;
        }
        SemTable[handle]--;
    }
} /* semPReal */

/* semV-------------------------------------------------------------------
   Name - semV
   Purpose - checks the input args to see if the handle exists and then
                calls semVReal
   Parameters - systemArgs 
   Returns - void 
   -------------------------------------------------------------------- */
void semV(systemArgs * args)
{
    if (debugFlag){
        printf("semV():\n");
    }

    int handle = (int)args->arg1;
    if (SemTable[handle] == -1){
        args->arg4 = (void *) -1;
    }

    semVReal(handle);
    args->arg4 = 0;
    setToUserMode();
} /* semV */

/* semVReal---------------------------------------------------------------
   Name - semVReal
   Purpose - Increments the semaphore and then conditionally sends to
                the semaphores mailbox to unblock a blocked process.
   Parameters - semaphore handle
   Returns - void 
   -------------------------------------------------------------------- */
void semVReal(int handle)
{
    SemTable[handle]++;
    MboxCondSend(MboxTable[handle], NULL, 0);
} /* semVReal */

/* semFree----------------------------------------------------------------
   Name - semFree
   Purpose - check if the handle is valid and then call semFreeReal with
                the valid handle
   Parameters - systemArgs
   Returns - void 
   -------------------------------------------------------------------- */
void semFree(systemArgs * args)
{
    int handle = (int) args->arg1;
    if (SemTable[handle] < 0){
        args->arg4 = (void *) -1;
        return;
    }
    if (BlockedSems[handle] == 0){
        semCount--;
        args->arg4 = (void *) 0;
        semFreeReal(handle);
    }
    else {
        //deal with the case where we have blocked process on our sem.
    }
    setToUserMode();
} /* semFree */

/* semFreeReal------------------------------------------------------------
   Name - semFreeReal
   Purpose - Sets the semaphore at handle to be -1 or freed.
   Parameters - semaphore handle
   Returns - void 
   -------------------------------------------------------------------- */
void semFreeReal(int handle)
{

    SemTable[handle] = -1;
} /* semFreeReal */

/* getpid2----------------------------------------------------------------
   Name - getpid2
   Purpose - just returns the pid of the currently running process
   Parameters - takes in systemArgs
   Returns - void 
   -------------------------------------------------------------------- */
void getpid2(systemArgs * args)
{
    args->arg1 = getpid();
    setToUserMode();
} /* getpid2 */

/* setToUserMode----------------------------------------------------------
   Name - setToUserMode
   Purpose - to set the PSR to user mode
   Parameters - none
   Returns - none
   -------------------------------------------------------------------- */
void setToUserMode(){
    unsigned int currentMode = USLOSS_PsrGet();
    unsigned int newMode = currentMode & !1;
    int check = USLOSS_PsrSet(newMode);
} /* setToUserMode */

/* setToKernelMode--------------------------------------------------------
   Name - setToKernelMode
   Purpose - to set the PSR to kernel mode
   Parameters - none
   Returns - none
   -------------------------------------------------------------------- */
void setToKernelMode(){
    unsigned int currentMode = USLOSS_PsrGet();
    unsigned int newMode = currentMode | 1;
    int check = USLOSS_PsrSet(newMode);
} /* setToKernelMode */




