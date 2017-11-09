/*
 * These are the definitions for phase 4 of the project (support level, part 2).
 */

#ifndef _PHASE4_H
#define _PHASE4_H

//Maximum line length
#define MAXLINE         80

//Process statuses
#define ASLEEP 			-10
#define DQSLEEP 		77
#define PSLEEP			777

//Disk is blocked on private sem
#define DBLOCKED 		-5
#define DRUNNING		1

//Disk Request Constants
#define DISKREAD 		0
#define DISKWRITE 		1
#define ACTIVE 			555

#define UDTS 			USLOSS_DISK_TRACK_SIZE

typedef struct diskRequest * drPtr;
typedef struct p4proc 	   * procPtr;
typedef struct procQ  	   * qPtr;

typedef struct diskRequest {
	void * 		dbuff;
	int			unit;
	int 		track;
	int 		first;
	int 		sectors;
	int			rw;
	int			id;
	procPtr		process;
	drPtr 	 	next;
	int 		active;
} diskRequest;

typedef struct p4proc {
	int 		status;
	int 		pid;
	int 		pSem;
	int 		success;
	void *		dbuff;
} p4proc;

typedef struct procQ {
	int 		process;
	qPtr 		next;
	int 		wakeUpTime;
} procQ;

// Function Prototypes
extern  int  		DiskRead (void *diskBuffer, int unit, int track, int first, int sectors, int *status);
extern  int  		DiskWrite(void *diskBuffer, int unit, int track, int first, int sectors, int *status);
extern  int 		diskwriteReal(int unit, int track, int first, int sectors, void * buffer);
extern  int 		diskreadReal(int unit, int track, int first, int sectors, void * buffer);
extern  int  		TermRead (char *buffer, int bufferSize, int unitID, int *numCharsRead);
extern  int  		TermWrite(char *buffer, int bufferSize, int unitID, int *numCharsRead);
extern void 		disksizeReal(int unit, int * sector, int * track, int * disk);
extern  int 		DiskSize (int unit, int *sector, int *track, int *disk);
extern  int 		termreadReal(char * buff, int bsize, int unit);
extern  int			CheckSize(int track, int sectors, int size);
extern void 		Write(diskRequest * request, int size);
extern void 		Read(diskRequest * request, int size);
extern void			AddToQ(diskRequest * dr, int unit);
extern void 		diskwrite(systemArgs * args);
extern void			printDiskQ(int unit);
extern void 		disksize(systemArgs * args);
extern void        	termread(systemArgs * args);
extern void		termwrite(systemArgs * args);
extern void 		diskread(systemArgs * args);
extern void 		seek(int track, int unit);
extern void        	sleep(systemArgs * args);
extern void        	sleepReal(int seconds);
extern  int  		Sleep(int seconds);
extern  int  		start4(char *);
extern void 		DProcs();
extern void        	printQ();

#define ERR_INVALID             -1
#define ERR_OK                  0

#endif /* _PHASE4_H */
