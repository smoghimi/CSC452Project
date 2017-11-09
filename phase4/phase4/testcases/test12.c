/* DISKTEST
   - note that the test script should clean out the disk file
   each time before running this test.
   Write three sectors to the disk and then read them back. 
   Do not span track boundaries. Check all 3*512 bytes are correct.
*/

#include <stdio.h>
#include <usloss.h>
#include <usyscall.h>
#include <libuser.h>
#include <assert.h>
#include <phase1.h>
#include <phase2.h>

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
    int i;
    int failed;

    for ( i = 0; i < 512 * 3; i++ ) {
        sectors[i] = ((i % 13) * 3456);
    }
    USLOSS_Console("start4(): Writing data to 3 disk sectors, then reading them back\n");
    USLOSS_Console("          Confirm that data read back matches data read in\n");

    USLOSS_Console("start4(): Testing Disk 0\n");
    failed = 0;
    result = DiskWrite((char *) sectors, 0, 4, 2, 3, &status);
    assert(result == 0);
    assert(status == 0);
    result = DiskRead((char *) copy, 0, 4, 2, 3, &status);
    for ( i = 0; i < 512 * 3; i++ ) {
        if (copy[i]!=sectors[i]) {
            USLOSS_Console("start4(): Buffer read back fm disk 0 invalid at byte %d\n", i);
            failed = 1;
            break;
        }
    }
    if ( ! failed )
        USLOSS_Console("start4(): Test of disk 0 succeeded.\n");

   USLOSS_Console("start4(): Testing Disk 1\n");
   failed = 0;
   result = DiskWrite((char *) sectors, 1, 4, 2, 3, &status);
   assert(result == 0);
   assert(status == 0);
   result = DiskRead((char *) copy, 1, 4, 2, 3, &status);
   for ( i = 0; i < 512 * 3; i++ ) {
       if ( copy[i]!=sectors[i] ) {
           USLOSS_Console("start4(): Buffer read back fm disk 1 invalid at byte %d\n", i);
         failed = 1;
         break;
       }
    }
    if ( ! failed )
        USLOSS_Console("start4(): Test of disk 1 succeeded.\n");

    USLOSS_Console("start4(): Done.\n");

    Terminate(24);
    return 0;
} /* start4 */

