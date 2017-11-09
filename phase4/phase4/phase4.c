#include <providedPrototypes.h>
#include <stdio.h>
#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>
#include <phase4.h>
#include <stdlib.h>

#define INDEX getpid()%MAXPROC

// This macro will help us get rid of the unused variable warning
#define UNUSED {		\
	if(result < 0) 		\
		printf("Should never see this.\n"); 	\
}

#define CHECKMODE {    	\
	if (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) { \
		USLOSS_Console("Trying to invoke syscall from kernel\n"); \
		USLOSS_Halt(1);  \
	}  \
}

#define CHECKKERNEL {	\
	if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0) {	\
		USLOSS_Console("System should be in kernel mode. Halting\n");	\
		USLOSS_Halt(1);	\
	}	\
}

static int	ClockDriver(char *);
static int	DiskDriver(char *);
static int  TermDriver(char *);

/* ---------- Globals ---------- */
p4proc      	ProcTable[MAXPROC];
p4proc 			diskTable[2];
procQ       	SleepQueue;
qPtr 			head = &SleepQueue;
diskRequest		diskHead[2];
int 			requestID = 0;	// This will just be like a PID but for disk requests
int 			diskTrack[2];
int         	semRunning;
int 			dqSem;			// Semaphore for accessing the disk Q.
int         	clockProcs;
int         	dFlag = 0;

void start3(void)
{
	//Check kernel mode here.
	CHECKKERNEL;

	for (int i = 0; i < MAXPROC; i++){
		ProcTable[i].pSem = semcreateReal(0);
	}

	systemCallVec[SYS_SLEEP]     = (void *) sleep;
	systemCallVec[SYS_TERMREAD]  = (void *) termread;
	systemCallVec[SYS_TERMWRITE] = (void *) termwrite;
	systemCallVec[SYS_DISKREAD]  = (void *) diskread;
	systemCallVec[SYS_DISKSIZE]  = (void *) disksize;
	systemCallVec[SYS_DISKWRITE] = (void *) diskwrite;

	/*
	 * Create clock device driver 
	 * I am assuming a semaphore here for coordination.  A mailbox can
	 * be used instead -- your choice.
	 */
	clockProcs = 0;
	semRunning = semcreateReal(0);
	int	clockPID;
	clockPID = fork1("Clock driver", ClockDriver, NULL, USLOSS_MIN_STACK, 2);
	if (clockPID < 0) {
	USLOSS_Console("start3(): Can't create clock driver\n");
	USLOSS_Halt(1);
	}
	/*
	 * Wait for the clock driver to start. The idea is that ClockDriver
	 * will V the semaphore "semRunning" once it is running.
	 */

	sempReal(semRunning);

	/*
	 * Create the disk device drivers here.  You may need to increase
	 * the stack size depending on the complexity of your
	 * driver, and perhaps do something with the pid returned.
	 */
	int disk0pid, disk1pid;
	disk0pid = fork1("Disk0Driver", DiskDriver, "0", USLOSS_MIN_STACK, 2);
	diskTable[0].pid = disk0pid;
	diskTable[0].pSem = semcreateReal(0);

	disk1pid = fork1("Disk1Driver", DiskDriver, "1", USLOSS_MIN_STACK, 2);
	diskTable[1].pid = disk1pid;
	diskTable[1].pSem = semcreateReal(0);

	dqSem = semcreateReal(0);
	semvReal(dqSem);
	
	// May be other stuff to do here before going on to terminal drivers

	/*
	 * Create terminal device drivers.
	 */
	//int term0pid, term1pid, term2pid, term3pid;
	//term0pid = fork1("Term0Driver", TermDriver, "0", USLOSS_MIN_STACK, 2);
	//term1pid = fork1("Term1Driver", TermDriver, "1", USLOSS_MIN_STACK, 2);
	//term2pid = fork1("Term2Driver", TermDriver, "2", USLOSS_MIN_STACK, 2);	
	//term3pid = fork1("Term3Driver", TermDriver, "3", USLOSS_MIN_STACK, 2);

	/*
	 * Create first user-level process and wait for it to finish.
	 * These are lower-case because they are not system calls;
	 * system calls cannot be invoked from kernel mode.
	 * I'm assuming kernel-mode versions of the system calls
	 * with lower-case first letters, as shown in provided_prototypes.h
	 */
	int pid = spawnReal("start4", start4, NULL, 4 * USLOSS_MIN_STACK, 3);
	int	status;
	pid = waitReal(&status);

	/*
	 * Zap the device drivers
	 */
	zap(clockPID);  // clock driver
	//dumpProcesses();

	if(diskTable[0].status == DBLOCKED){
		semvReal(diskTable[0].pSem);
	}
	zap(disk0pid);

	if(diskTable[1].status == DBLOCKED){
		semvReal(diskTable[1].pSem);
	}
	zap(disk1pid);

	//zap(term0pid);
	//zap(term1pid);
	//zap(term2pid);
	//zap(term3pid);

	// eventually, at the end:
	quit(0);   
}

/* Sleep -----------------------------------------------------------------
   Purpose : Sleep's purpose is to be the function that a program calls
				when it wants to be delayed for some reason.
 Arguments : Takes an int as the number of seconds that it wants to sleep
   Returns : int
 * ---------------------------------------------------------------------*/
int Sleep(int seconds)
{
	if(dFlag){
		printf("Sleep(): Calling sleep with seconds.\n");
		printf(" -- seconds = %i\n\n", seconds);
	}
	USLOSS_Sysargs sysArg;

	CHECKMODE;
	sysArg.number = SYS_SLEEP;
	sysArg.arg1 = (void *)(long) seconds;

	USLOSS_Syscall(&sysArg);

	return (long) sysArg.arg4;
} /* Sleep */

/* sleep -----------------------------------------------------------------
   Purpose : sleep's purpose is to be the function that the system call 
				vector assigns to the sleep syscall.
 Arguments : Takes systemArgs as input
				arg1 : Seconds to sleep
   Returns : void
 * ---------------------------------------------------------------------*/
void sleep(systemArgs * args)
{
	if(dFlag){
		USLOSS_Console("sleep(): Calling sleep with args.\n");
		USLOSS_Console(" -- args.arg1 = %i\n\n", args->arg1);
	}
	int seconds = (long) args->arg1;
	if(seconds < 0){
		args->arg4 = (void *)(long) -1;
	} else {
		args->arg4 = (void *)(long) 0;
		sleepReal(seconds);
	}
} /* sleep */

/* sleepReal -------------------------------------------------------------
   Purpose : sleepReal is the function that actually does the sleeping.
 Arguments : Takes an int as the number of seconds that it wants to sleep
   Returns : void
 * ---------------------------------------------------------------------*/
void sleepReal(int seconds)
{
	if (dFlag){
		USLOSS_Console("sleepReal(): calling sleepReal with seconds.\n");
		USLOSS_Console(" -- seconds = %i\n\n", seconds);
	}

	clockProcs++;

	// mark our status as being asleep in the proctable
	int pid;
	pid = getpid();
	ProcTable[pid%MAXPROC].status = ASLEEP;
	// Calculate the time from now + seconds
	int time;
	time = readtime();
	int wakeUpTime = time + (seconds * 1000000);

	// add ourself to the Queue
	// if no one is in the Q yet
	if (head->process <= 0){
		head->process = pid;
		head->wakeUpTime = wakeUpTime;
	} // else if there are others in the q we must arrange them by wake-up-time
	else {
		if (wakeUpTime < head->wakeUpTime){
			procQ temp;
			temp.process = pid;
			temp.wakeUpTime = wakeUpTime;
			temp.next = head;
			head = &temp;
		}
		else {
			qPtr iter = head;
			while (iter->next != NULL && wakeUpTime > iter->next->wakeUpTime){
				iter = iter->next;
			}
			procQ temp;
			temp.process = pid;
			temp.wakeUpTime = wakeUpTime;
			temp.next = iter->next;
			iter->next = &temp;
		}
	}
	if (dFlag)
		printQ();
	sempReal(ProcTable[INDEX].pSem);
}
 /* sleepReal */

/* TermRead --------------------------------------------------------------
   Purpose : TermRead's purpose is to be the function that a program calls
				when it wants to read in from a terminal.
 Arguments : Takes multiple arguments
				buff: The address of the user's buffer
				bsize: The maximum size of the buffer
				unit_id: The unit number of the terminal to read
				nread: The number of character's read
   Returns : int
 * ---------------------------------------------------------------------*/
int TermRead(char * buff, int bsize, int unit_id, int *nread)
{
	if(dFlag){
		USLOSS_Console("TermRead(): Calling TermRead with:\n");
		USLOSS_Console(" -- buff = %i\n", buff);
		USLOSS_Console(" -- bsize = %i\n", bsize);
		USLOSS_Console(" -- unit_id = %i\n", unit_id);
		USLOSS_Console(" -- nread = %i\n\n", nread);
	}
	USLOSS_Sysargs sysArgs;

	CHECKMODE;
	sysArgs.number = SYS_TERMREAD;

	sysArgs.arg1 = (void *)(long)buff;
	sysArgs.arg2 = (void *)(long)bsize;
	sysArgs.arg3 = (void *)(long)unit_id;

	USLOSS_Syscall(&sysArgs);

	*nread = (long)sysArgs.arg2;
	return (long) sysArgs.arg4;
} /* TermRead */

/* termread --------------------------------------------------------------
   Purpose : termread's purpose is to be the function that the system call 
				vector assigns to the termread syscall.
 Arguments : Takes systemArgs as input
				arg1: address of the line buffer
				arg2: maximum size of the buffer
				arg3: the unit number of the terminal to read from
   Returns : void
 * ---------------------------------------------------------------------*/
void termread(systemArgs * args)
{
	if(dFlag){
		USLOSS_Console("termread(): Calling termread with\n");
		USLOSS_Console(" -- arg1 = %i\n", args->arg1);
		USLOSS_Console(" -- arg2 = %i\n", args->arg2);
		USLOSS_Console(" -- arg3 = %i\n\n", args->arg3);
	}

	// unpack args
	char * buff = (char *)args->arg1;
	int bsize = (long)args->arg2;
	int unit = (long)args->arg3;

	// check if our arguments are valid
	if(bsize <= 0 || (unit < 0 || unit > 3)){
		args->arg2 = 0;
		args->arg4 = 0;
	} else {
		int read = termreadReal(buff, bsize, unit);
		args->arg2 = (void *)(long) read;
	}
} /* termread */

/* termreadReal ----------------------------------------------------------
   Purpose : termreadReal is the function that actually does the reading
				from a terminal.
 Arguments : Takes multiple arguments
				buff: the address of the line buffer
				bsize: the maximum size of the buffer
				unit: the unit of the terminal to read from
   Returns : int
 * ---------------------------------------------------------------------*/
int termreadReal(char * buff, int bsize, int unit)
{
	if(dFlag){
		USLOSS_Console("termreadReal(): Calling termreadReal with\n");
		USLOSS_Console(" -- buff = %i\n", buff);
		USLOSS_Console(" -- bsize = %i\n", bsize);
		USLOSS_Console(" -- unit = %i\n\n", unit);
	}
	return 0;
} /* termreadReal */

int TermWrite(char * buff, int size, int unit, int * nwrite){
        if(dFlag){
                USLOSS_Console("TermRead(): Calling TermRead with:\n");
                USLOSS_Console(" -- buff = %i\n", buff);
                USLOSS_Console(" -- bsize = %i\n", size);
                USLOSS_Console(" -- unit_id = %i\n", unit);
                USLOSS_Console(" -- nread = %i\n\n", nwrite);
        }
	USLOSS_Sysargs sysArgs;

        CHECKMODE;
        sysArgs.number = SYS_TERMWRITE;

        sysArgs.arg1 = (void *)(long)buff;
        sysArgs.arg2 = (void *)(long)size;
        sysArgs.arg3 = (void *)(long)unit;

        USLOSS_Syscall(&sysArgs);

        *nwrite = (long)sysArgs.arg2;
        return (long) sysArgs.arg4;
}

void termwrite(systemArgs * args){
        if(dFlag){
                USLOSS_Console("termread(): Calling termread with\n");
                USLOSS_Console(" -- arg1 = %i\n", args->arg1);
                USLOSS_Console(" -- arg2 = %i\n", args->arg2);
                USLOSS_Console(" -- arg3 = %i\n\n", args->arg3);
        }

        // unpack args
        char* buff = (char *)args->arg1;
        int size = (long)args->arg2;
        int unit = (long)args->arg3;

        // check if our arguments are valid
        if(size <= 0 || (unit < 0 || unit > 3)){
                args->arg2 = 0;
                args->arg4 = 0;
        } else {
                int write = termwriteReal(buff, size, unit);
                args->arg2 = (void *)(long) write;
        }
}

int termwriteReal(char * buff, int size, int unit){
        if(dFlag){
                USLOSS_Console("termreadReal(): Calling termreadReal with\n");
                USLOSS_Console(" -- buff = %i\n", buff);
                USLOSS_Console(" -- bsize = %i\n", size);
                USLOSS_Console(" -- unit = %i\n\n", unit);
        }
        return 0;
}

/* DiskRead --------------------------------------------------------------
   Purpose : DiskReads's purpose is to be the function that a program calls
                when it wants to read in from a disk.
 Arguments : Takes multiple arguments
                dbuff: The address of the user's buffer
                unit: the disk unit
                track: the starting disk track number
                first: the starting disk sector number
                sectors: the number of sectors to read
   Returns : int
 * ---------------------------------------------------------------------*/
int DiskRead(void * dbuff, int unit, int track, int first, int sectors, int *status)
{
    if(dFlag){
        USLOSS_Console("DiskRead(): Calling DiskRead with\n");
        USLOSS_Console(" -- dbuff   = %i\n", dbuff);
        USLOSS_Console(" -- unit    = %i\n", unit);
        USLOSS_Console(" -- track   = %i\n", track);
        USLOSS_Console(" -- first   = %i\n", first);
        USLOSS_Console(" -- sectors = %i\n\n", sectors);
    }

    USLOSS_Sysargs sysArgs;

    CHECKMODE;
    sysArgs.number = SYS_DISKREAD;

    sysArgs.arg1 = dbuff;
    sysArgs.arg2 = (void *)(long) sectors;
    sysArgs.arg3 = (void *)(long) track;
    sysArgs.arg4 = (void *)(long) first;
    sysArgs.arg5 = (void *)(long) unit;

    USLOSS_Syscall(&sysArgs);
    *status = (long) sysArgs.arg1;
    return (long) sysArgs.arg4;

} /* DiskRead */

/* diskread --------------------------------------------------------------
   Purpose : termread's purpose is to be the function that the system call 
				vector assigns to the termread syscall.
 Arguments : Takes systemArgs as input
				arg1: the memory address to transfer to
				arg2: number of sectors to read
				arg3: the starting disk track number
				arg4: the starting disk sector number
				arg5: the unit number of the disk to read
   Returns : void
 * ---------------------------------------------------------------------*/
void diskread(systemArgs * args)
{
	if(dFlag){
        USLOSS_Console("diskread(): Calling diskread with\n");
        USLOSS_Console(" -- arg1 = %i\n", args->arg1);
        USLOSS_Console(" -- arg2 = %i\n", args->arg2);
        USLOSS_Console(" -- arg3 = %i\n", args->arg3);
        USLOSS_Console(" -- arg4 = %i\n", args->arg4);
        USLOSS_Console(" -- arg5 = %i\n\n", args->arg5);
    }

    // Unpack args
	int unit     = (long) args->arg5;
	int track    = (long) args->arg3;
	int first    = (long) args->arg4;
	int sectors  = (long) args->arg2;
	void * dbuff = args->arg1;

	// Check if the inputs are valid.
	if(unit < 0 || unit > 1) { 	// Check to see if the unit is wrong
		args->arg4 = (void *)(long) -1;
	}
	else if (track < 0) { 		// Check to see if the track is wrong
		args->arg4 = (void *)(long) -1;
	}
	else if (first < 0) { 		// Check to see if the starting sector is wrong
		args->arg4 = (void *)(long) -1;
	}
	else if (sectors < 0) { 	// Check to see if the # of sectors is wrong
		args->arg4 = (void *)(long) -1;
	}
	else {
		args->arg1 = (void *)(long) diskreadReal((long) args->arg5, (long) args->arg3, (long) args->arg4, (long) args->arg2, args->arg1);
		args->arg4 = 0;
	}
} /* diskread */

/* diskreadReal ----------------------------------------------------------
   Purpose : diskreadReal is the function that actually reads from the 
   				disk.
 Arguments : Takes multiple arguments
				buff: the address of the line buffer
				bsize: the maximum size of the buffer
				unit: the unit of the terminal to read from
   Returns : int
 * ---------------------------------------------------------------------*/
int diskreadReal(int unit, int track, int first, int sectors, void * buffer)
{
	if(dFlag){
        USLOSS_Console("diskreadReal(): Calling diskreadReal with\n");
        USLOSS_Console(" -- unit    = %i\n", unit);
        USLOSS_Console(" -- track   = %i\n", track);
        USLOSS_Console(" -- first   = %i\n", first);
        USLOSS_Console(" -- sectors = %i\n", sectors);
        USLOSS_Console(" -- buffer  = %s\n", buffer);
    }

    // Create our disk request
    diskRequest dreq;
    dreq.id 	 = requestID++;
    dreq.dbuff   = buffer;
    dreq.unit    = unit;
    dreq.track   = track;
    dreq.first   = first;
    dreq.sectors = sectors;
    dreq.rw		 = DISKREAD;
    dreq.active	 = ACTIVE;
    dreq.process = &ProcTable[INDEX];
    dreq.process->dbuff = buffer;

    // Put our disk request on the Q
    ProcTable[INDEX].pid = getpid();
    ProcTable[INDEX].status = DQSLEEP;
    sempReal(dqSem);	// Gain access to the diskQ
    ProcTable[INDEX].status = 1;
    AddToQ(&dreq, unit);
    semvReal(dqSem);	// Release access to the diskQ

    semvReal(diskTable[unit].pSem);

    //block on our own private mailbox/sem
    ProcTable[INDEX].status = PSLEEP;
	printf("P\n");
    sempReal(ProcTable[INDEX].pSem);

    //return some sort of results
    return ProcTable[INDEX].success;
} /* diskreadReal */

/* DiskWrite -------------------------------------------------------------
   Purpose : DiskWrite's purpose is to be the function that a program calls
                when it wants to write to a disk.
 Arguments : Takes multiple arguments
                dbuff: The address of the user's buffer
                unit: the disk unit
                track: the starting disk track number
                first: the starting disk sector number
                sectors: the number of sectors to read
   Returns : int
 * ---------------------------------------------------------------------*/
int DiskWrite(void * dbuff, int unit, int track, int first, int sectors, int * status)
{
    if(dFlag){
        USLOSS_Console("DiskWrite(): Calling DiskWrite with\n");
        USLOSS_Console(" -- dbuff   = %s", dbuff);
        USLOSS_Console(" -- unit    = %i\n", unit);
        USLOSS_Console(" -- track   = %i\n", track);
        USLOSS_Console(" -- first   = %i\n", first);
        USLOSS_Console(" -- sectors = %i\n\n", sectors);
    }

    USLOSS_Sysargs sysArgs;

    CHECKMODE;
    sysArgs.number = SYS_DISKWRITE;

    sysArgs.arg1 = dbuff;
    sysArgs.arg2 = (void *)(long) sectors;
    sysArgs.arg3 = (void *)(long) track;
    sysArgs.arg4 = (void *)(long) first;
    sysArgs.arg5 = (void *)(long) unit;

    USLOSS_Syscall(&sysArgs);
    *status = (long) sysArgs.arg1;
    return (long) sysArgs.arg4;
} /* DiskWrite */

/* diskwrite -------------------------------------------------------------
   Purpose : disksize's purpose is to be the function that the system call 
				vector assigns to the disksize syscall.
 Arguments : Takes systemArgs as input
				arg1: the memory address to write from
				arg2: the number of sectors to write
				arg3: the starting disk track number
				arg4: the starting disk sector number
				arg5: the unit of the disk
   Returns : void
 * ---------------------------------------------------------------------*/
void diskwrite(systemArgs * args)
{
	if(dFlag){
		USLOSS_Console("diskwrite(): Calling diskwrite with\n");
        USLOSS_Console(" -- arg1 = %s", args->arg1);
        USLOSS_Console(" -- arg2 = %i\n", args->arg2);
        USLOSS_Console(" -- arg3 = %i\n", args->arg3);
        USLOSS_Console(" -- arg4 = %i\n", args->arg4);
        USLOSS_Console(" -- arg5 = %i\n\n", args->arg5);
	}

	// Unpack args
	int unit     = (long) args->arg5;
	int track    = (long) args->arg3;
	int first    = (long) args->arg4;
	int sectors  = (long) args->arg2;
	void * dbuff = args->arg1;

	// Check if the inputs are valid.
	if(unit < 0 || unit > 1) { 	// Check to see if the unit is wrong
		args->arg4 = (void *)(long) -1;
	}
	else if (track < 0) { 		// Check to see if the track is wrong
		args->arg4 = (void *)(long) -1;
	}
	else if (first < 0) { 		// Check to see if the starting sector is wrong
		args->arg4 = (void *)(long) -1;
	}
	else if (sectors < 0) { 	// Check to see if the # of sectors is wrong
		args->arg4 = (void *)(long) -1;
	}
	else {
		args->arg1 = (void *)(long) diskwriteReal((long) args->arg5, (long) args->arg3, (long) args->arg4, (long) args->arg2, args->arg1);
		args->arg4 = 0;
	}
} /* diskwrite */

/* diskwriteReal ---------------------------------------------------------
   Purpose : diskreadReal is the function that actually reads from the 
   				disk.
 Arguments : Takes multiple arguments
				unit    : the disk unit to write to
				track   : the track to start writing on
				first   : the first sector to write on
				sectors : the number of sectors to write
				buffer  : what we are writing onto the sectors
   Returns : int
 * ---------------------------------------------------------------------*/
int diskwriteReal(int unit, int track, int first, int sectors, void * buffer)
{
	if(dFlag){
        USLOSS_Console("diskwriteReal(): Calling diskwriteReal with\n");
        USLOSS_Console(" -- unit    = %i\n", unit);
        USLOSS_Console(" -- track   = %i\n", track);
        USLOSS_Console(" -- first   = %i\n", first);
        USLOSS_Console(" -- sectors = %i\n", sectors);
        USLOSS_Console(" -- buffer  = %s\n", buffer);
    }

    // Create our disk request
    diskRequest dreq;
    dreq.id 	 = requestID++;
    dreq.dbuff   = buffer;
    dreq.unit    = unit;
    dreq.track   = track;
    dreq.first   = first;
    dreq.sectors = sectors;
    dreq.rw		 = DISKWRITE;
    dreq.active	 = ACTIVE;
    dreq.process = &ProcTable[INDEX];

    // Put our disk request on the Q
    ProcTable[INDEX].pid = getpid();
    ProcTable[INDEX].status = DQSLEEP;
    sempReal(dqSem);	// Gain access to the diskQ
    ProcTable[INDEX].status = 1;
    AddToQ(&dreq, unit);
    semvReal(dqSem);	// Release access to the diskQ

    //wake up disk process, this depends on the unit
    semvReal(diskTable[unit].pSem);

    //block on our own private mailbox/sem
    ProcTable[INDEX].status = PSLEEP;
	printf("P\n");
    sempReal(ProcTable[INDEX].pSem);

    //return some sort of results
    return ProcTable[INDEX].success;
} /* diskwriteReal */

/* DiskSize --------------------------------------------------------------
   Purpose : DiskSize's purpose is to be the function that a program calls
                when it wants to find the size of a disk.
 Arguments : Takes multiple arguments
                unit   : the disk unit
                sector : the size of a sector
                track  : the sectors in a track
                disk   : the number of tracks on a disk
   Returns : int
 * ---------------------------------------------------------------------*/
int DiskSize(int unit, int * sector, int * track, int * disk)
{
    if(dFlag){
        USLOSS_Console("DiskSize(): Calling DiskSize with\n");
        USLOSS_Console(" -- unit   = %i\n", unit);
        USLOSS_Console(" -- sector = %i\n", sector);
        USLOSS_Console(" -- track  = %i\n", track);
        USLOSS_Console(" -- disk   = %i\n", disk);
    }

    USLOSS_Sysargs sysArgs;

    CHECKMODE;
    sysArgs.number = SYS_DISKSIZE;

    sysArgs.arg1 = (void *)(long) unit;

    USLOSS_Syscall(&sysArgs);

    *sector = (long) sysArgs.arg1;
    *track  = (long) sysArgs.arg2;
    *disk   = (long) sysArgs.arg3;
    return (long) sysArgs.arg4;
} /* DiskSize */

/* disksize --------------------------------------------------------------
   Purpose : disksize's purpose is to be the function that the system call 
				vector assigns to the disksize syscall.
 Arguments : Takes systemArgs as input
				arg1: the disk unit to get the size of.
   Returns : void
 * ---------------------------------------------------------------------*/
void disksize(systemArgs * args)
{
	if(dFlag){
		USLOSS_Console("disksize(): Calling disksize with\n");
        USLOSS_Console(" -- arg1 = %i\n", args->arg1);
	}

	// Check if inputs are valid.
	if((long) args->arg1 < 0 || (long) args->arg1 > 1) {		// Check to see if the unit is valid
		args->arg4 = (void *)(long) -1;
	} 
	else {
		disksizeReal((long) args->arg5, args->arg1, args->arg2, args->arg3);
		args->arg4 = 0;
	}
} /* disksize */

/* disksizeReal ----------------------------------------------------------
   Purpose : disksizeReal is the function that actually gets the size
   				info from the disk
 Arguments : Takes multiple arguments
				buff: the address of the line buffer
				bsize: the maximum size of the buffer
				unit: the unit of the terminal to read from
   Returns : int
 * ---------------------------------------------------------------------*/
void disksizeReal(int unit, int * sector, int * track, int * disk)
{
	int tracks;
	USLOSS_DeviceRequest devReq;
	devReq.opr = USLOSS_DISK_TRACKS;
	devReq.reg1 = &tracks;
	int result = USLOSS_DeviceOutput(USLOSS_DISK_DEV, unit, &devReq);
	UNUSED;
	int status;
	result = waitDevice(USLOSS_DISK_DEV, unit, &status);
	*disk = tracks;
	*sector = USLOSS_DISK_SECTOR_SIZE;
	*track = USLOSS_DISK_TRACK_SIZE;
} /* disksizeReal */

/* ClockDriver -----------------------------------------------------------
   Purpose : ClockDriver's purpose is to essentially allow programs to 
				processes to have a delay
 Arguments : Takes a char as input
   Returns : an int.
 * ---------------------------------------------------------------------*/
static int ClockDriver(char *arg)
{
	if (dFlag){
		printf("ClockDriver(): Starting ClockDriver\n\n");
	}
	int result;
	int status;

	// Let the parent know we are running and enable interrupts.
	semvReal(semRunning);
	result = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
	UNUSED;

	// Infinite loop until we are zap'd
	while(! isZapped()) {
		result = waitDevice(USLOSS_CLOCK_DEV, 0, &status);
		while (clockProcs){
			int time;
			gettimeofdayReal(&time);
			while(time > head->wakeUpTime && head->process > 0){
				semvReal(ProcTable[head->process%MAXPROC].pSem);
				if(head->next != NULL){
					head = head->next;
				}
				else {
					head->wakeUpTime = 0;
					head->process = 0;
				}
				clockProcs--;
			}
			result = waitDevice(USLOSS_CLOCK_DEV, 0, &status);
		}
	}
	return 0;
} /* ClockDriver */

/* TermDriver ------------------------------------------------------------
   Purpose : Not sure yet
 Arguments : Takes a char as input
   Returns : an int.
 * ---------------------------------------------------------------------*/
static int TermDriver(char *arg)
{
	int terminal = atoi(arg);
	if(dFlag){
		USLOSS_Console("TermDriver(): Starting TermDriver with\n");
		USLOSS_Console(" -- arg = %i\n\n", terminal);
	}
	while(! isZapped()){
		//do nothing;
	}
	return 0;
} /* TermDriver */

/* DiskDriver ------------------------------------------------------------
   Purpose : Not sure yet
 Arguments : Takes a char as input
   Returns : an int.
 * ---------------------------------------------------------------------*/
static int DiskDriver(char *arg)
{
	int disk = atoi(arg);
	if(dFlag){
		USLOSS_Console("DiskDriver(): Starting DiskDriver with\n");
		USLOSS_Console(" -- arg = %i\n\n", disk);
	}

	// Find out how many tracks are on this disk
	int tracks, sectorSize, sectors;
	disksizeReal(disk, &sectorSize, &sectors, &tracks);

	// infinite loop on our sem
	while(! isZapped())
	{
		diskTable[disk].status = DBLOCKED;
		if(dFlag)
			USLOSS_Console(" -- Disk %i going to sleep.\n", disk);
		sempReal(diskTable[disk].pSem);
		diskTable[disk].status = DRUNNING;
		if(dFlag)
			USLOSS_Console(" -- Disk %i waking up.\n", disk);
		
		if(! isZapped()){
			// Gain access to the diskQ
			if(diskHead[disk].process != NULL){ 	// If there is a request...
				sempReal(dqSem);
				drPtr request = &diskHead[disk];
				semvReal(dqSem);
				diskTrack[disk] = request->track;
				request->active = 0;
				
				// Read or write to the disk
				if(request->rw == DISKWRITE){
					Write(request, tracks * sectors);
					printf("V\n");
					semvReal(request->process->pSem);
				}
				else {
					Read(request, tracks * sectors);
					semvReal(request->process->pSem);
				}

				// Adjust the Q
				sempReal(dqSem);
				if(request->next->active != ACTIVE){	// If that was the only request...
					diskRequest dreq;
					diskHead[disk] = dreq;
				} 
				else{									// If there is more than one request...
					diskHead[disk] = *diskHead[disk].next;
				}

				// Release access to the diskQ
				semvReal(dqSem);
			}
		}
		
	}
	return 0;
} /* DiskDriver */

/* Write -----------------------------------------------------------------
   Purpose : This is a helper method for the disk driver. Instead of 
   				having all of this code inside of the driver we can pull
   				it out here to make the driver cleaner. This does the
   				actual writing to the disk.
 Arguments : Takes a diskRequest as input. This request has all of the
 				info that we need to write!
   Returns : void
 * ---------------------------------------------------------------------*/
void Write(diskRequest * request, int size)
{
	if(dFlag){
		USLOSS_Console("Write(): Calling Write with\n");
		USLOSS_Console(" -- request\n");
		USLOSS_Console(" -- size    : %i\n\n", size);
	}

	int status, result;
	int disk = request->unit;

	//We need to check and see if we have enough space on our disk
	if(CheckSize(request->track, request->first + request->sectors, size)){
		//First we have to seek to the correct track
		USLOSS_DeviceRequest devReq;
		Seek(request->track, request->unit);

		//Now we write!
		for(int i = 0; i < request->sectors; i++){
			int sector = request->first;
			if(request->track*UDTS + sector >= request->track*UDTS+16){
				request->track++;
				request->first = 0;
				Seek(request->track, request->unit);
			}

			devReq.opr = USLOSS_DISK_WRITE;
			devReq.reg1 = request->first++;
			devReq.reg2 = request->dbuff;
			result = USLOSS_DeviceOutput(USLOSS_DISK_DEV, disk, &devReq);
			result = waitDevice(USLOSS_DISK_DEV, disk, &status);
			request->dbuff = request->dbuff + USLOSS_DISK_SECTOR_SIZE;
			if(result != 0)
				request->process->success = status;
			else
				request->process->success = result;
		}
	}
	else {
		request->process->success = -1;
	}
} /* Write */

/* Read ------------------------------------------------------------------
   Purpose : This is a helper method for the disk driver. Instead of 
   				having all of this code inside of the driver we can pull
   				it out here to make the driver cleaner. This does the
   				actual reading from the disk.
 Arguments : Takes a diskRequest as input. This request has all of the
 				info that we need to read!
   Returns : void
 * ---------------------------------------------------------------------*/
void Read(diskRequest * request, int size)
{
	if(dFlag){
		USLOSS_Console("Read(): Calling Read with\n");
		USLOSS_Console(" -- request\n");
		USLOSS_Console(" -- size    : %i\n\n", size);
	}

	int status, result;
	int disk = request->unit;
	//We need to check and see if we have enough space on our disk
	if(CheckSize(request->track, request->first + request->sectors, size)){
		//First we have to seek to the correct track
		USLOSS_DeviceRequest devReq;
		Seek(request->track, request->unit);

		//Now we write!
		for(int i = 0; i < request->sectors; i++){
			int sector = request->first;
			if(request->track*UDTS + sector >= request->track*UDTS+16){
				request->track++;
				request->first = 0;
				Seek(request->track, request->unit);
			}

			devReq.opr = USLOSS_DISK_READ;
			devReq.reg1 = request->first++;
			devReq.reg2 = request->dbuff;
			result = USLOSS_DeviceOutput(USLOSS_DISK_DEV, disk, &devReq);
			result = waitDevice(USLOSS_DISK_DEV, disk, &status);
			if(result != 0)
				request->process->success = status;
			else
				request->process->success = result;
			request->dbuff = request->dbuff + USLOSS_DISK_SECTOR_SIZE;
		}
	}
	else {
		request->process->success = -1;
	}
} /* Write */

/* Seek ------------------------------------------------------------------
   Purpose : This is a helper method for the disk driver. Instead of 
   				having all of this code inside of the driver we can pull
   				it out here to make the driver cleaner. This does the
   				actual seeking on the disk.
 Arguments : Takes a track and the unit to seek on.
   Returns : void
 * ---------------------------------------------------------------------*/
void Seek(int track, int unit)
{
	int status;
	USLOSS_DeviceRequest devReq;
	devReq.opr = USLOSS_DISK_SEEK;
	devReq.reg1 = track;
	int result = USLOSS_DeviceOutput(USLOSS_DISK_DEV, unit, &devReq);
	result = waitDevice(USLOSS_DISK_DEV, unit, &status);
	UNUSED;
} /* Seek */

void AddToQ(diskRequest * dr, int unit)
{
	if(dFlag){
		USLOSS_Console("AddToQ(): Calling AddToQ with\n");
		USLOSS_Console(" -- track: %i\n", dr->track);
		USLOSS_Console(" -- unit : %i\n", unit);
		USLOSS_Console(" -- curr : %i\n", diskTrack[unit]);
	}
    if(diskHead[unit].active != ACTIVE){		// This is the case that the Q is empty
	   	diskHead[unit] = *dr;
	}
	else { 									// The Q is not empty
		// There are two cases in this situation. The request that we are adding is
		// > the track that the disk is currently on and the case that the request
		// we are adding is < the track that the disk is currently on.
		drPtr temp = &diskHead[unit];
		int track = dr->track;
		if(track <= diskTrack[unit]){ 		// Our track is < current track
			while(track < temp->track && temp->next->active == ACTIVE){
				temp = temp->next;
			}
			if(temp->next->active != ACTIVE){		// if our iterator isn't a request yet...
				temp = dr;
			}
		}
		else {								// Our track is > current track
			while(track > temp->track && temp->next->active == ACTIVE){
				temp = temp->next;
			}
			if(temp->next->active != ACTIVE){
				temp->next = dr;
			}
		}

	}

	printDiskQ(unit);
}

int CheckSize(int track, int sectors, int size)
{
	int endIndex = track * 16 + sectors;
	if(endIndex > size){
		return 0;
	}
	return 1;
}

void printDiskQ(int unit)
{
	if(dFlag){
		drPtr temp = &diskHead[unit];
		while(temp->active == ACTIVE){
			printf("%i -> ", temp->track);
			temp = temp->next;
		}
		printf("\n\n");
	}
}

void printQ()
{
	if(dFlag){
		USLOSS_Console("printQ(): calling printQ.\n");
	}
	qPtr iter = head;
	while (iter->process > 0 && iter->next != NULL){
		printf("%i -> ", iter->process);
		iter = iter->next;
	}
	printf("%i\n\n", iter->process);
}

void DProcs(){
	for(int i = 9; i < 12; i++){
		printf("%i - %i\n", ProcTable[i].pid, ProcTable[i].status);
	}
}
