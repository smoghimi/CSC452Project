/*
 * These are the definitions for phase 4 of the project (support level, part 2).
 */

#ifndef _PHASE4_H
#define _PHASE4_H

//Maximum line length
#define MAXLINE         80

//Sleeping status
#define ASLEEP 			-10

typedef struct p4proc * procPtr;
typedef struct procQ  * qPtr;

typedef struct p4proc {
	int 		status;
	int 		pid;
} p4proc;

typedef struct procQ {
	int 		process;
	qPtr 		next;
	int 		wakeUpTime;
} procQ;

/*
 * Function prototypes for this phase.
 */

extern  int  Sleep(int seconds);

extern  int  DiskRead (void *diskBuffer, int unit, int track, int first, 
                       int sectors, int *status);
extern  int  DiskWrite(void *diskBuffer, int unit, int track, int first,
                       int sectors, int *status);
extern  int  DiskSize (int unit, int *sector, int *track, int *disk);
extern  int  TermRead (char *buffer, int bufferSize, int unitID,
                       int *numCharsRead);
extern  int  TermWrite(char *buffer, int bufferSize, int unitID,
                       int *numCharsRead);

extern  int  start4(char *);

#define ERR_INVALID             -1
#define ERR_OK                  0

#endif /* _PHASE4_H */
