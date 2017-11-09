#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <libuser.h>
#include <usloss.h>
#include <phase4.h>

int Child(char *arg);

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int start4(char *arg)
{
    int  kidpid, status;
    int  i;
    char buf[12];

    // This read is here since USLOSS seems to not want to begin
    // writing to any terminal unless a read has been done from one
    // terminal.  Does not matter from which terminal the read is done.
    // I am not sure if this is a "feature" of USLOSS or if there is
    // something wrong in my phase4 solution...  -- Patrick
    int j, len;
    int result;
    char data[256];
    len = 0;
    bzero(data, 256);
    USLOSS_Console("start4: reading a character from terminal 3\n");
    result = TermRead(data, 3, 1, &len);
    j = result;


    USLOSS_Console("start4(): Spawn 4 children. Each child writes one line\n");
    USLOSS_Console("          to each terminal\n");

    for (i = 0; i < 4; i++) {
        sprintf(buf, "%d", i);
        Spawn("Child", Child, buf, 2 * USLOSS_MIN_STACK, 4, &kidpid);
    }

    USLOSS_Console("start4(): calling Wait\n");

    for (i = 0; i < 4; i++) {
        Wait(&kidpid, &status);
    }

    USLOSS_Console("start4(): calling Terminate\n");
    Terminate(0);

    USLOSS_Console("start4(): should not see this message!\n");
    return 0;

} /* start4 */


int Child(char *arg)
{
    char buffer[MAXLINE];
    int  result, size;
    int  unit = atoi(arg);
    int  i;

    USLOSS_Console("Child(%d): started\n", unit);

    sprintf(buffer, "Child %d: A Something interesting to print here...",
            unit);
    switch(unit) {
    case 0:
        strcat(buffer, "zero\n");
        break;
    case 1:
        strcat(buffer, "one\n");
        break;
    case 2:
        strcat(buffer, "second\n");
        break;
    case 3:
        strcat(buffer, "three\n");
        break;
    }

    for (i = 0; i < 4; i++) {
        result = TermWrite(buffer, strlen(buffer), i, &size);
        if ( result < 0 || size != strlen(buffer) ) {
            USLOSS_Console("\n ***** Child(%d): ", unit);
            USLOSS_Console("got bad result or bad size! *****\n\n");
        }
        USLOSS_Console("Child(%d): done with write #%d\n", unit, i);
   }

   USLOSS_Console("Child(%d): terminating\n", unit);

   Terminate(1);

   USLOSS_Console("Child(%d): should not see this message!\n", unit);
   return 1;

} /* Child */
