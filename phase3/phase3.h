/*
 * These are the definitions for phase 3 of the project
 */

#ifndef _PHASE3_H
#define _PHASE3_H

#define MAXSEMS         200

typedef struct p3proc * procPtr;
typedef struct sem * semPtr;

typedef struct p3proc {
	int 		pid;
	int 		children;
	int 		(* startFunc) (char *);
	char * 		arg;	
	procPtr		child;
	procPtr 	nextSibling;
	procPtr 	nextSemBlocked;
} p3proc;

typedef struct sem {
	int			licenses;
	procPtr		blockList;
} sem;

#endif /* _PHASE3_H */

