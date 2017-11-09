/* TERMTEST
 * Read exactly 13 bytes from term 1. Display the bytes to stdout.
 */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <libuser.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

char buf[256];

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start4(char *arg)
{
    int j, length;
    char dataBuffer[256];
    int result;
  
    USLOSS_Console("start4(): Read from terminal 1, but ask for fewer\n");
    USLOSS_Console("          chars than are present on the first line.\n");

    length = 0;
    memset(dataBuffer, 'a', 256);  // Fill dataBuffer with a's
    dataBuffer[254] = '\n';
    dataBuffer[255] = '\0';

    result = TermRead(dataBuffer, 13, 1, &length);
    if (result < 0) {
        USLOSS_Console("start4(): ERROR from Readterm, result = %d\n", result);
        Terminate(0);
    }	

    USLOSS_Console("start4(): term1 read %d bytes, first 13 bytes: `", length);
    USLOSS_Console(buf);
    for (j = 0; j < 13; j++)
        USLOSS_Console("%c", dataBuffer[j]);	    
    USLOSS_Console("'\n");
  
    USLOSS_Console("start4(): simple terminal test is done.\n");

    Terminate(3);

    return 0;

} /* start4 */
