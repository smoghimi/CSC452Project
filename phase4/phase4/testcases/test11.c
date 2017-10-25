/*  DISKTEST
    - note that the test script should clean out the disk file
    each time before running this test.
    Write three sectors to the disk and then read them back.
    Do not span track boundaries. 
*/

#include <stdio.h>
#include <usloss.h>
#include <usyscall.h>
#include <libuser.h>
#include <assert.h>
#include <phase1.h>
#include <phase2.h>
#include <string.h>


char sectors[3 * 512];
char copy[3 * 512];

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start4(char *arg)
{
    int result;
    int status;

    USLOSS_Console("start4(): Testing disk 0\n");
    strcpy(&sectors[0 * 512], "This is a test\n");
    strcpy(&sectors[1 * 512], "Does it work?\n");
    strcpy(&sectors[2 * 512], "One last chance\n");
    result = DiskWrite((char *) sectors, 0, 4, 2, 3, &status);
    assert(result == 0);
    assert(status == 0);
    result = DiskRead((char *) copy, 0, 4, 2, 3, &status);
    USLOSS_Console("start4(): Read from disk: %s\n", &copy[0*512]);
    USLOSS_Console("start4(): Read from disk: %s\n", &copy[1*512]);
    USLOSS_Console("start4(): Read from disk: %s\n", &copy[2*512]);

    USLOSS_Console("start4(): Testing disk 1\n");
    strcpy(&sectors[0 * 512], "This is a test\n");
    strcpy(&sectors[1 * 512], "Does it work?\n");
    strcpy(&sectors[2 * 512], "One last chance\n");
    result = DiskWrite((char *) sectors, 1, 4, 2, 3, &status);
    assert(result == 0);
    assert(status == 0);
    result = DiskRead((char *) copy, 1, 4, 2, 3, &status);
    USLOSS_Console("start4(): Read from disk: %s\n", &copy[0*512]);
    USLOSS_Console("start4(): Read from disk: %s\n", &copy[1*512]);
    USLOSS_Console("start4(): Read from disk: %s\n", &copy[2*512]);

    USLOSS_Console("start4(): Terminating\n");
    Terminate(24);
    return 0;
} /* start4 */

