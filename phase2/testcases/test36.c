
/* A simple test of USLOSS_Syscall and sys_vec.  Makes a call to system trap 
 * number 0.  Should cause USLOSS to halt.
 */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>
#include <phase2.h>


extern void USLOSS_Syscall(void *arg);

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

void enableUserMode(){
    int result;

    result = USLOSS_PsrSet( USLOSS_PsrGet() & (~ USLOSS_PSR_CURRENT_MODE) );
    if ( result != USLOSS_DEV_OK ) {
        USLOSS_Console("enableUserMode(): USLOSS_PsrSet returned %d ", result);
        USLOSS_Console("Halting...\n");
        USLOSS_Halt(1);
    }
}


int start2(char *arg)
{
    systemArgs args;

    USLOSS_Console("start2(): putting itself into user mode\n");
    enableUserMode();

    args.number=0;

    USLOSS_Console("start2(): calling USLOSS_Syscall executing syscall 0, ");
    USLOSS_Console("this should halt\n");

    USLOSS_Syscall((void *)&args);

    USLOSS_Console("start2(): should not see this message!\n");
    quit(0);

    return 0;

} /* start2 */
