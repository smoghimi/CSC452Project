/*
 * These are the definitions for phase1 of the project (the kernel).
 */

#ifndef _PHASE1_H
#define _PHASE1_H

#include <usloss.h>

/*
 * Maximum number of processes. 
 */

#define MAXPROC      50

/*
 * Maximum length of a process name
 */

#define MAXNAME      50

/*
 * Maximum length of string argument passed to a newly created process
 */

#define MAXARG       100

/*
 * Maximum number of syscalls.
 */

#define MAXSYSCALLS  50


/* 
 * Function prototypes for this phase.
 */

extern int   fork1(char *name, int(*func)(char *), char *arg,
                   int stacksize, int priority);
extern int   join(int *status);
extern void  quit(int status);
extern int   zap(int pid);
extern int   isZapped(void);
extern int   getpid(void);
extern void  dumpProcesses(void);
extern int   blockMe(int block_status);
extern int   unblockProc(int pid);
extern int   readCurStartTime(void);
extern void  timeSlice(void);
extern void  dispatcher(void);
extern int   readtime(void);

extern void  p1_fork(int pid);
extern void  p1_quit(int pid);
extern void  p1_switch(int old, int new);

#endif /* _PHASE1_H */
