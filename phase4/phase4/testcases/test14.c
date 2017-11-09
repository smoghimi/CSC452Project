#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <libuser.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static char XXbuf[4][512];
int ubiq(int);

void test_setup(int argc, char *argv[])
{
}

void test_cleanup(int argc, char *argv[])
{
}

int k1(char *arg)
{
    printf("k1-1\n");
    ubiq(0);
    printf("k1-2\n");
    ubiq(3);
    printf("k1-3\n");
    ubiq(0);
    printf("k1-4\n");
    ubiq(5);
    printf("k1-5\n");
    ubiq(0);
    printf("k1-6\n");
    ubiq(5);
    printf("k1-7\n");
    ubiq(0); 
    printf("k1-8\n");
    Terminate(2);
    return 0;
}

int k2(char *arg)
{
    printf("k2-1\n");
    ubiq(5);
    printf("k2-2\n");
    ubiq(4);
    printf("k2-3\n");
    ubiq(1);
    printf("k2-4\n");
    ubiq(6);
    printf("k2-5\n");
    ubiq(1);
    printf("k2-6\n");
    ubiq(6);
    printf("k2-7\n");
    ubiq(1);
    printf("k2-8\n");
    Terminate(3);
    return 0;
}

int k3(char *arg)
{
    printf("k3-1\n");
    ubiq(9);
    printf("k3-2\n");
    ubiq(9);
    printf("k3-3\n");
    ubiq(2);
    printf("k3-4\n");
    ubiq(7);
    printf("k3-5\n");
    ubiq(2);
    printf("k3-6\n");
    ubiq(7);
    printf("k3-7\n");
    ubiq(2);
    printf("k3-8\n");
    Terminate(4);
    return 0;
}

int k4(char *arg)
{
    printf("k4-1\n");
    ubiq(2);
    printf("k4-2\n");
    ubiq(9);
    printf("k4-3\n");
    ubiq(3);
    printf("k4-4\n");
    ubiq(8);
    printf("k4-5\n");
    ubiq(3);
    printf("k4-6\n");
    ubiq(8);
    printf("k4-7\n");
    ubiq(3);
    printf("k4-8\n");
    Terminate(5);
    return 0;
}

int checkSum = 0;

int ubiq(int t)
{
    int status = -1;
    char buf[50];
    int index = t % 4;
    int numSectors = 1;
    int sectorStart = 1;
    if ( DiskWrite(XXbuf[index], numSectors, index, index, sectorStart, &status) < 0 ) {
	USLOSS_Console("ERROR: DiskPut\n");
    } 
    
    if ( status != 0 ) { 
	sprintf(buf,"disk_put returned error   %d\n",t);
	USLOSS_Console(buf);
    }  
    else {
	    checkSum += t;
    }
    return 0;
}


int start4(char *arg)
{
    int j, status, pid;
    
   USLOSS_Console("start4(): disk scheduling test, create 4 processes that write\n");
   USLOSS_Console("          various places on the disk.  After the 4 processes are\n");
   USLOSS_Console("          done, start4 reads the 4 locations & prints the\n");
   USLOSS_Console("          results.  Uses disk 1.\n");

    strcpy(XXbuf[0],"One flew East\n");
    strcpy(XXbuf[1],"One flew West\n");
    strcpy(XXbuf[2],"One flew over the coo-coo's nest\n");
    strcpy(XXbuf[3],"--did it work?\n");
    
    Spawn("k1", k1, NULL, USLOSS_MIN_STACK, 4, &pid);
    Spawn("k2", k2, NULL, USLOSS_MIN_STACK, 4, &pid);
    Spawn("k3", k3, NULL, USLOSS_MIN_STACK, 4, &pid);
    Spawn("k4", k4, NULL, USLOSS_MIN_STACK, 4, &pid);

    Wait(&pid, &status);
    printf("%i - %i\n", pid, status);
    DProcs();
    Wait(&pid, &status);
    printf("%i - %i\n", pid, status);
    Wait(&pid, &status);
    printf("%i - %i\n", pid, status);
    Wait(&pid, &status);
    printf("%i - %i\n", pid, status);

    // Erase previous contents of XXbuf
    strcpy(XXbuf[0],"xxx");
    strcpy(XXbuf[1],"xxx");
    strcpy(XXbuf[2],"xxx");
    strcpy(XXbuf[3],"xxx");

    printf("in test14\n");
    // Read items from disk that we just wrote
    for (j = 0; j < 4; j++) {
	status = -1;
	if (DiskRead(XXbuf[j], 1, j, j, 1, &status) < 0) {
	    USLOSS_Console("start4(): ERROR: DiskGet\n");
	}
	if (status < 0)
	  USLOSS_Console("start4(): disk_get returned error");
	USLOSS_Console("start4(): read: %s", XXbuf[j]);
    } 
    
    USLOSS_Console("start4(): done. checkSum = %d\n", checkSum);
    Terminate(2);
    return 0;
} /* start4 */
