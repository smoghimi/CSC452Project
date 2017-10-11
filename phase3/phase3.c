#include <usloss.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>

void spawn(systemArgs * args);
int spawnReal(char* name, int(*func)(char *), char*arg, int stacksize, int priority);
int waitReal(int*);
extern int start3(char*);
int spawnLaunch();


p3proc ProcTable[MAXPROC];

int
start2(char *arg)
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

    /* Call the waitReal version of your wait code here.
     * You call waitReal (rather than Wait) because start2 is running
     * in kernel (not user) mode.
     */
    pid = waitReal(&status);

    return 0;
} /* start2 */

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
    printf("%i\n", pid);
}

int spawnReal(char * name, int(*startFunc)(char *), char * arg, int stacksize, int priority)
{
    int a = fork1(name, spawnLaunch, arg, stacksize, priority);
    return a;
}

int spawnLaunch()
{
    unsigned int a = USLOSS_PsrGet();
    a ^= USLOSS_PSR_CURRENT_MODE;
    USLOSS_PsrSet(a);
    start3("a");
    return 0;
}

int waitReal(int* status)
{
    return 0;
}

