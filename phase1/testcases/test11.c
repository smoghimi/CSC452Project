/* A simple check for zap() and isZapped() */

#include <stdio.h>
#include <usloss.h>
#include <phase1.h>

int XXp1(char *);

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}


int
start1( char *arg ) 
{

    int status, kidpid, zap_result, join_result;

    USLOSS_Console("START1: calling fork1 for XXp1\n");
    kidpid = fork1("XXp1", XXp1, "XXp1", USLOSS_MIN_STACK, 5);

    USLOSS_Console("START1: calling zap\n");
    zap_result = zap(kidpid);
    USLOSS_Console("START1: zap_result = %d\n", zap_result);

    join_result = join(&status);
    USLOSS_Console("START1: join returned %d, status %d\n",
                   join_result, status);

    quit(0);
    return 0;

} /* start1 */


int
XXp1( char *arg )
{

    USLOSS_Console("XXp1: started\n");

    while ( isZapped() == 0 ) {
    }

    USLOSS_Console("XXp1: calling quit\n");
    quit(5);
    return 0;

} /* XXp1 */
