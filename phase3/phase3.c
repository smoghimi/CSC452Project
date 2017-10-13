#include <usloss.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>

/* ------------------------- Function Prototypes ------------------------- */
/* ---------- Externs ---------- */
extern int start3(char*);

/* ---------- Void Prototypes ---------- */
void spawn(systemArgs * args);
void wait2(systemArgs * args);
void setToUserMode();

/* ---------- Int Prototypes ---------- */
int spawnReal(char* name, int(*func)(char *), char*arg, int stacksize, int priority);
int waitReal(int*);
int spawnLaunch();

/* ------------------------- Globals ------------------------- */
p3proc ProcTable[MAXPROC];

int start2(char *arg)
{
    int pid;
    int status;
    /*
     * Check kernel mode here.
     */

    /*
     * Data structure initialization as needed...
     */
    systemCallVec[SYS_SPAWN] = spawn;
    systemCallVec[SYS_WAIT]  = wait2;

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
    if(args->arg1 == NULL){
        USLOSS_Console("Null function pointer. Halting...(Shaion you will need to change this)\n");
        USLOSS_Halt(1);
    }
    if((int)args->arg3 < USLOSS_MIN_STACK){
        USLOSS_Console("Stacksize is too small. Halting...(Shaion you will need to change this)\n");
        USLOSS_Halt(1);
    }
    if(args->arg4 < 0 || (int)args->arg4 >= 6){
        USLOSS_Console("Priority is out of bounds. Halting...(Shaion you will need to change this)\n");
        USLOSS_Halt(1);
    }
    int pid = spawnReal(args->arg5, args->arg1, args->arg2, (int)args->arg3, (int)args->arg4);
    ProcTable[pid%MAXPROC].startFunc = args->arg1;
    ProcTable[pid%MAXPROC].arg = args->arg2;
    args->arg1 = pid;
    setToUserMode();
} /* spawn */

void setToUserMode(){
    unsigned int currentMode = USLOSS_PsrGet();
    unsigned int newMode = currentMode & !1;
    int check = USLOSS_PsrSet(newMode);
}

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
    args->arg1 = waitReal(args->arg2);
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

